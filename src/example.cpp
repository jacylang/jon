#include "jon/jon.h"

using jon = jacylang::jon;

struct S {
    S(jon::int_t field, std::string kek) : field {field}, kek {kek} {}

    jon::int_t field;
    jon::str_t kek;

    static S fromJon(const jon & val) {
        return S {val.at<jon::int_t>("num"), val.at<jon::str_t>("str")};
    }
};

int main(const int, const char**) {
    using namespace jacylang::literal;

    auto file = jon::fromFile("../../examples/sample_1.jon", true);

    std::cout << file.typeStr() << std::endl;

    S s = file.at<jon::int_t>("num");

//    auto val = file.at("value");
//    auto schema = file.at("schema");
//
//    std::cout << val.validate(schema).dump("  ") << std::endl;

    return 0;
}
