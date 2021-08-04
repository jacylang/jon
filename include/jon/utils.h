#ifndef JON_UTILS_H
#define JON_UTILS_H

#include <string>
#include <sstream>

#include "error.h"

namespace jon {
    struct Indent {
        using size_type = int32_t;

        Indent(const std::string & val, size_type size = 0) : val(val), size(size) {}

        friend std::ostream & operator<<(std::ostream & os, const Indent & indent) {
            for (size_type i = 0; i < indent.size; i++) {
                os << indent.val;
            }
            return os;
        }

        friend Indent operator+(const Indent & indent, size_type offset) {
            if (indent.size == -1) {
                return indent;
            }
            return Indent(indent.val, indent.size + offset);
        }

        friend Indent operator-(const Indent & indent, size_type offset) {
            if (indent.size == -1) {
                return indent;
            }
            return Indent(indent.val, indent.size - offset);
        }

        const std::string val;
        const size_type size;
    };

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

    static inline std::string rtrim(const std::string & s) {
        std::string res = s;
        res.erase(std::find_if(res.rbegin(), res.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        }).base(), res.end());
        return res;
    }

    static inline uint8_t hexChar2Int(char c) {
        if (c >= '0' and c <= '9') {
            return static_cast<uint8_t>(c - '0');
        }

        if (c >= 'a' and c <= 'f') {
            return static_cast<uint8_t>(c - 'a') + 10;
        }

        if (c >= 'A' and c <= 'F') {
            return static_cast<uint8_t>(c - 'A') + 10;
        }

        throw jon::jon_exception("[jon bug]: Called `hexChar2Int` with not a hex digit char");
    }
}

#endif // JON_UTILS_H
