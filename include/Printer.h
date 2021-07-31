#ifndef JON_PRINTER_H
#define JON_PRINTER_H

#include <iostream>

#include "ast.h"
#include "utils.h"

namespace jon {
    class Printer : public ast::ValueVisitor {
    public:
        Printer() {}
        ~Printer() = default;

        void printTokens(const TokenStream & tokens) {
            out("Tokens: [");
            for (auto it = tokens.begin(); it != tokens.end(); it++) {
                out(it->toString());

                if (it != std::prev(tokens.end())) {
                    out(", ");
                }
            }
            out("]");
            nl();
        }

        void printAst(const value_ptr & ast) {
            indent = 0;
            ast->accept(*this);
        }

    private:
        uint16_t indent;

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
            out("'", escstr(string.val), "'");
        }

        void visit(const Object & object) override {
            out("{");
            nl();
            indent++;

            for (const auto & entry : object.entries) {
                printIndent();
                out(escstr(entry.key.val), ": ");
                entry.val->accept(*this);
                nl();
            }

            indent--;
            out("}");
        }

        void visit(const Array & array) override {
            out("[");
            nl();
            indent++;

            for (const auto & value : array.values) {
                printIndent();
                value->accept(*this);
            }

            indent--;
            nl();
            out("]");
        }

    public:
        template<class ...Args>
        void out(Args && ...args) {
            std::cout << mstr(std::forward<Args>(args)...);
        }

        void nl() {
            std::cout << std::endl;
        }

    private:
        void printIndent() {
            out(std::string(indent * 2, ' '));
        }
    };
}

#endif // JON_PRINTER_H
