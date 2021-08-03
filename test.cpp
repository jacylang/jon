#include "jon/jon.h"

int main(const int argc, const char ** argv) {
    using namespace jon::literal;

    auto val = R"(
        hello: [
            1
        ]
    )"_jon;

    auto schema = R"(
        type: 'object'
        props: {
            hello: {
                type: 'array'
                items: 'string'
            }
        }
    )"_jon;

    std::cout << schema.dump(2) << std::endl;

    std::cout << val.validate(schema).dump("  ") << std::endl;

    return 0;
}
