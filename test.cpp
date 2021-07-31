#include "jon.h"

int main(const int argc, const char ** argv) {
    jon::jon file(std::filesystem::path("F:/projects/Jacy/JON/examples/sample_1.jon"));

    file.print();

    return 0;
}
