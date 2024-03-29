#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

#include "Interpreter.h"

using namespace basic;

TEST_CASE("expression") {

    auto frag = std::make_shared<Fragment>();
    std::ostringstream out{};
    std::ostringstream err{};
    std::istringstream in{};
    Interpreter inter{frag, out, err, in};

    SUBCASE("power") {
        frag->append("LET x = 2 ** 3");
        frag->append("LET y = 3 ** 4");
        frag->append("PRINT x");
        frag->append("PRINT y");

        inter.interpret();

        CHECK(out.str() == "8\n81\n");
        CHECK(err.str() == "");
    }

    SUBCASE("divide") {
        frag->append("LET x = 2 / 3");
        frag->append("LET y = 100 / -3");
        frag->append("PRINT x");
        frag->append("PRINT y");

        inter.interpret();

        CHECK(out.str() == "0\n-33\n");
        CHECK(err.str() == "");
    }

    SUBCASE("modulo") {
        frag->append("LET x = 2 MOD 3");
        frag->append("LET y = 100 MOD -3");
        frag->append("LET z = -10 MOD -3");
        frag->append("LET w = -1000 MOD 3");
        frag->append("PRINT x");
        frag->append("PRINT y");
        frag->append("PRINT z");
        frag->append("PRINT w");

        inter.interpret();

        CHECK(out.str() == "2\n-2\n-1\n2\n");
        CHECK(err.str() == "");
    }
}

TEST_CASE("control flow") {
    auto frag = std::make_shared<Fragment>();
    std::ostringstream out{};
    std::ostringstream err{};
    std::istringstream in{};
    Interpreter inter{frag, out, err, in};

    SUBCASE("goto statement") {
        frag->append("LET x = 1");   // 100
        frag->append("PRINT x");     // 110
        frag->append("GOTO 140");    // 120
        frag->append("LET x = x+2"); // 130
        frag->append("PRINT x");     // 140

        inter.interpret();

        CHECK(out.str() == "1\n1\n");
    }

    SUBCASE("if statement") {
        frag->append("LET T = 5");         // 100
        frag->append("IF T = 0 THEN 150"); // 110
        frag->append("PRINT T");           // 120
        frag->append("LET T = T - 1");     // 130
        frag->append("GOTO 110");          // 140
        frag->append("END");               // 150

        inter.interpret();

        CHECK(out.str() == "5\n4\n3\n2\n1\n");
    }

    SUBCASE("condition") {
        frag->append("LET x = 10");         // 100
        frag->append("IF x < 9 THEN 140");  // 110
        frag->append("IF x > 12 THEN 150"); // 120
        frag->append("IF x > 8 THEN 160");  // 130
        frag->append("PRINT x - 1");        // 140
        frag->append("PRINT x ");           // 150
        frag->append("PRINT x  + 1");       // 160

        inter.interpret();

        CHECK(out.str() == "11\n");
    }
}

TEST_CASE("input variable") {
    auto frag = std::make_shared<Fragment>();
    std::ostringstream out{};
    std::ostringstream err{};
    std::istringstream in{"20"};
    int input_times = 0;
    Interpreter inter{frag, out, err, [&]() -> std::string {
                          ++input_times;
                          std::string input{};
                          std::getline(in, input);
                          return input;
                      }};

    frag->append("INPUT x");
    frag->append("PRINT x/2");

    inter.interpret();

    CHECK(out.str() == "10\n");
    CHECK(input_times == 1);
}

TEST_CASE("input error") {
    auto frag = std::make_shared<Fragment>();
    std::ostringstream out{};
    std::ostringstream err{};
    std::string input_str{};
    Interpreter inter{frag, out, err, [&]() {
                          return input_str;
                      }};

    SUBCASE("empty input") {
        input_str.clear();
        REQUIRE(input_str == "");

        frag->append("INPUT x");
        inter.interpret();

        CHECK(out.str() == "");
        CHECK(err.str() == "runtime error: empty input\n");
    }

    SUBCASE("invalid input") {
        // Cannot extract to number
        input_str = "abcd";

        frag->append("INPUT x");
        frag->append("PRINT x");
        inter.interpret();

        CHECK(out.str() == "");
        CHECK(err.str() == "runtime error: invalid input: abcd\n"
                           "line 2:11 Undefined variable: x\n");
    }
}

TEST_CASE("comments are lines") {
    auto frag = std::make_shared<Fragment>();
    std::ostringstream out{};
    std::ostringstream err{};
    std::istringstream in{};
    Interpreter inter{frag, out, err, in};

    frag->append("LET x = 0");
    frag->append("REM 1111111"); // 110
    frag->append("REM 2222222"); // 120
    frag->append("LET x = x+1");
    frag->append("PRINT x");
    frag->append("IF x < 5 THEN 110");
    frag->append("IF x < 10 THEN 120");

    inter.interpret();

    CHECK(out.str() == "1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n");
    CHECK(err.str() == "");
}

TEST_CASE("semantic error") {
    auto frag = std::make_shared<Fragment>();
    std::ostringstream out{};
    std::ostringstream err{};
    std::istringstream in{};
    Interpreter inter{frag, out, err, in};

    SUBCASE("negative exponent") {
        frag->append("LET x = 2 ** -3");
        frag->append("PRINT x");

        inter.interpret();

        CHECK(out.str() == "");
        CHECK(err.str() == "line 1:18 Unsupported negative exponent: -3\n"
                           "line 2:11 Undefined variable: x\n");
    }

    SUBCASE("undefined variable") {
        frag->append("PRINT x");

        inter.interpret();

        CHECK(out.str() == "");
        CHECK(err.str() == "line 1:11 Undefined variable: x\n");
    }

    SUBCASE("divided by zero") {
        frag->append("LET x = -2 / -0");

        inter.interpret();

        CHECK(out.str() == "");
        CHECK(err.str() == "line 1:18 Division by zero: -2 / 0\n");
    }

    SUBCASE("modulo by zero") {
        frag->append("LET x = -2 MOD 0");

        inter.interpret();

        CHECK(out.str() == "");
        CHECK(err.str() == "line 1:20 Modulus by zero: -2 MOD 0\n");
    }

    SUBCASE("invalid line number") {
        frag->append("IF 2 > 1 THEN 20");
        frag->append("GOTO 30");

        inter.interpret();

        CHECK(out.str() == "");
        CHECK(err.str() == "runtime error: invalid line number: 20\n");
    }
}

TEST_CASE("compounded") {
    std::ifstream test_ifs{"test_cases/fibonacci.in"};
    auto frag = std::make_shared<Fragment>(Fragment::read_stream(test_ifs));
    REQUIRE(frag->size() == 11);

    std::ostringstream out{};
    std::ostringstream err{};
    std::istringstream in{};
    Interpreter inter{frag, out, err, in};

    inter.interpret();

    std::stringstream ref_ss{};
    std::ifstream ref_ifs{"test_cases/fibonacci.ref"};
    REQUIRE(ref_ifs.is_open());
    ref_ss << ref_ifs.rdbuf();

    CHECK(out.str() == ref_ss.str());
}

TEST_CASE("show AST") {
    std::ifstream test_ifs{"test_cases/fibonacci.in"};
    auto frag = std::make_shared<Fragment>(Fragment::read_stream(test_ifs));
    REQUIRE(frag->size() == 11);

    std::ostringstream out{};
    std::ostringstream err{};
    Interpreter inter{frag, out, err};

    std::stringstream ast_ss{};
    std::fstream ast_fs{"test_cases/fibonacci.ast", std::ios::in};
    REQUIRE(ast_fs.is_open());
    ast_ss << ast_fs.rdbuf();

    CHECK(inter.show_ast() == ast_ss.str());
}

TEST_CASE("syntactic error") {
    auto frag = std::make_shared<Fragment>();
    std::ostringstream out{};
    std::ostringstream err{};
    Interpreter inter{frag, out, err};

    frag->append("INPUTx");
    frag->append("INPUT 2222abcd");
    frag->append("Hello world");
    inter.interpret();

    for (std::size_t i = 0; i < frag->size(); ++i) {
        CHECK(frag->get_line(100 + 10 * i).value_or("") == ERROR_LINE);
    }
}
