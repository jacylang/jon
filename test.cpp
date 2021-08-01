#include "jon.h"

int main(const int argc, const char ** argv) {
    using namespace jon::literals;

    auto val = R"(
        {
            hello: {
                hi: 'killurself'
            }
        }
    )"_jon;

    std::cout << val.stringify({"  ", 0});

    return 0;
}
