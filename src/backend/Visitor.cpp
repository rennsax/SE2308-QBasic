#include "Visitor.h"

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

    transform(
        begin(stm0_list), end(stm0_list), inserter(stm_list, end(stm_list)),
        [this](
            BasicParser::Stm0Context *stm0) -> decltype(stm_list)::value_type {
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
            runtime_error("invalid line number: " + std::to_string(cur_line));
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

std::any InterpretVisitor::visitEndStm(BasicParser::EndStmContext *ctx) {
    return static_cast<LSize>(0);
}
std::any InterpretVisitor::visitGotoStm(BasicParser::GotoStmContext *ctx) {
    return static_cast<LSize>(std::stoi(ctx->INT()->getText()));
}
std::any InterpretVisitor::visitIfStm(BasicParser::IfStmContext *ctx) {
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
std::any InterpretVisitor::visitPrintStm(BasicParser::PrintStmContext *ctx) {
    auto val = parseExpr(ctx->expr());
    if (!val.has_value()) {
        return {};
    }
    out << val.value() << '\n';
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
        var_env[id] = std::stoi(input_str);
    }
    return {};
}
std::any InterpretVisitor::visitLetStm(BasicParser::LetStmContext *ctx) {
    auto val = parseExpr(ctx->expr());
    if (!val.has_value()) {
        return {};
    }
    auto id = parseId(ctx->ID());
    var_env[id] = val.value();
    return {};
}
std::any InterpretVisitor::visitPowerExpr(BasicParser::PowerExprContext *ctx) {
    auto maybe_base = parseExpr(ctx->expr(0));
    auto maybe_exponent = parseExpr(ctx->expr(1));

    if (!(maybe_exponent.has_value() && maybe_base.has_value())) {
        return {};
    }

    auto base = maybe_base.value();
    auto exponent = maybe_exponent.value();

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
    auto maybe_dividend = parseExpr(ctx->expr(0));
    auto maybe_divisor = parseExpr(ctx->expr(1));
    if (!(maybe_dividend.has_value() && maybe_divisor.has_value())) {
        return {};
    }
    auto dividend = maybe_dividend.value();
    auto divisor = maybe_divisor.value();
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
    auto left_operand = parseExpr(ctx->expr(0));
    auto right_operand = parseExpr(ctx->expr(1));

    if (!(left_operand.has_value() && right_operand.has_value())) {
        return {};
    }

    return left_operand.value() + right_operand.value();
}
std::any InterpretVisitor::visitMinusExpr(BasicParser::MinusExprContext *ctx) {
    auto left_operand = parseExpr(ctx->expr(0));
    auto right_operand = parseExpr(ctx->expr(1));

    if (!(left_operand.has_value() && right_operand.has_value())) {
        return {};
    }

    return left_operand.value() - right_operand.value();
}
std::any InterpretVisitor::visitMultExpr(BasicParser::MultExprContext *ctx) {
    auto left_operand = parseExpr(ctx->expr(0));
    auto right_operand = parseExpr(ctx->expr(1));

    if (!(left_operand.has_value() && right_operand.has_value())) {
        return {};
    }

    return left_operand.value() * right_operand.value();
}
std::any InterpretVisitor::visitIntExpr(BasicParser::IntExprContext *ctx) {
    auto int_str = ctx->INT()->getText();
    return std::stoi(int_str);
}
std::any InterpretVisitor::visitVarExpr(BasicParser::VarExprContext *ctx) {
    auto var_name = parseId(ctx->ID());
    if (auto it = var_env.find(var_name); it == var_env.end()) {
        auto wrong_token = ctx->ID()->getSymbol();
        std::stringstream err_ss{};
        err_ss << "Undefined variable: " << var_name;
        static_error(wrong_token, err_ss.str());
        return {};
    } else {
        return it->second;
    }
}
std::any InterpretVisitor::visitNegExpr(BasicParser::NegExprContext *ctx) {
    auto val = parseExpr(ctx->expr());
    if (!val.has_value()) {
        return {};
    }
    return -val.value();
}
std::any InterpretVisitor::visitModExpr(BasicParser::ModExprContext *ctx) {
    auto maybe_dividend = parseExpr(ctx->expr(0));
    auto maybe_modulus = parseExpr(ctx->expr(1));

    if (!(maybe_dividend.has_value() && maybe_modulus.has_value())) {
        return {};
    }

    auto dividend = maybe_dividend.value();
    auto modulus = maybe_modulus.value();

    if (modulus == 0) {
        auto wrong_token = ctx->expr(1)->getStart();
        std::stringstream err_ss{};
        err_ss << "Modulus by zero: " << dividend << " MOD " << modulus;
        static_error(wrong_token, err_ss.str());
        return {};
    }

    return dividend % modulus;
}
std::any InterpretVisitor::visitParenExpr(BasicParser::ParenExprContext *ctx) {
    return parseExpr(ctx->expr());
}
std::optional<VarType> InterpretVisitor::parseExpr(
    BasicParser::ExprContext *ctx) noexcept {
    auto val = visit(ctx);
    if (val.has_value()) {
        return std::any_cast<VarType>(visit(ctx));
    } else {
        return {};
    }
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
} // namespace basic_visitor