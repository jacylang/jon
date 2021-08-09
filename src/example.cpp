#include "jon/jon.h"

using jon = jacylang::jon;

int main(const int, const char**) {
    using namespace jacylang::literal;

    auto file = jon::fromFile("/home/hazer-hazer/dev/jacylang/jon/examples/sample_1.jon");

    std::cout << file.dump(2) << std::endl;

    auto val = file.at("value");
    auto schema = file.at("schema");

    std::cout << val.validate(schema).dump("  ") << std::endl;

    return 0;
}
