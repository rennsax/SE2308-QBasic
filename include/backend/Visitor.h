#ifndef BASIC_VISITOR_H
#define BASIC_VISITOR_H

#include "common.h"
#include <BasicANTLR.h>
#include <unordered_map>

namespace basic_visitor {

using namespace basic;
using namespace antlr4;
using namespace antlr_basic;

struct VariableEnv {
    /// Value and ref time.
    using EnvInformation = std::pair<VarType, int>;

    std::optional<VarType> lookup(const std::string &var_name) noexcept {
        if (!exist(var_name)) {
            return std::nullopt;
        }
        auto &var = var_env.at(var_name);
        var.second++;
        return var.first;
    };

    int get_ref_time(const std::string &var_name) const noexcept {
        if (!exist(var_name)) {
            return -1;
        }
        return var_env.at(var_name).second;
    }

    bool exist(const std::string &var_name) const noexcept {
        return var_env.find(var_name) != end(var_env);
    }

    void enter(const std::string &var_name, VarType value) noexcept {
        if (exist(var_name)) {
            var_env.at(var_name).first = value;
        } else {
            var_env.emplace(var_name, EnvInformation{value, 0});
        }
    }

    std::unordered_map<std::string, EnvInformation> var_env;
};

class InterpretVisitor : public BasicBaseVisitor {

public:
    explicit InterpretVisitor(
        std::ostream &out, std::ostream &err,
        const std::function<std::string()> &input_action) noexcept;

    ~InterpretVisitor() override = default;

    // No copy or move.
    InterpretVisitor(const InterpretVisitor &other) = delete;
    InterpretVisitor(InterpretVisitor &&other) = delete;
    InterpretVisitor &operator=(const InterpretVisitor &other) = delete;
    InterpretVisitor &operator=(InterpretVisitor &&other) = delete;

    std::shared_ptr<VariableEnv> get_var_env() const noexcept {
        return v_env;
    }

    /**
     * @brief
     *
     * @param ctx
     * @return std::any LSize
     */
    std::any visitLineNum(BasicParser::LineNumContext *ctx) override;

    std::any visitProg(BasicParser::ProgContext *ctx) override;

    /**
     * The following three: END, GOTO, IF, may change control flow.
     */

    std::any visitEndStm(BasicParser::EndStmContext *ctx) override;

    std::any visitGotoStm(BasicParser::GotoStmContext *ctx) override;

    std::any visitIfStm(BasicParser::IfStmContext *ctx) override;

    std::any visitPrintStm(BasicParser::PrintStmContext *ctx) override;

    std::any visitInputStm(BasicParser::InputStmContext *ctx) override;

    std::any visitLetStm(BasicParser::LetStmContext *ctx) override;

    std::any visitPowerExpr(BasicParser::PowerExprContext *ctx) override;

    std::any visitDivExpr(BasicParser::DivExprContext *ctx) override;

    std::any visitPlusExpr(BasicParser::PlusExprContext *ctx) override;

    std::any visitMinusExpr(BasicParser::MinusExprContext *ctx) override;

    std::any visitMultExpr(BasicParser::MultExprContext *ctx) override;

    std::any visitIntExpr(BasicParser::IntExprContext *ctx) override;

    std::any visitVarExpr(BasicParser::VarExprContext *ctx) override;

    std::any visitNegExpr(BasicParser::NegExprContext *ctx) override;

    std::any visitModExpr(BasicParser::ModExprContext *ctx) override;

    std::any visitParenExpr(BasicParser::ParenExprContext *ctx) override;

private:
    std::ostream &out, &err;
    std::reference_wrapper<const std::function<std::string()>> input_action_ref;

    std::shared_ptr<VariableEnv> v_env{std::make_unique<VariableEnv>()};

    /**
     * @brief Just a wrapper for #visit, which convert the result to
     * #ValType.
     *
     * @param ctx
     * @return VarType
     */
    std::optional<VarType> parseExpr(BasicParser::ExprContext *ctx) noexcept;

    /**
     * @brief Retrieve the Id from the lexer node.
     *
     * @param node
     * @return std::string
     */
    static std::string parseId(tree::TerminalNode *node) noexcept;

    static VarType quickPower(VarType base, VarType exponent) noexcept;

    void static_error(Token *wrong_token, const std::string &msg);

    void runtime_error(std::string_view msg);
};

class ASTConstructVisitor : public BasicBaseVisitor {

public:
    explicit ASTConstructVisitor(
        const std::shared_ptr<VariableEnv> v_env) noexcept;

    std::string get_ast() const noexcept {
        return ast_ss.str();
    }

    std::any visitLineNum(BasicParser::LineNumContext *ctx) override;

    std::any visitStm0(BasicParser::Stm0Context *ctx) override;

    std::any visitErrStm(BasicParser::ErrStmContext *ctx) override;

    std::any visitEndStm(BasicParser::EndStmContext *ctx) override;

    std::any visitGotoStm(BasicParser::GotoStmContext *ctx) override;

    std::any visitIfStm(BasicParser::IfStmContext *ctx) override;

    std::any visitPrintStm(BasicParser::PrintStmContext *ctx) override;

    std::any visitInputStm(BasicParser::InputStmContext *ctx) override;

    std::any visitLetStm(BasicParser::LetStmContext *ctx) override;

    std::any visitPowerExpr(BasicParser::PowerExprContext *ctx) override;

    std::any visitDivExpr(BasicParser::DivExprContext *ctx) override;

    std::any visitPlusExpr(BasicParser::PlusExprContext *ctx) override;

    std::any visitMultExpr(BasicParser::MultExprContext *ctx) override;

    std::any visitNegExpr(BasicParser::NegExprContext *ctx) override;

    std::any visitVarExpr(BasicParser::VarExprContext *ctx) override;

    std::any visitIntExpr(BasicParser::IntExprContext *ctx) override;

    std::any visitModExpr(BasicParser::ModExprContext *ctx) override;

    std::any visitParenExpr(BasicParser::ParenExprContext *ctx) override;

    std::any visitMinusExpr(BasicParser::MinusExprContext *ctx) override;

private:
    std::shared_ptr<VariableEnv> v_env{};
    std::size_t indent_size = 0;
    std::stringstream ast_ss{};
    bool need_space = false;
    template <typename T> void ast_out(T &&msg) {
        if (need_space) {
            ast_ss << ' ' << std::forward<T>(msg);
        } else {
            ast_ss << std::forward<T>(msg);
            need_space = true;
        }
    }
    void ast_indent(std::size_t indent_cnt) {
        ast_ss << std::string(indent_cnt, '\t');
    }
    void ast_indent() {
        ast_indent(indent_size);
    }
    void ast_newline() {
        ast_ss << '\n';
        need_space = false;
    }

    template <typename Context>
    void visitBinaryOpExpr(Context *ctx, const std::string &op) {
        static_assert(std::is_base_of_v<BasicParser::ExprContext, Context>,
                      "Context must be derived from ExprContext");
        ast_indent();
        ast_out(op);
        ast_newline();
        indent_size++;
        visit(ctx->expr(0));
        visit(ctx->expr(1));
        indent_size--;
    }
};

} // namespace basic_visitor

#endif // BASIC_VISITOR_H