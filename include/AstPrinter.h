#ifndef JON_INCLUDE_ASTPRINTER_H
#define JON_INCLUDE_ASTPRINTER_H

#include <iostream>

#include "ast.h"
#include "utils.h"

namespace jon {
    class AstPrinter : public ValueVisitor {
    public:
        AstPrinter() {}
        ~AstPrinter() = default;

        void visit(const Null & null) override {
            out("null");
        }

        void visit(const Bool & bVal) override {
            out(bVal.val ? "true" : "false");
        }

        void visit(const Int & _int) override {
            out(_int.val);
        }

        void visit(const Float & _float) override {
            out(_float.val);
        }

        void visit(const String & string) override {
            out(string.val);
        }

        void visit(const Object & object) override {
            out("{");

            for (const auto & entry : object.entries) {
                out(entry.key.val, ": ");
            }

            out("}");
        }

    private:
        template<class ...Args>
        void out(Args && ...args) {
            std::cout << mstr(std::forward<Args>(args)...);
        }
    };
}

#endif // JON_INCLUDE_ASTPRINTER_H
