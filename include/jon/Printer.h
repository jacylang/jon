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
            nl();
            const auto ind = std::string(2, ' ');
            for (auto it = tokens.begin(); it != tokens.end(); it++) {
                out(ind, it->toString());
                out(" at ", it->span.pos, "; len = ", it->span.len);
                nl();
            }
            out("]");
            nl();
        }

        void printAst(const ast::value_ptr & ast) {
            indent = 0;
            ast->accept(*this);
        }

    private:
        uint16_t indent;

        void visit(const ast::Null & null) override {
            out("null");
        }

        void visit(const ast::Bool & bVal) override {
            out(bVal.val ? "true" : "false");
        }

        void visit(const ast::Int & _int) override {
            out(_int.val);
        }

        void visit(const ast::Float & _float) override {
            out(_float.val);
        }

        void visit(const ast::String & string) override {
            out("'", escstr(string.val), "'");
        }

        void visit(const ast::Object & object) override {
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

        void visit(const ast::Array & array) override {
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
