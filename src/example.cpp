#include "jon/jon.h"

int main(const int, const char**) {
    using namespace jon::literal;

    auto file = jon::jon::fromFile("/home/hazer-hazer/dev/jacylang/jon/examples/sample_1.jon");

    auto val = file.at("value");
    auto schema = file.at("schema");

    std::cout << val.validate(schema).dump("  ") << std::endl;

    return 0;
}
