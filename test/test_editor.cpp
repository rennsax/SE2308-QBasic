#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "Fragment.h"
#include <doctest.h>

TEST_CASE("fragment") {
    basic::Fragment frag{};

    frag.insert("hello");
    frag.insert("world");

    SUBCASE("insertion") {
        CHECK(frag.size() == 2);
        CHECK(frag.render() == "0 hello\n1 world\n");
        CHECK(frag.get_frag_stream().str() == "hello\nworld\n");
    }

    SUBCASE("remove") {
        REQUIRE(frag.remove(0));
        CHECK(frag.render() == "0 world\n");
        CHECK(frag.get_frag_stream().str() == "world\n");
        CHECK(!frag.get_line(1));
    }
}
