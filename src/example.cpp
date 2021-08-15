#include "jon/jon.h"

using jon = jacylang::jon;
using namespace jacylang::literal;

const static auto config = R"(
true: 'kek'
false: 'lol'
    )"_jon;

int main(const int, const char**) {
    using namespace jacylang::literal;

    std::cout << config.dump(2) << std::endl;

    return 0;
}
