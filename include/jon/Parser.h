#ifndef JON_PARSER_H
#define JON_PARSER_H

#include "Lexer.h"
#include "ast.h"

namespace jon {
    class Parser {
    public:
        Parser() {}
        ~Parser() = default;

        ast::value_ptr parse(const std::string & source) {
            index = 0;
            lastNl = 0;
            this->source = source;

            Lexer lexer;
            tokens = lexer.lex(source);

            return parseObject(true);
        }

    private:
        std::string source;
        TokenStream tokens;
        size_t index;
        size_t lastNl{0};

        const Token & peek() const {
            return tokens.at(index);
        }

        const Token & advance() {
            return tokens.at(index++);
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
                    lastNl = index;
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

        bool skipOptSep() {
            // Skip first new lines (optionally)
            bool nl = skipNls(true);

            // Skip comma, even if it goes after new lines, skipping next new lines optionally
            bool comma = skipOpt(TokenKind::Comma, true);

            return nl or comma;
        }

        void skipSep() {
            if (not skipOptSep()) {
                expectedError("delimiter: `,` or new line");
            }
        }

        ast::value_ptr parseValue() {
            switch (peek().kind) {
                case TokenKind::LBrace: {
                    return parseObject();
                }
                case TokenKind::LBracket: {
                    return parseArray();
                }
                case TokenKind::Null: {
                    advance();
                    return std::make_unique<ast::Null>();
                }
                case TokenKind::True:
                case TokenKind::False: {
                    return std::make_unique<ast::Bool>(advance().kind == TokenKind::True);
                }
                case TokenKind::BinInt: {
                    return std::make_unique<ast::Int>(std::stoul(advance().val, nullptr, 2));
                }
                case TokenKind::OctoInt: {
                    return std::make_unique<ast::Int>(std::stoul(advance().val, nullptr, 8));
                }
                case TokenKind::HexInt: {
                    return std::make_unique<ast::Int>(std::stoul(advance().val, nullptr, 16));
                }
                case TokenKind::DecInt: {
                    return std::make_unique<ast::Int>(std::stoul(advance().val));
                }
                case TokenKind::Float: {
                    return std::make_unique<ast::Float>(std::stod(advance().val));
                }
                case TokenKind::String: {
                    return std::make_unique<ast::String>(advance().val);
                }
                default: {
                    expectedError("value");
                }
            }
        }

        ast::value_ptr parseObject(bool root = false) {
            bool rootBraced = false;
            if (not root) {
                skip(TokenKind::LBrace, "[BUG] expected `{`", true); // Skip `{`
            } else {
                skipNls(true);
                rootBraced = skipOpt(TokenKind::LBrace, true);
            }

            bool first = true;
            ast::Object::Entries entries;
            while (not eof()) {
                if (is(TokenKind::RBrace)) {
                    break;
                }

                if (first) {
                    first = false;
                } else {
                    skipSep();
                }

                if (is(TokenKind::RBrace)) {
                    break;
                }

                auto key = ast::Ident {
                    skip(TokenKind::String, "key", true).val
                };
                skip(TokenKind::Colon, "`:` delimiter", true);
                auto val = parseValue();

                entries.emplace_back(ast::KeyValue{std::move(key), std::move(val)});
            }

            if (not root or rootBraced) {
                skip(TokenKind::RBrace, "closing `}`", false);
            }

            return std::make_unique<ast::Object>(std::move(entries));
        }

        ast::value_ptr parseArray() {
            skip(TokenKind::LBracket, "[BUG] expected `[`", true);

            bool first = true;
            ast::value_list values;
            while (not eof()) {
                if (is(TokenKind::RBracket)) {
                    break;
                }

                if (first) {
                    first = false;
                } else {
                    skipSep();
                }

                if (is(TokenKind::RBracket)) {
                    break;
                }

                values.emplace_back(parseValue());
            }

            skipOptSep(); // Trailing separator

            skip(TokenKind::RBracket, "Closing `]`", false);

            return std::make_unique<ast::Array>(std::move(values));
        }

        // Errors //
        void expectedError(const std::string & expected) {
            // TODO: Token to string
            throw std::runtime_error(mstr("Expected ", expected, ", got ", peek().toString()));
        }

        void error(const std::string & msg) {

        }
    };
}

#endif // JON_PARSER_H
