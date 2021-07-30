#ifndef JON_UTILS_H
#define JON_UTILS_H

#include <string>
#include <sstream>

namespace jon {
    // Merges arguments overloaded ostream::operator<<
    template<class ...Args>
    static std::string mstr(Args && ...args) {
        std::stringstream ss;
        ((ss << args), ...);
        return ss.str();
    }
}

#endif // JON_UTILS_H
