#ifndef JON_JON_H
#define JON_JON_H

#include <filesystem>
#include <fstream>

#include "Lexer.h"
#include "Parser.h"
#include "AstPrinter.h"

namespace jon {
    class jon {
    public:
        jon(const std::filesystem::path & path) {
            std::fstream file(path);

            if (not file.is_open()) {
                throw std::runtime_error(mstr("File '", path.string(), "' not found"));
            }

            std::stringstream ss;
            ss << file.rdbuf();
            file.close();
        }

    private:
        Lexer lexer;
        Parser parser;
    };
}

#endif // JON_JON_H
