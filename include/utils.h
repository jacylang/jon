#ifndef JON_UTILS_H
#define JON_UTILS_H

#include <string>
#include <sstream>

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
}

#endif // JON_UTILS_H
