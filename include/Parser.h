#ifndef JON_PARSER_H
#define JON_PARSER_H

#include "Lexer.h"

namespace jon {
    class Parser {
    public:
        Parser() {}
        ~Parser() = default;

        void parse() {

        }

    private:
        TokenStream tokens;
    };
}

#endif // JON_PARSER_H
