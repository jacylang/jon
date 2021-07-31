#ifndef JON_PARSER_H
#define JON_PARSER_H

#include "Lexer.h"
#include "ast.h"

namespace jon {
    class Parser {
    public:
        Parser() {}
        ~Parser() = default;

        value_ptr parse(TokenStream && tokens) {
            this->tokens = tokens;
            this->index = 0;

            return parseObject(true);
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
            return is(TokenKind::Eof);
        }

        bool is(TokenKind kind) const {
            return peek().kind == kind;
        }

        Token skip(TokenKind kind, const std::string & expected, bool rightNls) {
            auto token = peek();
            if (token.kind == kind) {
                advance();

                if (rightNls) {
                    skipNls(true);
                }

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
            } else if (not optional) {
                expectedError("new line");
            }
            return false;
        }

        bool skipOpt(TokenKind kind, bool rightNls = false) {
            if (peek().kind == kind) {
                advance();

                if (rightNls) {
                    skipNls(true);
                }

                return true;
            }

            return false;
        }

        void skipSep() {
            // Skip first new lines (optionally)
            bool nl = skipNls(true);

            // Skip comma, even if it goes after new lines, skipping next new lines optionally
            bool comma = skipOpt(TokenKind::Comma, true);

            if (not nl and not comma) {
                expectedError("delimiter: `,` or new line");
            }
        }

        value_ptr parseValue() {
            switch (peek().kind) {
                case TokenKind::LBrace: {
                    return parseObject();
                }
                case TokenKind::LBracket: {
                    return parseArray();
                }
                case TokenKind::Null: {
                    advance();
                    return std::make_unique<Null>();
                }
                case TokenKind::True:
                case TokenKind::False: {
                    auto boolVal = peek();
                    advance();
                    return std::make_unique<Bool>(boolVal.kind == TokenKind::True);
                }
                case TokenKind::Int: {
                    auto intVal = peek();
                    advance();
                    return std::make_unique<Int>(intVal.val);
                }
                case TokenKind::String: {
                    auto str = peek();
                    advance();
                    return std::make_unique<String>(str.val);
                }
                default: {
                    expectedError("value");
                }
            }
        }

        value_ptr parseObject(bool root = false) {
            bool rootBraced = false;
            if (not root) {
                skip(TokenKind::LBrace, "[BUG] expected `{`", true); // Skip `{`
            } else {
                rootBraced = skipOpt(TokenKind::LBrace);
            }

            Object::Entries entries;
            while (not eof()) {
                auto key = Ident {
                    skip(TokenKind::String, "key", true).val
                };
                skip(TokenKind::Colon, "`:` delimiter", true);
                auto val = parseValue();

                skipSep();

                entries.emplace_back(KeyValue{std::move(key), std::move(val)});
            }

            if (not root or rootBraced) {
                skip(TokenKind::RBrace, "closing `}`", false);
            }

            return std::make_unique<Object>(std::move(entries));
        }

        value_ptr parseArray() {
            skip(TokenKind::LBracket, "[BUG] expected `[`", true);

            value_list values;
            while (not eof()) {
                values.emplace_back(parseValue());
                skipSep();
            }

            skip(TokenKind::RBracket, "Closing `]`", false);

            return std::make_unique<Array>(std::move(values));
        }

        // Errors //
        void expectedError(const std::string & expected) {
            // TODO: Token to string
            throw std::runtime_error(mstr("Expected ", expected, ", got ", peek().toString()));
        }
    };
}

#endif // JON_PARSER_H
