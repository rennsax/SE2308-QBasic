#include "Visitor.h"

using namespace std::string_literals;

namespace basic_visitor {

InterpretVisitor::InterpretVisitor(
    std::ostream &out, std::ostream &err,
    const std::function<std::string()> &input_action) noexcept
    : out(out), err(err), input_action_ref(input_action) {
    assert(input_action_ref.get());
}

std::any InterpretVisitor::visitLineNum(BasicParser::LineNumContext *ctx) {
    return static_cast<LSize>(std::stoi(ctx->INT()->getText()));
}

std::any InterpretVisitor::visitProg(BasicParser::ProgContext *ctx) {
    std::vector<BasicParser::Stm0Context *> stm0_list = ctx->stm0();
    if (stm0_list.empty()) {
        return {};
    }
    std::map<LSize, BasicParser::StmContext *> stm_list{};
    std::map<LSize, std::string> comment_list{}; // Trimmed

    for (auto stm0 : stm0_list) {
        LSize line_num = std::any_cast<LSize>(visit(stm0->line_num()));
        if (stm0->stm()) {
            stm_list.insert({line_num, stm0->stm()});
        } else if (stm0->COMMENT()) {
            auto comment = stm0->COMMENT()->getText();
            comment.erase(end(comment) - 1);

            comment_list.emplace(std::piecewise_construct,
                                 std::forward_as_tuple(line_num),
                                 std::forward_as_tuple(std::move(comment)));
        }
    }

    assert(stm_list.size() + comment_list.size() == stm0_list.size());

    auto get_next_line = [&](LSize cur_line) -> LSize {
        auto nl_it_stm = stm_list.upper_bound(cur_line);
        auto nl_it_cmt = comment_list.upper_bound(cur_line);

        if (nl_it_stm == end(stm_list) && nl_it_cmt == end(comment_list)) {
            // The prog reaches its end.
            return 0;
        } else if (nl_it_stm == end(stm_list)) {
            return nl_it_cmt->first;
        } else if (nl_it_cmt == end(comment_list)) {
            return nl_it_stm->first;
        }
        return std::min(nl_it_stm->first, nl_it_cmt->first);
    };

    // Interpret
    for (LSize cur_line = begin(stm_list)->first; cur_line != 0;) {

        if (comment_list.find(cur_line) == end(comment_list) &&
            stm_list.find(cur_line) == end(stm_list)) {
            runtime_error("invalid line number: " + std::to_string(cur_line));
            break;
        }

        if (comment_list.find(cur_line) != end(comment_list)) {
            // The statement is a comment.
            cur_line = get_next_line(cur_line);
            continue;
        }

        auto stm_to_visit = stm_list.at(cur_line);
        // The statement may return a line number, which indicates the change of
        // control flow.
        auto maybe_nl = visit(stm_to_visit);
        if (maybe_nl.has_value()) {
            cur_line = std::any_cast<LSize>(maybe_nl);
        } else {
            // If the returned line number isn't presented, just fallback to the
            // next line.
            cur_line = get_next_line(cur_line);
        }
    }

    return {};
}

std::any InterpretVisitor::visitEndStm(BasicParser::EndStmContext *ctx) {
    return static_cast<LSize>(0);
}
std::any InterpretVisitor::visitGotoStm(BasicParser::GotoStmContext *ctx) {
    ctx->exec_times++;
    return static_cast<LSize>(std::stoi(ctx->INT()->getText()));
}
std::any InterpretVisitor::visitIfStm(BasicParser::IfStmContext *ctx) {
    auto left_expr = visit(ctx->expr(0));
    auto right_expr = visit(ctx->expr(1));
    if (!(left_expr.has_value() && right_expr.has_value())) {
        return {};
    }
    auto left_expr_res = unwrap_val(left_expr);
    auto right_expr_res = unwrap_val(right_expr);
    bool cond{};
    if (ctx->cmp_op()->EQUAL()) {
        cond = left_expr_res == right_expr_res;
    } else if (ctx->cmp_op()->GT()) {
        cond = left_expr_res > right_expr_res;
    } else if (ctx->cmp_op()->LT()) {
        cond = left_expr_res < right_expr_res;
    }

    if (cond) {
        ctx->true_times++;
        return static_cast<LSize>(std::stoi(ctx->INT()->getText()));
    } else {
        ctx->false_times++;
    }

    return {};
}
std::any InterpretVisitor::visitPrintStm(BasicParser::PrintStmContext *ctx) {
    auto val = visit(ctx->expr());
    if (!val.has_value()) {
        return {};
    }
    out << unwrap_val(val) << '\n';
    return {};
}
std::any InterpretVisitor::visitInputStm(BasicParser::InputStmContext *ctx) {
    auto id = parseId(ctx->ID());

    std::string input_str = input_action_ref();
    if (input_str.empty()) {
        runtime_error("empty input");
    } else if (!all_of(begin(input_str), end(input_str), ::isdigit)) {
        runtime_error("invalid input: " + input_str);
    } else {
        v_env->enter(id, std::stoi(input_str));
    }
    return {};
}
std::any InterpretVisitor::visitLetStm(BasicParser::LetStmContext *ctx) {
    ctx->exec_times++;
    auto val = visit(ctx->expr());
    if (!val.has_value()) {
        return {};
    }
    auto id = parseId(ctx->ID());
    v_env->enter(id, unwrap_val(val));
    return {};
}
std::any InterpretVisitor::visitPowerExpr(BasicParser::PowerExprContext *ctx) {
    auto maybe_base = visit(ctx->expr(0));
    auto maybe_exponent = visit(ctx->expr(1));

    if (!(maybe_exponent.has_value() && maybe_base.has_value())) {
        return {};
    }

    auto base = unwrap_val(maybe_base);
    auto exponent = unwrap_val(maybe_exponent);

    if (exponent < 0) {
        auto wrong_token = ctx->expr(1)->getStart();
        std::stringstream err_ss{};
        err_ss << "Unsupported negative exponent: " << exponent;
        static_error(wrong_token, err_ss.str());
        return {};
    }

    return quickPower(base, exponent);
}
std::any InterpretVisitor::visitDivExpr(BasicParser::DivExprContext *ctx) {
    auto maybe_dividend = visit(ctx->expr(0));
    auto maybe_divisor = visit(ctx->expr(1));
    if (!(maybe_dividend.has_value() && maybe_divisor.has_value())) {
        return {};
    }
    auto dividend = unwrap_val(maybe_dividend);
    auto divisor = unwrap_val(maybe_divisor);
    if (divisor == 0) {
        auto wrong_token = ctx->expr(1)->getStart();
        std::stringstream err_ss{};
        err_ss << "Division by zero: " << dividend << " / " << divisor;
        static_error(wrong_token, err_ss.str());
        return {};
    }

    return dividend / divisor;
}
std::any InterpretVisitor::visitPlusExpr(BasicParser::PlusExprContext *ctx) {
    auto left_operand = visit(ctx->expr(0));
    auto right_operand = visit(ctx->expr(1));

    if (!(left_operand.has_value() && right_operand.has_value())) {
        return {};
    }

    return unwrap_val(left_operand) + unwrap_val(right_operand);
}
std::any InterpretVisitor::visitMinusExpr(BasicParser::MinusExprContext *ctx) {
    auto left_operand = visit(ctx->expr(0));
    auto right_operand = visit(ctx->expr(1));

    if (!(left_operand.has_value() && right_operand.has_value())) {
        return {};
    }

    return unwrap_val(left_operand) - unwrap_val(right_operand);
}
std::any InterpretVisitor::visitMultExpr(BasicParser::MultExprContext *ctx) {
    auto left_operand = visit(ctx->expr(0));
    auto right_operand = visit(ctx->expr(1));

    if (!(left_operand.has_value() && right_operand.has_value())) {
        return {};
    }

    return unwrap_val(left_operand) * unwrap_val(right_operand);
}
std::any InterpretVisitor::visitIntExpr(BasicParser::IntExprContext *ctx) {
    auto int_str = ctx->INT()->getText();
    return std::stoi(int_str);
}
std::any InterpretVisitor::visitVarExpr(BasicParser::VarExprContext *ctx) {
    auto var_name = parseId(ctx->ID());
    if (!v_env->exist(var_name)) {
        auto wrong_token = ctx->ID()->getSymbol();
        std::stringstream err_ss{};
        err_ss << "Undefined variable: " << var_name;
        static_error(wrong_token, err_ss.str());
        return {};
    } else {
        return v_env->lookup(var_name).value();
    }
}
std::any InterpretVisitor::visitNegExpr(BasicParser::NegExprContext *ctx) {
    auto val = visit(ctx->expr());
    if (!val.has_value()) {
        return {};
    }
    return -unwrap_val(val);
}
std::any InterpretVisitor::visitModExpr(BasicParser::ModExprContext *ctx) {
    auto maybe_dividend = visit(ctx->expr(0));
    auto maybe_quotient = visit(ctx->expr(1));

    if (!(maybe_dividend.has_value() && maybe_quotient.has_value())) {
        return {};
    }

    auto dividend = unwrap_val(maybe_dividend);
    auto quotient = unwrap_val(maybe_quotient);

    if (quotient == 0) {
        auto wrong_token = ctx->expr(1)->getStart();
        std::stringstream err_ss{};
        err_ss << "Modulus by zero: " << dividend << " MOD " << quotient;
        static_error(wrong_token, err_ss.str());
        return {};
    }

    if (quotient * dividend < 0) {
        dividend -= (dividend / quotient - 1) * quotient;
    }

    return dividend % quotient;
}
std::any InterpretVisitor::visitParenExpr(BasicParser::ParenExprContext *ctx) {
    return visit(ctx->expr());
}
VarType InterpretVisitor::unwrap_val(std::any val) noexcept {
    return std::any_cast<VarType>(val);
}
std::string InterpretVisitor::parseId(tree::TerminalNode *node) noexcept {
    return node->getText();
}
VarType InterpretVisitor::quickPower(VarType base, VarType exponent) noexcept {
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
void InterpretVisitor::static_error(Token *wrong_token,
                                    const std::string &msg) {
    log_error(err, wrong_token->getLine(),
              wrong_token->getCharPositionInLine() + 1, msg);
}
void InterpretVisitor::runtime_error(std::string_view msg) {
    err << "runtime error: " << msg << '\n';
}

std::any ASTConstructVisitor::visitLineNum(BasicParser::LineNumContext *ctx) {
    ast_out(ctx->INT()->getText());
    return {};
}
std::any ASTConstructVisitor::visitStm0(BasicParser::Stm0Context *ctx) {
    if (ctx->COMMENT()) {
        // Is comment
        visit(ctx->line_num());
        auto comment_str = ctx->COMMENT()->getText();
        // remove beginning "REM " and NL
        // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
        comment_str = comment_str.substr(4, comment_str.length() - 5);
        ast_out("REM\n"s + "\t" + comment_str);
        ast_newline();
        return {};
    }
    assert(ctx->stm());
    visitChildren(ctx);
    return {};
}

ASTConstructVisitor::ASTConstructVisitor(
    const std::shared_ptr<VariableEnv> v_env) noexcept
    : v_env(v_env) {
}

std::any ASTConstructVisitor::visitEndStm(BasicParser::EndStmContext *ctx) {
    ast_out("END");
    ast_newline();
    return {};
}

std::any ASTConstructVisitor::visitGotoStm(BasicParser::GotoStmContext *ctx) {
    ast_out("GOTO");
    ast_out(ctx->exec_times);
    ast_newline();
    ast_indent(1);
    ast_out(ctx->INT()->getText());
    ast_newline();
    return {};
}

std::any ASTConstructVisitor::visitIfStm(BasicParser::IfStmContext *ctx) {
    ast_out("IF THEN");
    ast_out(ctx->true_times);
    ast_out(ctx->false_times);
    ast_newline();
    indent_size++;
    visit(ctx->expr(0));
    ast_indent();
    if (ctx->cmp_op()->EQUAL()) {
        ast_out("=");
    } else if (ctx->cmp_op()->GT()) {
        ast_out(">");
    } else if (ctx->cmp_op()->LT()) {
        ast_out("<");
    }
    ast_newline();
    visit(ctx->expr(1));
    indent_size--;
    return {};
}

std::any ASTConstructVisitor::visitPrintStm(BasicParser::PrintStmContext *ctx) {
    ast_out("PRINT");
    ast_newline();
    indent_size++;
    visit(ctx->expr());
    indent_size--;
    return {};
}

std::any ASTConstructVisitor::visitInputStm(BasicParser::InputStmContext *ctx) {
    ast_out("INPUT");
    ast_newline();
    ast_indent(1);
    ast_out(ctx->ID()->getText());
    ast_newline();
    return {};
}

std::any ASTConstructVisitor::visitLetStm(BasicParser::LetStmContext *ctx) {
    ast_out("LET =");
    ast_out(ctx->exec_times);
    ast_newline();
    indent_size++;
    ast_indent();
    ast_out(ctx->ID()->getText());
    ast_out(v_env->get_ref_time(ctx->ID()->getText()));
    ast_newline();
    visit(ctx->expr());
    indent_size--;
    return {};
}
std::any ASTConstructVisitor::visitPowerExpr(
    BasicParser::PowerExprContext *ctx) {
    visitBinaryOpExpr(ctx, "**");
    return {};
}
std::any ASTConstructVisitor::visitDivExpr(BasicParser::DivExprContext *ctx) {
    visitBinaryOpExpr(ctx, "/");
    return {};
}
std::any ASTConstructVisitor::visitPlusExpr(BasicParser::PlusExprContext *ctx) {
    visitBinaryOpExpr(ctx, "+");
    return {};
}
std::any ASTConstructVisitor::visitMultExpr(BasicParser::MultExprContext *ctx) {
    visitBinaryOpExpr(ctx, "*");
    return {};
}
std::any ASTConstructVisitor::visitNegExpr(BasicParser::NegExprContext *ctx) {
    ast_indent();
    ast_out("-");
    ast_newline();
    indent_size++;
    visit(ctx->expr());
    indent_size--;
    return {};
}
std::any ASTConstructVisitor::visitVarExpr(BasicParser::VarExprContext *ctx) {
    ast_indent();
    ast_out(ctx->ID()->getText());
    ast_newline();
    return {};
}
std::any ASTConstructVisitor::visitIntExpr(BasicParser::IntExprContext *ctx) {
    ast_indent();
    ast_out(ctx->INT()->getText());
    ast_newline();
    return {};
}
std::any ASTConstructVisitor::visitModExpr(BasicParser::ModExprContext *ctx) {
    visitBinaryOpExpr(ctx, "%");
    return {};
}
std::any ASTConstructVisitor::visitParenExpr(
    BasicParser::ParenExprContext *ctx) {
    visit(ctx->expr());
    return {};
}
std::any ASTConstructVisitor::visitMinusExpr(
    BasicParser::MinusExprContext *ctx) {
    visitBinaryOpExpr(ctx, "-");
    return {};
}
std::any ASTConstructVisitor::visitErrStm(BasicParser::ErrStmContext *ctx) {
    ast_out("ERROR");
    ast_newline();
    return {};
}
} // namespace basic_visitor