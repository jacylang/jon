#ifndef JON_PRINTER_H
#define JON_PRINTER_H

#include <iostream>

#include "ast.h"
#include "utils.h"

namespace jon {
    class Printer : public ValueVisitor {
    public:
        Printer() {}
        ~Printer() = default;

        void printTokens(const TokenStream & tokens) {
            for (const auto & token : tokens) {
                out(token.toString(), ", ");
            }
        }

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
            nl();

            for (const auto & entry : object.entries) {
                out(entry.key.val, ": ");
                entry.val->accept(*this);
                nl();
            }

            nl();
            out("}");
        }

        void visit(const Array & array) override {
            out("[");
            nl();

            for (const auto & value : array.values) {
                value->accept(*this);
            }

            nl();
            out("]");
        }

    private:
        template<class ...Args>
        void out(Args && ...args) {
            std::cout << mstr(std::forward<Args>(args)...);
        }

        void nl() {
            std::cout << std::endl;
        }
    };
}

#endif // JON_PRINTER_H
