#ifndef JON_LEXER_H
#define JON_LEXER_H

#include <string>
#include <stdexcept>
#include <vector>
#include <iostream>
#include <assert.h>

#include "utils.h"

namespace jon {
    struct Token;
    using TokenStream = std::vector<Token>;

    enum class TokenKind {
        Eof,

        NL,

        // Punctuations
        Comma,
        Colon,
        LBrace,
        RBrace,
        LBracket,
        RBracket,

        Null,
        False,
        True,

        BinInt,
        HexInt,
        OctoInt,
        DecInt,

        Float,

        /// Either string enclosed into quotes (maybe triple if multi-line) or identifier
        // Note: Separate with identifier if would be needed
        String,
    };

    struct Span {
        using pos_t = size_t;
        using len_t = size_t;

        pos_t pos;
        len_t len;
    };

    struct Token {
        Token(TokenKind kind, const std::string & val, const Span & span) : kind(kind), val(val) {}

        TokenKind kind;
        std::string val;

        std::string toString() const {
            switch (kind) {
                case TokenKind::Eof: {
                    return "[EOF]";
                }
                case TokenKind::NL: {
                    return "new line";
                }
                case TokenKind::Comma: {
                    return "`,`";
                }
                case TokenKind::Colon: {
                    return "`:`";
                }
                case TokenKind::LBrace: {
                    return "`{`";
                }
                case TokenKind::RBrace: {
                    return "`}`";
                }
                case TokenKind::LBracket: {
                    return "`[`";
                }
                case TokenKind::RBracket: {
                    return "`]`";
                }
                case TokenKind::Null: {
                    return "`null`";
                }
                case TokenKind::False: {
                    return "`false`";
                }
                case TokenKind::True: {
                    return "`true`";
                }
                case TokenKind::String: {
                    return "string '" + escstr(val) + "'";
                }
                case TokenKind::BinInt: {
                    return "number `0b" + val + "`";
                }
                case TokenKind::OctoInt: {
                    return "number `0o" + val + "`";
                }
                case TokenKind::HexInt: {
                    return "number `0x" + val + "`";
                }
                case TokenKind::DecInt: {
                    return "number `" + val + "`";
                }
                case TokenKind::Float: {
                    return "number `" + val + "`";
                }
            }
        }

        uint8_t intBase() const {
            switch (kind) {
                case TokenKind::DecInt: {
                    return 0;
                }
                case TokenKind::HexInt: {
                    return 16;
                }
                case TokenKind::OctoInt: {
                    return 8;
                }
                case TokenKind::BinInt: {
                    return 2;
                }
                default: {
                    throw std::logic_error("Called `Token::intBase` with non-int token");
                }
            }
        }
    };

    class Lexer {
    public:
        Lexer() = default;
        ~Lexer() = default;

        TokenStream lex(const std::string & source) {
            this->source = source;
            index = 0;
            lastNl = 0;
            col = 0;
            tokens.clear();

            while (not eof()) {
                tokenPos = index;
                lexCurrent();
            }

            tokenPos = index;
            addTokenAdvance(TokenKind::Eof, 1);

            return std::move(tokens);
        }

    private:
        std::string source;

        size_t index{0};
        size_t lastNl{0};
        uint16_t col{0};
        Span::pos_t tokenPos;

        char peek() {
            return source.at(index);
        }

        char advance(uint8_t dist = 1) {
            auto cur = peek();
            for (uint8_t i = 0; i < dist; i++) {
                if (isNL()) {
                    lastNl = index;
                    col = 0;
                } else {
                    col++;
                }
                index++;
            }
            if (eof()) {
                return '\0';
            }
            return cur;
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
            uint8_t offset{0};
            return (... and (lookup(offset++) == chars));
        }

        bool is(char c) {
            return peek() == c;
        }

        template<class ...Args>
        bool isAnyOf(Args && ...chars) {
            return (... or is(chars));
        }

        bool isHidden() {
            return isAnyOf(' ', '\t', '\r');
        }

        bool isDigit() {
            return peek() >= '0' and peek() <= '9';
        }

        bool isHexDigit() {
            return isDigit()
                or peek() >= 'a' and peek() <= 'f'
                or peek() >= 'A' and peek() <= 'F';
        }

        bool isOctDigit() {
            return peek() >= '0' and peek() <= '7';
        }

        void skip(char c) {
            if (peek() != c) {
                expectedError("`" + mstr(c) + "`");
            }
            advance();
        }

        bool skipOpt(char c) {
            if (is(c)) {
                advance();
                return true;
            }
            return false;
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
                    addTokenAdvance(TokenKind::Comma, 1);
                    break;
                }
                case ':': {
                    addTokenAdvance(TokenKind::Colon, 1);
                    break;
                }
                case '{': {
                    addTokenAdvance(TokenKind::LBrace, 1);
                    break;
                }
                case '}': {
                    addTokenAdvance(TokenKind::RBrace, 1);
                    break;
                }
                case '[': {
                    addTokenAdvance(TokenKind::LBracket, 1);
                    break;
                }
                case ']': {
                    addTokenAdvance(TokenKind::RBracket, 1);
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

            bool closed = false;
            std::string val;
            while (not eof()) {
                if (isSeq(quote, quote, quote)) {
                    closed = true;
                    break;
                }
                val += advance();
            }

            if (!closed) {
                expectedError(mstr(quote, quote, quote));
            }

            advance(3);

            addToken(TokenKind::String, val);
        }

        void lexNormalString() {
            const auto quote = advance();

            std::string val;
            while (not eof()) {
                if (isNL() or is(quote)) {
                    break;
                }
                val += advance();
            }

            skip(quote);

            addToken(TokenKind::String, val);
        }

        void lexNum() {
            TokenKind kind;
            std::string val;

            bool baseSpecific = false;

            // Binary //
            if (peek() == '0' and (lookup() == 'b' or lookup() == 'B')) {
                baseSpecific = true;

                advance(2);
                if (not isAnyOf('0', '1')) {
                    expectedError("binary digit");
                }
                while (not eof()) {
                    skipOpt('_');
                    if (not isAnyOf('0', '1')) {
                        break;
                    }
                    val += advance();
                }

                kind = TokenKind::OctoInt;
            }

            // Hexadecimal //
            if (peek() == '0' and (lookup() == 'x' or lookup() == 'X')) {
                baseSpecific = true;

                advance(2);
                if (not isHexDigit()) {
                    expectedError("hexadecimal digit");
                }
                while (not eof()) {
                    skipOpt('_');
                    if (not isHexDigit()) {
                        break;
                    }
                    val += advance();
                }

                kind = TokenKind::HexInt;
            }

            // Octodecimal //
            if (peek() == '0' and (lookup() == 'o' or lookup() == 'O')) {
                baseSpecific = true;

                advance(2);
                if (not isOctDigit()) {
                    expectedError("octodecimal digit");
                }
                while (not eof()) {
                    skipOpt('_');
                    if (not isOctDigit()) {
                        break;
                    }
                    val += advance();
                }

                kind = TokenKind::OctoInt;
            }

            if (not baseSpecific) {
                while (not eof()) {
                    skipOpt('_');
                    if (not isDigit()) {
                        break;
                    }
                    val += advance();
                }

                kind = TokenKind::DecInt;

                if (is('.')) {
                    val += advance();
                    if (not isDigit()) {
                        expectedError("fractional part of number");
                    }
                    while (not eof()) {
                        skipOpt('_');
                        if (not isDigit()) {
                            break;
                        }
                        val += advance();
                    }
                    kind = TokenKind::Float;
                }
            }

            addToken(kind, val);
        }

        void lexMisc() {
            if (isNL()) {
                addTokenAdvance(TokenKind::NL, 1);
                return;
            }

            if (isHidden()) {
                advance();
                return;
            }

            if (isSeq('n', 'u', 'l', 'l')) {
                addTokenAdvance(TokenKind::Null, 4);
                return;
            }

            if (isSeq('f', 'a', 'l', 's', 'e')) {
                addTokenAdvance(TokenKind::False, 5);
                return;
            }

            if (isSeq('t', 'r', 'u', 'e')) {
                addTokenAdvance(TokenKind::True, 4);
                return;
            }

            if (isDigit()) {
                return lexNum();
            }

            // Identifier is anything not containing specific tokens
            std::string val;
            while (not eof()) {
                if (isAnyOf(',', ':', '{', '}', '[', ']', '\'', '"') or isNL()) {
                    break;
                }
                val += advance();
            }

            // Add identifier as string
            addToken(TokenKind::String, std::move(val));
        }

        // Tokens //
        TokenStream tokens;

        void addToken(TokenKind kind, const std::string & val) {
            tokens.emplace_back(kind, val, Span {tokenPos, val.size()});
        }

        void addTokenAdvance(TokenKind kind, Span::len_t len) {
            advance(len);
            tokens.emplace_back(kind, "", Span {tokenPos, len});
        }

        // Errors //
        void unexpectedToken() {
            throw std::runtime_error(mstr("Unexpected token '", peek(), "'"));
        }

        void expectedError(const std::string & expected) {
            std::string got;
            if (isNL()) {
                got = "new line";
            } else {
                got = mstr("`", peek(), "`");
            }

            size_t sliceTo = index;
            while (not eof()) {
                sliceTo = index;
                if (isNL()) {
                    break;
                }
                advance();
            }

            const auto msg = mstr("Expected ", expected, ", got ", got);

            const auto & line = source.substr(lastNl, sliceTo - lastNl);
            std::string pointLine;
            if (msg.size() + 2 < col) {
                pointLine = std::string(col - msg.size() - 1, ' ') + msg + " ^";
            } else {
                pointLine = std::string(col, ' ') + "^ " + msg;
            }

            throw std::runtime_error(
                mstr("\n", line, "\n", pointLine)
            );
        }
    };
}

#endif // JON_LEXER_H
