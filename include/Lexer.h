#ifndef JON_LEXER_H
#define JON_LEXER_H

#include <string>
#include <stdexcept>
#include <vector>

#include "utils.h"

namespace jon {
    struct Token;
    using TokenStream = std::vector<Token>;

    enum class TokenKind {
        NL,

        // Punctuations
        Comma,
        Colon,
        LBrace,
        RBrace,
        LBracket,
        RBracket,

        /// Either string enclosed into quotes (maybe triple if multi-line) or identifier
        // Note: Separate with identifier if would be needed
        String,
    };

    struct Token {
        Token(TokenKind kind, const std::string & val) : kind(kind), val(val) {}

        TokenKind kind;
        std::string val;
    };

    class Lexer {
    public:
        Lexer(std::string && source) : source(std::move(source)) {}
        ~Lexer() = default;

        TokenStream lex() {
            while (not eof()) {
                lexCurrent();
            }
            return std::move(tokens);
        }

    private:
        std::string source;

        size_t index;

        char peek() {
            return source.at(index);
        }

        char advance(uint8_t dist = 1) {
            index += dist;
            return peek();
        }

        char lookup(uint8_t dist = 1) {
            return index < source.size() - dist ? source.at(index + dist) : '\0';
        }

        bool eof() {
            return index >= source.size();
        }

        bool isNL() {
            // TODO: Handle '\r' and '\r\n'
            return peek() == '\n';
        }

        template<class ...Args>
        bool isSeq(Args && ...chars) {
            return (... and lookup(chars));
        }

        bool is(char c) {
            return peek() == c;
        }

        template<class ...Args>
        bool isAnyOf(Args && ...chars) {
            return (... or is(chars));
        }

        void lexCurrent() {
            switch (peek()) {
                case '/': {
                    return lexComment();
                }
                case '\'':
                case '"': {
                    return lexString();
                }
                case ',': {
                    addToken(TokenKind::Comma);
                    break;
                }
                case ':': {
                    addToken(TokenKind::Colon);
                    break;
                }
                case '{': {
                    addToken(TokenKind::LBrace);
                    break;
                }
                case '}': {
                    addToken(TokenKind::RBrace);
                    break;
                }
                case '[': {
                    addToken(TokenKind::LBracket);
                    break;
                }
                case ']': {
                    addToken(TokenKind::RBracket);
                    break;
                }
                default: {
                    return lexMisc();
                }
            }
        }

        void lexComment() {
            if (peek() != '/') {
                throw std::runtime_error("Called `Lexer::lexComment` with not the '/' char");
            }
            if (lookup() == '*') {
                advance(2);

                // Parse block comment handling nested
                uint8_t depth{1};
                while (not eof()) {
                    if (peek() == '/' and lookup() == '*') {
                        depth++;
                    }

                    if (peek() == '*' and lookup() == '/') {
                        depth--;
                    }

                    if (depth == 0) {
                        break;
                    }
                }
            } else if (lookup() == '/') {
                while (not eof()) {
                    advance();
                    if (isNL()) {
                        break;
                    }
                }
            }
        }

        void lexString() {
            const auto quote = peek();
            if (isSeq(quote, quote, quote)) {
                return lexMLString();
            }

            return lexNormalString();
        }

        void lexMLString() {
            const auto quote = peek();
            // Note: Skip triple quote
            advance(3);

            std::string val;
            while (not eof()) {
                if (isSeq(quote, quote, quote)) {
                    break;
                }
                val += peek();
            }

            addToken(TokenKind::String, val);
        }

        void lexNormalString() {
            const auto quote = peek();

            // Skip quote
            advance();

            std::string val;
            while (not eof()) {
                if (isNL()) {
                    expectedError("' at the end of string");
                }
                if (peek() == quote) {
                    break;
                }
                val += peek();
            }

            addToken(TokenKind::String, val);
        }

        void lexMisc() {
            if (isNL()) {
                addToken(TokenKind::NL);
                return;
            }

            // Identifier is anything not containing specific tokens
            std::string val;
            while (not eof()) {
                if (isAnyOf(',', ':', '{', '}', '[', ']', '\'', '"') or isNL()) {
                    break;
                }
                val += peek();
            }

            // Add identifier as string
            addToken(TokenKind::String, val);
        }

        // Tokens //
        TokenStream tokens;

        void addToken(TokenKind kind, const std::string & val = "") {
            tokens.emplace_back(kind, val);
        }

        // Errors //
        void unexpectedToken() {
            throw std::runtime_error(mstr("Unexpected token '", peek(), "'"));
        }

        void expectedError(const std::string & msg) {
            throw std::runtime_error(mstr("Expected ", msg, ", got `", peek(), "`"));
        }
    };
}

#endif // JON_LEXER_H
