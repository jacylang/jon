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

        bool skipNls(bool optional) {
            if (is(TokenKind::NL)) {
                while (is(TokenKind::NL)) {
                    advance();
                }
                return true;
            }
            return false;
        }

        bool skipOpt(TokenKind kind) {
            if (peek().kind == kind) {
                advance();
                return true;
            }

            return false;
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

        value_ptr parseObject(bool root = false) {
            bool rootBraced = false;
            if (not root) {
                skip(TokenKind::LBrace, "[BUG] expected `{`"); // Skip `{`
            } else {
                rootBraced = skipOpt(TokenKind::LBrace);
            }

            Object::Entries entries;
            while (not eof()) {
                auto key = Ident {
                    skip(TokenKind::String, "key").val
                };
                skip(TokenKind::Colon, "`:` delimiter");
                auto val = parseValue();

                entries.emplace_back(KeyValue{std::move(key), std::move(val)});
            }

            if (not root or rootBraced) {
                skip(TokenKind::RBrace, "closing `}`");
            }

            return std::make_unique<Object>(std::move(entries));
        }

        value_ptr parseArray() {
            skip(TokenKind::LBracket, "[BUG] expected `[`");

            value_list values;
            while (not eof()) {
                values.emplace_back(parseValue());
            }

            skip(TokenKind::RBracket, "Closing `]`");
        }

        // Errors //
        void expectedError(const std::string & expected) {
            // TODO: Token to string
            throw std::runtime_error(mstr("Expected ", expected, ", got ", peek().toString()));
        }
    };
}

#endif // JON_PARSER_H
