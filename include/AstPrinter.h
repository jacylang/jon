#ifndef JON_INCLUDE_ASTPRINTER_H
#define JON_INCLUDE_ASTPRINTER_H

#include <iostream>

#include "ast.h"
#include "utils.h"

namespace jon {
    class AstPrinter {
    public:
        AstPrinter() {}
        ~AstPrinter() = default;

        void print(const Null & null) {
            out("null");
        }

        void print(const Bool & bVal) {
            out(bVal.val ? "true" : "false");
        }

    private:
        template<class ...Args>
        void out(Args && ...args) {
            std::cout << mstr(std::forward<Args>(args)...);
        }
    };
}

#endif // JON_INCLUDE_ASTPRINTER_H
