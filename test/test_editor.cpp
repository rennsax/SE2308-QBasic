#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "Fragment.h"
#include <doctest.h>

TEST_CASE("fragment manipulate") {
    basic::Fragment frag{};

    frag.append("hello");
    frag.append("world");

    CHECK(frag.size() == 2);
    CHECK(frag.render() == "100 hello\n110 world\n");

    REQUIRE(frag.remove(100));
    CHECK(frag.render() == "110 world\n");
    CHECK(!frag.get_line(100).has_value());

    frag.insert(105, "SJTU");
    frag.insert(115, "QT");
    frag.append("what?");

    CHECK(frag.render() == "105 SJTU\n110 world\n115 QT\n120 what?\n");
    CHECK(frag.get_line(115) == "QT");

    // Fragment should refuse to insert empty line.
    CHECK(frag.insert(200, "") == false);
    CHECK(frag.append("") == false);
}

TEST_CASE("read from file") {
    std::ifstream ifs{"test_cases/fibonacci.in"};
    basic::Fragment frag = basic::Fragment::read_stream(ifs);

    CHECK(frag.size() == 11);
    CHECK(frag.get_line(145) == "PRINT n1");

    frag.insert(195, "PRINT n2");

    CHECK(frag.size() == 12);
}