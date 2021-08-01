#ifndef JON_JON_H
#define JON_JON_H

#include <filesystem>
#include <fstream>

#include "Lexer.h"
#include "Parser.h"
#include "Printer.h"
#include "SerDes.h"

namespace jon {
    enum class Mode : uint8_t {
        Default,

        Debug,
    };

    inline Mode operator|(Mode lhs, Mode rhs) {
        return static_cast<Mode>(
            static_cast<std::underlying_type_t<Mode>>(lhs) | static_cast<std::underlying_type_t<Mode>>(rhs)
        );
    }

    class jon {
    public:
        jon() {}

        jon(const std::filesystem::path & path, Mode mode = Mode::Default) {
            this->mode = mode;

            std::fstream file(path);

            if (not file.is_open()) {
                throw std::runtime_error(mstr("File '", path.string(), "' not found"));
            }

            std::stringstream ss;
            ss << file.rdbuf();
            file.close();

            fromSource(ss.str());
        }

        jon & operator[](const std::string & key) {
            if (value.type() != val::Type::Object) {
                throw std::runtime_error(mstr("Type mismatch: cannot set property ", key, " of "));
            }
        }

    private:
        void fromSource(const std::string & source) {
            Lexer lexer;
            Parser parser;
            Printer printer;

            logDebug("Lexing...");

            auto tokens = lexer.lex(source);

            if (mode == Mode::Debug) {
                printer.printTokens(tokens);
            }

            logDebug("Parsing...");

            auto ast = parser.parse(std::move(tokens));

            if (mode == Mode::Debug) {
                logDebug("AST:");
                ast->accept(printer);
            }

            value = SerDes::fromAst(std::move(ast));
        }

    private:
        Mode mode;
        val::Value value;

    private:
        template<class ...Args>
        void logDebug(Args && ...args) {
            if (mode != Mode::Debug) {
                return;
            }
            std::cout << mstr(std::forward<Args>(args)...) << std::endl;
        }
    };
}

#endif // JON_JON_H
