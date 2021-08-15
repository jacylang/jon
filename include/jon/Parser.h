#ifndef JON_PARSER_H
#define JON_PARSER_H

#include "Lexer.h"
#include "Printer.h"
#include "ast.h"

namespace jacylang {
    class Parser {
    public:
        Parser() {}
        ~Parser() = default;

        ast::value_ptr parse(const std::string & source, bool debug) {
            index = 0;
            lastNl = 0;
            this->source = source;

            Lexer lexer;
            tokens = lexer.lex(source);

            skipNls(true);

            auto ast = parseValue(true);
            if (debug) {
                Printer printer;
                printer.printTokens(tokens);
                printer.printAst(ast);
            }

            return ast;
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

        bool isKey() const {
            switch (peek().kind) {
                case TokenKind::Null:
                case TokenKind::False:
                case TokenKind::True:
                case TokenKind::NaN:
                case TokenKind::PosNaN:
                case TokenKind::NegNaN:
                case TokenKind::Inf:
                case TokenKind::PosInf:
                case TokenKind::NegInf:
                case TokenKind::BinInt:
                case TokenKind::HexInt:
                case TokenKind::OctoInt:
                case TokenKind::DecInt:
                case TokenKind::Float:
                case TokenKind::String: {
                    return true;
                }
                case TokenKind::LBrace:
                case TokenKind::Eof:
                case TokenKind::NL:
                case TokenKind::Comma:
                case TokenKind::Colon:
                case TokenKind::RBrace:
                case TokenKind::LBracket:
                case TokenKind::RBracket: {
                    return false;
                }
                default: {
                    throw std::logic_error("Unhandled `TokenKind` in `Parser::isKey`");
                }
            }
        }

        bool lookupIs(TokenKind kind) const {
            if (eof()) {
                return false;
            }
            return tokens.at(index + 1).kind == kind;
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

        ast::Ident parseKey() {
            switch (peek().kind) {
                case TokenKind::String: {
                    return ast::Ident {
                        skip(TokenKind::String, "[jon bug] key", true).val
                    };
                }
                case TokenKind::Null: {
                    return ast::Ident {"null"};
                }
                case TokenKind::False: {
                    return ast::Ident {"false"};
                }
                case TokenKind::True: {
                    return ast::Ident {"true"};
                }
                case TokenKind::NaN: {
                    return ast::Ident {"nan"};
                }
                case TokenKind::PosNaN: {
                    return ast::Ident {"+nan"};
                }
                case TokenKind::NegNaN: {
                    return ast::Ident {"-nan"};
                }
                case TokenKind::Inf: {
                    return ast::Ident {"inf"};
                }
                case TokenKind::PosInf: {
                    return ast::Ident {"+inf"};
                }
                case TokenKind::NegInf: {
                    return ast::Ident {"-inf"};
                }
                case TokenKind::DecInt:
                case TokenKind::BinInt:
                case TokenKind::HexInt:
                case TokenKind::OctoInt:
                case TokenKind::Float: {
                    return ast::Ident {advance().val};
                }
                case TokenKind::Ref: {
                    return ast::Ident {"$" + advance().val};
                }
                case TokenKind::Eof:
                case TokenKind::NL:
                case TokenKind::Comma:
                case TokenKind::Colon:
                case TokenKind::LBrace:
                case TokenKind::RBrace:
                case TokenKind::LBracket:
                case TokenKind::RBracket: {
                    expectedError("key");
                }
                default: {
                    throw std::logic_error("Unhandled `TokenKind` in `Parser::parseKey`");
                }
            }
        }

        ast::value_ptr parseValue(bool root = false) {
            if (root and isKey() and lookupIs(TokenKind::Colon)) {
                return parseObject(true);
            }

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
                case TokenKind::NaN:
                case TokenKind::PosNaN:
                case TokenKind::NegNaN: {
                    advance();
                    return std::make_unique<ast::Float>(std::numeric_limits<double>::quiet_NaN());
                }
                case TokenKind::Inf:
                case TokenKind::NegInf:
                case TokenKind::PosInf: {
                    return std::make_unique<ast::Float>(
                        std::numeric_limits<double>::infinity() * (advance().kind == TokenKind::NegInf ? -1 : 1)
                    );
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
                case TokenKind::Ref: {
                    return std::make_unique<ast::Ref>(ast::Ident {advance().val});
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

                if (is(TokenKind::RBrace) or eof()) {
                    break;
                }

                auto key = parseKey();
                skipNls(true);
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

                if (is(TokenKind::RBracket) or eof()) {
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
            error(mstr("Expected ", expected, ", got ", peek().toString()));
        }

        void error(const std::string & msg) {
            size_t errorIndex = index;
            size_t sliceTo = index;
            while (not eof()) {
                if (is(TokenKind::NL)) {
                    sliceTo = index;
                    break;
                }
                advance();
            }

            const auto & lastNlPos = tokens.at(lastNl).span.pos;
            const auto & line = source.substr(lastNlPos, tokens.at(sliceTo).span.pos - lastNlPos);
            const auto col = tokens.at(errorIndex).span.pos - lastNlPos;

            std::string pointLine;
            if (msg.size() + 2 < col) {
                pointLine = std::string(col - msg.size() - 1, ' ') + msg + " ^";
            } else {
                pointLine = std::string(col, ' ') + "^ " + msg;
            }

            throw parse_error(
                mstr("(Parsing error)", line, "\n", pointLine)
            );
        }
    };
}

#endif // JON_PARSER_H
