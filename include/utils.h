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

    static std::string escstr(const std::string & str) {
        std::string escaped;
        for (const auto & c : str) {
            switch (c) {
                case '\n': {
                    escaped += "\\n";
                    break;
                }
                case '\r': {
                    escaped += "\\r";
                    break;
                }
                case '\t': {
                    escaped += "\\t";
                    break;
                }
                default: {
                    escaped += c;
                }
            }
        }
        return escaped;
    }
}

#endif // JON_UTILS_H
