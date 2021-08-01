#include "jon.h"

int main(const int argc, const char ** argv) {
    using namespace jon::literals;

    auto val = "key: 'value'"_jon;

    std::cout << val.stringify({" ", 2});

    return 0;
}
