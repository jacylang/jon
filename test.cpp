#include "jon.h"

int main(const int argc, const char ** argv) {
    jon::jon file = jon::jon::fromFile(std::filesystem::path("F:/projects/Jacy/JON/examples/sample_1.jon"));

    std::cout << file["val"].get<int64_t>();

    return 0;
}
