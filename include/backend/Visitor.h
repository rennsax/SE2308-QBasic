#ifndef BASIC_VISITOR_H
#define BASIC_VISITOR_H

#include "common.h"
#include <BasicANTLR.h>
#include <unordered_map>

namespace basic_visitor {

using namespace basic;
using namespace antlr4;
using namespace antlr_basic;

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

    std::unordered_map<std::string, VarType> var_env;

    std::map<LSize, BasicParser::StmContext *> stm_list{};

    /**
     * @brief Just a wrapper for #visit, which convert the result to #ValType.
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

} // namespace basic_visitor

#endif // BASIC_VISITOR_H