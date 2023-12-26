#include "Interpreter.h"
#include "common.h"
#include <BasicANTLR.h>

#include <type_traits>
#include <unordered_map>

using namespace antlr_basic;
using namespace antlr4;

namespace basic {

class InterpretVisitor : public BasicBaseVisitor {

    using VarType = std::int32_t;
    using ExprFallback = std::integral_constant<VarType, 0>;

public:
    // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
    explicit InterpretVisitor(std::ostream &out, std::ostream &err,
                              std::istream &in, bool enable_log)
        : out(out), err(err), in(in), enable_log(enable_log), has_error(false) {
    }

    // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
    explicit InterpretVisitor(std::ostream &out, std::ostream &err,
                              std::istream &in)
        : InterpretVisitor(out, err, in, false) {
    }

    ~InterpretVisitor() override = default;

    // No copy or move.
    InterpretVisitor(const InterpretVisitor &other) = delete;
    InterpretVisitor(InterpretVisitor &&other) = delete;
    InterpretVisitor &operator=(const InterpretVisitor &other) = delete;
    InterpretVisitor &operator=(InterpretVisitor &&other) = delete;

    std::any visitProg(BasicParser::ProgContext *ctx) override {
        std::vector<BasicParser::Stm0Context *> stm0_list = ctx->stm0();
        std::vector<BasicParser::StmContext *> stm_list{};

        transform(begin(stm0_list), end(stm0_list), back_inserter(stm_list),
                  [](BasicParser::Stm0Context *stm0) {
                      return stm0->stm();
                  });
        assert(stm_list.size() == stm0_list.size());

        for (LSize l = 0; l < stm_list.size();) {
            auto stm = stm_list[l];
            if (stm == nullptr) {
                // Empty line or comment
                ++l;
                continue;
            }
            if (stm->end_stm()) {
                // Program ends
                break;
            }
            if (stm->goto_stm()) {
                auto goto_target =
                    std::any_cast<tree::TerminalNode *>(visit(stm));
                auto next_line = std::stoi(goto_target->getText());
                if (next_line < 0 || next_line >= stm_list.size()) {
                    auto wrong_token = goto_target->getSymbol();
                    std::stringstream err_ss{};
                    err_ss << "Invalid line number: " << next_line << '\n';
                    report_error(wrong_token, err_ss.str());
                    ++l;
                } else {
                    l = next_line;
                }
            } else if (stm->if_stm()) {
                auto cjump_optional_target = visit(stm);
                if (cjump_optional_target.has_value()) {
                    auto cjump_target = std::any_cast<tree::TerminalNode *>(
                        cjump_optional_target);
                    auto next_line = std::stoi(cjump_target->getText());
                    if (next_line < 0 || next_line >= stm_list.size()) {
                        auto wrong_token = cjump_target->getSymbol();
                        std::stringstream err_ss{};
                        err_ss << "Invalid line number: " << next_line << '\n';
                        report_error(wrong_token, err_ss.str());
                        ++l;
                        continue;
                    }

                    l = next_line;
                } else {
                    ++l;
                }
            } else {
                visit(stm);
                ++l;
            }
        }

        return {};
    }

    /**
     * @brief
     *
     * @param ctx
     * @return std::any The line number to jump to.
     */
    std::any visitGotoStm(BasicParser::GotoStmContext *ctx) override {
        return ctx->INT();
    }

    /**
     * @brief
     *
     * @param ctx
     * @return std::any Empty if not jump, otherwise the target line number.
     */
    std::any visitIfStm(BasicParser::IfStmContext *ctx) override {
        auto left_expr = parseExpr(ctx->expr(0));
        auto right_expr = parseExpr(ctx->expr(1));
        bool cond{};
        if (ctx->cmp_op()->EQUAL()) {
            cond = left_expr == right_expr;
        } else if (ctx->cmp_op()->GT()) {
            cond = left_expr > right_expr;
        } else if (ctx->cmp_op()->LT()) {
            cond = left_expr < right_expr;
        }

        if (cond) {
            return ctx->INT();
        }

        return {};
    }

    std::any visitPrintStm(BasicParser::PrintStmContext *ctx) override {
        auto val = parseExpr(ctx->expr());
        if (has_error) {
            return {};
        }
        out << val << '\n';
        return {};
    }

    std::any visitLetStm(BasicParser::LetStmContext *ctx) override {
        auto val = parseExpr(ctx->expr());
        auto id = parseId(ctx->ID());
        var_env[id] = val;
        return {};
    }

    std::any visitPowerExpr(BasicParser::PowerExprContext *ctx) override {
        auto base = parseExpr(ctx->expr(0));
        auto exponent = parseExpr(ctx->expr(1));

        if (exponent < 0) {
            auto wrong_token = ctx->expr(1)->getStart();
            std::stringstream err_ss{};
            err_ss << "Unsupported negative exponent: " << exponent << '\n';
            report_error(wrong_token, err_ss.str());
            return ExprFallback::value;
        }

        return quickPower(base, exponent);
    }

    std::any visitDivExpr(BasicParser::DivExprContext *ctx) override {
        auto dividend = parseExpr(ctx->expr(0));
        auto divisor = parseExpr(ctx->expr(1));

        if (divisor == 0) {
            auto wrong_token = ctx->expr(1)->getStart();
            std::stringstream err_ss{};
            err_ss << "Division by zero: " << dividend << " / " << divisor
                   << '\n';
            report_error(wrong_token, err_ss.str());
            return ExprFallback::value;
        }

        return dividend / divisor;
    }

    std::any visitPlusExpr(BasicParser::PlusExprContext *ctx) override {
        auto left_operand = parseExpr(ctx->expr(0));
        auto right_operand = parseExpr(ctx->expr(1));

        return left_operand + right_operand;
    }

    std::any visitMultExpr(BasicParser::MultExprContext *ctx) override {
        auto left_operand = parseExpr(ctx->expr(0));
        auto right_operand = parseExpr(ctx->expr(1));

        return left_operand * right_operand;
    }

    std::any visitIntExpr(BasicParser::IntExprContext *ctx) override {
        auto int_str = ctx->INT()->getText();
        return std::stoi(int_str);
    }

    std::any visitVarExpr(BasicParser::VarExprContext *ctx) override {
        auto var_name = parseId(ctx->ID());
        if (auto it = var_env.find(var_name); it == var_env.end()) {
            auto wrong_token = ctx->ID()->getSymbol();
            std::stringstream err_ss{};
            err_ss << "Undefined variable: " << var_name << '\n';
            report_error(wrong_token, err_ss.str());
            return ExprFallback::value;
        } else {
            return it->second;
        }
    }

    std::any visitNegExpr(BasicParser::NegExprContext *ctx) override {
        VarType val = parseExpr(ctx->expr());
        return -val;
    }

    std::any visitModExpr(BasicParser::ModExprContext *ctx) override {
        auto dividend = parseExpr(ctx->expr(0));
        auto modulus = parseExpr(ctx->expr(1));

        if (modulus == 0) {
            auto wrong_token = ctx->expr(1)->getStart();
            std::stringstream err_ss{};
            err_ss << "Modulus by zero: " << dividend << " MOD " << modulus
                   << '\n';
            report_error(wrong_token, err_ss.str());
            return ExprFallback::value;
        }

        return dividend % modulus;
    }

    std::any visitParenExpr(BasicParser::ParenExprContext *ctx) override {
        return parseExpr(ctx->expr());
    }

    std::any visitMinusExpr(BasicParser::MinusExprContext *ctx) override {
        auto left_operand = parseExpr(ctx->expr(0));
        auto right_operand = parseExpr(ctx->expr(1));

        return left_operand - right_operand;
    }

private:
    std::ostream &out, &err;
    std::istream &in;
    bool enable_log;
    bool has_error;

    std::unordered_map<std::string, VarType> var_env;

    /**
     * @brief Just a wrapper for #visit, which convert the result to #ValType.
     *
     * @param ctx
     * @return VarType
     */
    VarType parseExpr(BasicParser::ExprContext *ctx) noexcept {
        return std::any_cast<VarType>(visit(ctx));
    }

    /**
     * @brief Retrieve the Id from the lexer node.
     *
     * @param node
     * @return std::string
     */
    static std::string parseId(tree::TerminalNode *node) noexcept {
        return node->getText();
    }

    static VarType quickPower(VarType base, VarType exponent) noexcept {
        VarType result = 1;
        while (exponent > 0) {
            if (exponent & 1) {
                result *= base;
            }
            base *= base;
            exponent >>= 1;
        }
        return result;
    }

    void report_error(Token *wrong_token, const std::string &msg) {
        log_error(err, wrong_token->getLine(),
                  wrong_token->getCharPositionInLine(), msg);
        has_error = true;
    }
};

Interpreter::Interpreter(std::shared_ptr<Fragment> frag, std::ostream &out,
                         std::ostream &err, std::istream &is)
    : out(out), err(err), is(is), frag(std::move(frag)) {
}

void Interpreter::interpret() {
    auto code_stream = frag->get_frag_stream();

    // Lexer, get tokens
    ANTLRInputStream input(code_stream);
    BasicLexer lexer(&input);
    CommonTokenStream tokens(&lexer);

    // Parser, get AST
    BasicParser parser(&tokens);
    tree::ParseTree *tree = parser.prog();

    // Visitor, interpret
    InterpretVisitor visitor{out, err, is};
    visitor.visit(tree);
}

} // namespace basic