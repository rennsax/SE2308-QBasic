#include "Interpreter.h"
#include "Visitor.h"
#include "common.h"
#include <BasicANTLR.h>

#include <cassert>

using namespace antlr_basic;
using namespace antlr4;

namespace {

class CustomErrorListener : public BaseErrorListener {

    void syntaxError(Recognizer *recognizer, Token *offendingSymbol,
                     size_t line, size_t charPositionInLine,
                     const std::string &msg, std::exception_ptr e) override {
        last_err_line = line;
    }

public:
    basic::LSize last_err_line = 0;
};

class ThrowExceptionStrategy : public DefaultErrorStrategy {

public:
    void recover(Parser *recognizer, std::exception_ptr e) override {
        throw NoViableAltException{recognizer};
    }

    Token *recoverInline(Parser *recognizer) override {
        throw NoViableAltException{recognizer};
    }
};

} // namespace

namespace basic {

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define GET_BASIC_PARSER()                                                     \
    auto code_stream = frag->get_frag_stream();                                \
    ANTLRInputStream input(code_stream);                                       \
                                                                               \
    /* Lexer, get tokens */                                                    \
    BasicLexer lexer(&input);                                                  \
    CommonTokenStream tokens(&lexer);                                          \
    tokens.fill();                                                             \
                                                                               \
    /* Configure a parse */                                                    \
    BasicParser parser(&tokens);                                               \
    parser.setErrorHandler(std::make_shared<ThrowExceptionStrategy>());        \
    parser.removeErrorListeners();                                             \
    CustomErrorListener my_lister{};                                           \
    parser.addErrorListener(&my_lister);

Interpreter::Interpreter(std::shared_ptr<Fragment> frag, std::ostream &out,
                         std::ostream &err, std::istream &is)
    : Interpreter(std::move(frag), out, err, [&is]() -> std::string {
          std::string input_str{};
          std::getline(is, input_str);
          return input_str;
      }) {
}

Interpreter::Interpreter(std::shared_ptr<Fragment> frag, std::ostream &out,
                         std::ostream &err)
    : Interpreter(std::move(frag), out, err, []() -> std::string {
          return "10";
      }) {
}

Interpreter::Interpreter(std::shared_ptr<Fragment> frag, std::ostream &out,
                         std::ostream &err,
                         std::function<std::string()> input_action)
    : frag(std::move(frag)), out(out), err(err),
      input_action(std::move(input_action)) {
}

void Interpreter::interpret() {
    rewrite();
    GET_BASIC_PARSER();

    tree::ParseTree *tree = parser.prog();

    // Visitor, interpret
    basic_visitor::InterpretVisitor exec_visitor{out, err, input_action};
    exec_visitor.visit(tree);
    basic_visitor::ASTConstructVisitor ast_visitor{exec_visitor.get_var_env()};
    ast_visitor.visit(tree);
    ast_res = ast_visitor.get_ast();
    has_exec = true;
}

std::string Interpreter::show_ast() {
    if (has_exec) {
        return ast_res;
    } else {
        interpret();
        assert(has_exec);
        return ast_res;
    }
}

void Interpreter::rewrite() {
    if (already_rewritten) {
        return;
    }
    bool already_done = false;
    while (!already_done) {
        GET_BASIC_PARSER();
        try {
            // Try to parser, get AST
            tree::ParseTree *tree = parser.prog();
            already_done = true;

        } catch (const RecognitionException &e) {
            auto maybe_line_num =
                this->frag->get_line_number_at(my_lister.last_err_line);
            assert(maybe_line_num.has_value());
            // The line to be modified.
            LSize line_num = maybe_line_num.value();
            this->frag->remove(line_num);
            this->frag->insert(line_num, ERROR_LINE);
        } catch (const std::exception &e) {
            assert(0);
        }
    }
    already_rewritten = true;
}

} // namespace basic