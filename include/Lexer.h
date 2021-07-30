#ifndef JON_LEXER_H
#define JON_LEXER_H

#include <string>
#include <stdexcept>

#include "utils.h"

namespace jon {
    class Lexer {
    public:
        Lexer(std::string && source) : source(std::move(source)) {}
        ~Lexer() = default;

        void lex() {
            while (not eof()) {
                lexCurrent();
            }
        }

    private:
        std::string source;

        size_t index;

        char peek() {
            return source.at(index);
        }

        char advance() {
            return source.at(++index);
        }

        char lookup() {
            return index < source.size() - 1 ? source.at(index + 1) : '\0';
        }

        bool eof() {
            return index >= source.size();
        }

        void lexCurrent() {
            switch (peek()) {
                case '/': {
                    return lexComment();
                }
            }
        }

        void lexComment() {
            if (peek() != '/') {
                throw std::runtime_error("Called `Lexer::lexComment` with not the '/' char");
            }
            if (lookup() == '*') {
            } else if (lookup() == '/') {

            }
        }

        // Errors //
        void unexpectedToken() {
            throw std::runtime_error(mstr("Unexpected token '", peek(), "'"));
        }
    };
}

#endif // JON_LEXER_H
