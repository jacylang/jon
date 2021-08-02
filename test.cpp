#include "jon/jon.h"

int main(const int argc, const char ** argv) {
    using namespace jon::literal;

    auto val = R"(
        hello: {
            hi: {
                hi: 'killurself'
            }
        }
    )"_jon;

    auto schema = R"(
        type: 'object'
        props: {

        }
    )"_jon;

    std::cout << val.validate(schema);

    return 0;
}
