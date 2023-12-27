#include "Interpreter.h"
#include "common.h"
#include <BasicANTLR.h>

#include <cassert>
#include <type_traits>
#include <unordered_map>

using namespace antlr_basic;
using namespace antlr4;

namespace {
using namespace basic;
class InterpretVisitor : public BasicBaseVisitor {

    /// Value type in Basic
    using VarType = std::int32_t;
    using ExprFallback = std::integral_constant<VarType, 0>;

public:
    explicit InterpretVisitor(
        std::ostream &out, std::ostream &err,
        const std::function<std::string()> &input_action) noexcept
        : out(out), err(err), input_action_ref(input_action) {
        assert(input_action_ref.get());
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
                runtime_error("invalid line number: " +
                              std::to_string(cur_line));
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
        std::string input_str = input_action_ref();
        if (input_str.empty()) {
            runtime_error("empty input");
            var_env[id] = ExprFallback::value;
        } else if (!all_of(begin(input_str), end(input_str), ::isdigit)) {
            runtime_error("invalid input: " + input_str);
            var_env[id] = ExprFallback::value;
        } else {
            var_env[id] = std::stoi(input_str);
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
            static_error(wrong_token, err_ss.str());
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
            static_error(wrong_token, err_ss.str());
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
            static_error(wrong_token, err_ss.str());
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
            static_error(wrong_token, err_ss.str());
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
    std::reference_wrapper<const std::function<std::string()>> input_action_ref;

    bool has_error = false;

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

    void static_error(Token *wrong_token, const std::string &msg) {
        log_error(err, wrong_token->getLine(),
                  wrong_token->getCharPositionInLine() + 1, msg);
        has_error = true;
    }

    void runtime_error(std::string_view msg) {
        err << "runtime error: " << msg << '\n';
        has_error = true;
    }
};

class CustomErrorListener : public BaseErrorListener {

    void syntaxError(Recognizer *recognizer, Token *offendingSymbol,
                     size_t line, size_t charPositionInLine,
                     const std::string &msg, std::exception_ptr e) override {
        last_err_line = line;
    }

public:
    LSize last_err_line = 0;
};
} // namespace

namespace basic {

Interpreter::Interpreter(std::shared_ptr<Fragment> frag, std::ostream &out,
                         std::ostream &err, std::istream &is)
    : Interpreter(std::move(frag), out, err, [&is]() -> std::string {
          std::string input_str{};
          std::getline(is, input_str);
          return input_str;
      }) {
}

Interpreter::Interpreter(std::shared_ptr<Fragment> frag, std::ostream &out,
                         std::ostream &err,
                         std::function<std::string()> input_action)
    : frag(std::move(frag)), out(out), err(err),
      input_action(std::move(input_action)) {
}

void Interpreter::interpret() {
    auto code_stream = frag->get_frag_stream();

    // Lexer, get tokens
    ANTLRInputStream input(code_stream);
    BasicLexer lexer(&input);
    CommonTokenStream tokens(&lexer);

    // Parser, get AST
    BasicParser parser(&tokens);
    parser.setErrorHandler(std::make_shared<BailErrorStrategy>());
    parser.removeErrorListeners();
    CustomErrorListener my_lister{};
    parser.addErrorListener(&my_lister);

    while (1) {
        try {
            tree::ParseTree *tree = parser.prog();
            // Visitor, interpret
            InterpretVisitor visitor{out, err, input_action};
            visitor.visit(tree);
            break;
        } catch (const std::exception &e) {
            auto maybe_line_num =
                this->frag->get_line_number_at(my_lister.last_err_line);
            assert(maybe_line_num.has_value());
            // The line to be modified.
            LSize line_num = maybe_line_num.value();
            this->frag->remove(line_num);
            this->frag->insert(line_num, "___ERROR___");
        }
    }
}

std::string Interpreter::show_ast() const {
    auto code_stream = frag->get_frag_stream();

    // Lexer, get tokens
    ANTLRInputStream input(code_stream);
    BasicLexer lexer(&input);
    CommonTokenStream tokens(&lexer);

    // Parser, get AST
    BasicParser parser(&tokens);
    tree::ParseTree *tree = parser.prog();

    return tree->toStringTree(true);
}

} // namespace basic