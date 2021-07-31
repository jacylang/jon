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

        void print() {

        }

    private:
        template<class ...Args>
        void print(Args && ...args) {
            std::cout << mstr(std::forward<Args>(args)...);
        }
    };
}

#endif // JON_INCLUDE_ASTPRINTER_H
