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
        frag->insert("LET x = 2 ** 3");
        frag->insert("LET y = 3 ** 4");
        frag->insert("PRINT x");
        frag->insert("PRINT y");

        inter.interpret();

        CHECK(out.str() == "8\n81\n");
        CHECK(err.str() == "");
    }

    SUBCASE("divide") {
        frag->insert("LET x = 2 / 3");
        frag->insert("LET y = 100 / -3");
        frag->insert("PRINT x");
        frag->insert("PRINT y");

        inter.interpret();

        CHECK(out.str() == "0\n-33\n");
        CHECK(err.str() == "");
    }

    SUBCASE("modulo") {
        frag->insert("LET x = 2 MOD 3");
        frag->insert("LET y = 100 MOD -3");
        frag->insert("PRINT x");
        frag->insert("PRINT y");

        inter.interpret();

        CHECK(out.str() == "2\n1\n");
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
        frag->insert("LET x = 1");
        frag->insert("PRINT x");
        frag->insert("GOTO 4");
        frag->insert("LET x = x+2");
        frag->insert("PRINT x");

        inter.interpret();

        CHECK(out.str() == "1\n1\n");
    }

    SUBCASE("if statement") {
        frag->insert("LET T = 5");       // 0
        frag->insert("IF T = 0 THEN 5"); // 1
        frag->insert("PRINT T");         // 2
        frag->insert("LET T = T - 1");   // 3
        frag->insert("GOTO 1");          // 4
        frag->insert("END");             // 5

        inter.interpret();

        CHECK(out.str() == "5\n4\n3\n2\n1\n");
    }

    SUBCASE("condition") {
        frag->insert("LET x = 10");       // 0
        frag->insert("IF x < 9 THEN 4");  // 1
        frag->insert("IF x > 12 THEN 5"); // 2
        frag->insert("IF x > 8 THEN 6");  // 3
        frag->insert("PRINT x - 1");      // 4
        frag->insert("PRINT x ");         // 5
        frag->insert("PRINT x  + 1");     // 6

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
    Interpreter inter{frag, out, err, in, [&]() {
                          ++input_times;
                      }};

    frag->insert("INPUT x");
    frag->insert("PRINT x/2");

    inter.interpret();

    CHECK(out.str() == "10\n");
    CHECK(input_times == 1);
}

TEST_CASE("comments are lines") {
    auto frag = std::make_shared<Fragment>();
    std::ostringstream out{};
    std::ostringstream err{};
    std::istringstream in{};
    Interpreter inter{frag, out, err, in};

    frag->insert("LET x = 0");
    frag->insert("REM 1111111");
    frag->insert("REM 2222222");
    frag->insert("LET x = x+1");
    frag->insert("PRINT x");
    frag->insert("IF x < 5 THEN 1");
    frag->insert("IF x < 10 THEN 2");

    inter.interpret();

    CHECK(out.str() == "1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n");
    CHECK(err.str() == "");
}

TEST_CASE("error detection") {
    auto frag = std::make_shared<Fragment>();
    std::ostringstream out{};
    std::ostringstream err{};
    std::istringstream in{};
    Interpreter inter{frag, out, err, in};

    SUBCASE("negative exponent") {
        frag->insert("LET x = 2 ** -3");
        frag->insert("PRINT x");

        inter.interpret();

        CHECK(out.str() == "");
        CHECK(
            err.str() ==
            "Error at line 1, column 13: Unsupported negative exponent: -3\n");
    }

    SUBCASE("undefined variable") {
        frag->insert("PRINT x");

        inter.interpret();

        CHECK(out.str() == "");
        CHECK(err.str() ==
              "Error at line 1, column 6: Undefined variable: x\n");
    }

    SUBCASE("divided by zero") {
        frag->insert("LET x = -2 / -0");
        frag->insert("PRINT x");

        inter.interpret();

        CHECK(out.str() == "");
        CHECK(err.str() ==
              "Error at line 1, column 13: Division by zero: -2 / 0\n");
    }

    SUBCASE("modulo by zero") {
        frag->insert("LET x = -2 MOD 0");
        frag->insert("PRINT x");

        inter.interpret();

        CHECK(out.str() == "");
        CHECK(err.str() ==
              "Error at line 1, column 15: Modulus by zero: -2 MOD 0\n");
    }

    SUBCASE("invalid line number") {
        frag->insert("IF 2 > 1 THEN 20");
        frag->insert("GOTO 30");

        inter.interpret();

        CHECK(out.str() == "");
        CHECK(err.str() ==
              "Error at line 1, column 14: Invalid line number: 20\n"
              "Error at line 2, column 5: Invalid line number: 30\n");
    }
}
