#ifndef JON_JON_H
#define JON_JON_H

#include <filesystem>
#include <fstream>

#include "Lexer.h"
#include "Parser.h"
#include "Printer.h"

namespace jon {
    enum class Mode {
        Debug,
    };

    class jon {
    public:
        jon(const std::filesystem::path & path, Mode mode) {
            this->mode = mode;

            std::fstream file(path);

            if (not file.is_open()) {
                throw std::runtime_error(mstr("File '", path.string(), "' not found"));
            }

            std::stringstream ss;
            ss << file.rdbuf();
            file.close();

            source = std::move(ss.str());

            init();
        }

        void print() {
            ast->accept(printer);
        }

    private:
        void init() {
            auto tokens = lexer.lex(std::move(source));
            ast = parser.parse(std::move(tokens));
        }

    private:
        Mode mode;
        std::string source;
        Lexer lexer;
        Parser parser;
        Printer printer;
        value_ptr ast;
    };
}

#endif // JON_JON_H
