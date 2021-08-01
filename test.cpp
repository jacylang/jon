#include "jon.h"

int main(const int argc, const char ** argv) {
    using namespace jon;

    auto val = "key: 'value'"_jon;

    std::cout << val["key"].get<std::string>();

    return 0;
}
