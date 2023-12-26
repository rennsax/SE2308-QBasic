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
                              std::istream &in,
                              const std::function<void()> &prompt_fun,
                              bool enable_log)
        : out(out), err(err), in(in), show_prompt_ref(prompt_fun),
          enable_log(enable_log), has_error(false) {
    }

    // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
    explicit InterpretVisitor(std::ostream &out, std::ostream &err,
                              std::istream &in,
                              const std::function<void()> &prompt_fun)
        : InterpretVisitor(out, err, in, prompt_fun,
                           /*enable_log =*/false) {
    }

    ~InterpretVisitor() override = default;

    // No copy or move.
    InterpretVisitor(const InterpretVisitor &other) = delete;
    InterpretVisitor(InterpretVisitor &&other) = delete;
    InterpretVisitor &operator=(const InterpretVisitor &other) = delete;
    InterpretVisitor &operator=(InterpretVisitor &&other) = delete;

    /**
     * @brief
     *
     * @param ctx
     * @return std::any LSize
     */
    std::any visitLineNum(BasicParser::LineNumContext *ctx) override {
        return static_cast<LSize>(std::stoi(ctx->INT()->getText()));
    }

    std::any visitProg(BasicParser::ProgContext *ctx) override {
        std::vector<BasicParser::Stm0Context *> stm0_list = ctx->stm0();
        if (stm0_list.empty()) {
            return {};
        }

        transform(
            begin(stm0_list), end(stm0_list), inserter(stm_list, end(stm_list)),
            [this](BasicParser::Stm0Context *stm0)
                -> decltype(stm_list)::value_type {
                LSize line_num = std::any_cast<LSize>(visit(stm0->line_num()));
                return {line_num, stm0->stm()};
            });

        assert(stm_list.size() == stm0_list.size());

        auto get_next_line = [this](LSize cur_line) -> LSize {
            auto nl_it = stm_list.upper_bound(cur_line);
            if (nl_it == end(stm_list)) {
                // The prog reaches its end.
                return 0;
            } else {
                return nl_it->first;
            }
        };

        // Interpret
        for (LSize cur_line = begin(stm_list)->first; cur_line != 0;) {

            if (auto it = stm_list.find(cur_line); it == end(stm_list)) {
                err << "runtime error: invalid line number: " << cur_line
                    << '\n';
                has_error = true;
                break;
            }

            auto stm_to_visit = stm_list.at(cur_line);
            if (stm_to_visit == nullptr) {
                // The statement is a comment.
                cur_line = get_next_line(cur_line);
                continue;
            }

            auto maybe_nl = visit(stm_to_visit);
            if (maybe_nl.has_value()) {
                cur_line = std::any_cast<LSize>(maybe_nl);
            } else {
                cur_line = get_next_line(cur_line);
            }
        }

        return {};
    }

    /**
     * The following three: END, GOTO, IF, may change control flow.
     */

    std::any visitEndStm(BasicParser::EndStmContext *ctx) override {
        return static_cast<LSize>(0);
    }

    std::any visitGotoStm(BasicParser::GotoStmContext *ctx) override {
        return static_cast<LSize>(std::stoi(ctx->INT()->getText()));
    }

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
            return static_cast<LSize>(std::stoi(ctx->INT()->getText()));
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

    std::any visitInputStm(BasicParser::InputStmContext *ctx) override {
        auto id = parseId(ctx->ID());

        VarType val{};
        if (show_prompt_ref.get()) {
            show_prompt_ref();
        }
        in >> val;
        if (!in) {
            auto wrong_token = ctx->INPUT()->getSymbol();
            report_error(wrong_token, "Input error!");
            var_env[id] = ExprFallback::value;
        } else {
            var_env[id] = val;
        }
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
            err_ss << "Unsupported negative exponent: " << exponent;
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
            err_ss << "Division by zero: " << dividend << " / " << divisor;
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
            err_ss << "Undefined variable: " << var_name;
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
            err_ss << "Modulus by zero: " << dividend << " MOD " << modulus;
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
    std::reference_wrapper<const std::function<void()>> show_prompt_ref;

    bool enable_log;
    bool has_error;

    std::unordered_map<std::string, VarType> var_env;

    std::map<LSize, BasicParser::StmContext *> stm_list{};

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
                  wrong_token->getCharPositionInLine() + 1, msg);
        has_error = true;
    }
};

Interpreter::Interpreter(std::shared_ptr<Fragment> frag, std::ostream &out,
                         std::ostream &err, std::istream &is)
    : Interpreter(std::move(frag), out, err, is, /*show_prompt=*/{}) {
}

Interpreter::Interpreter(std::shared_ptr<Fragment> frag, std::ostream &out,
                         std::ostream &err, std::istream &is,
                         std::function<void()> prompt)
    : frag(std::move(frag)), out(out), err(err), is(is),
      show_prompt(std::move(prompt)) {
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
    InterpretVisitor visitor{out, err, is, show_prompt};
    visitor.visit(tree);
}

} // namespace basic