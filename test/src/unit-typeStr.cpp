#include "doctest/doctest.h"
#include "jon/jon.h"

using jon = jon::jon;

TEST_CASE("static typeStr") {
    CHECK(jon::typeStr())
}
