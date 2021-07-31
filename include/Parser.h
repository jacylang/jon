#ifndef JON_PARSER_H
#define JON_PARSER_H

#include "Lexer.h"
#include "ast.h"

namespace jon {
    class Parser {
    public:
        Parser() {}
        ~Parser() = default;

        void parse() {

        }

    private:
        TokenStream tokens;
        size_t index;

        Token peek() const {
            return tokens.at(index);
        }

        Token advance() {
            return tokens.at(++index);
        }

        bool eof() const {
            return index >= tokens.size();
        }

        bool is(TokenKind kind) const {
            return peek().kind == kind;
        }

        Token skip(TokenKind kind, const std::string & expected) {
            auto token = peek();
            if (token.kind == kind) {
                advance();
                return token;
            }
            expectedError(expected);
        }

        value_ptr parseValue() {
            switch (peek().kind) {
                case TokenKind::LBrace: {
                    return parseObject();
                }
                case TokenKind::LBracket: {

                }
                case TokenKind::String: {

                }
                default: {
                    expectedError("value");
                }
            }
        }

        value_ptr parseObject() {
            advance(); // Skip `{`
            auto object = parseObjectInner();
            skip(TokenKind::RBrace, "closing `}`");
            return object;
        }

        value_ptr parseObjectInner() {

        }

        // Errors //
        void expectedError(const std::string & expected) {
            // TODO: Token to string
            throw std::runtime_error(mstr("Expected ", expected, ", got ", peek().toString()));
        }
    };
}

#endif // JON_PARSER_H
