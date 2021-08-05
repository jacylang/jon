#include "doctest/doctest.h"
#include "jon/jon.h"

using jon = jacylang::jon;

TEST_CASE("static typeStr") {
    CHECK(jon::typeStr(jon::Type::Null) == "null");
    CHECK(jon::typeStr(jon::Type::Bool) == "bool");
    CHECK(jon::typeStr(jon::Type::Int) == "int");
    CHECK(jon::typeStr(jon::Type::Float) == "float");
    CHECK(jon::typeStr(jon::Type::String) == "string");
    CHECK(jon::typeStr(jon::Type::Object) == "object");
    CHECK(jon::typeStr(jon::Type::Array) == "array");

}
