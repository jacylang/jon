#ifndef JON_JON_H
#define JON_JON_H

#include <filesystem>
#include <fstream>

#include "Lexer.h"
#include "Parser.h"
#include "Printer.h"

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
        jon() {
            static value_ptr null = std::make_unique<ast::Null>();
            value = null;
        }

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

        void print() {
            Printer printer;
            value->accept(printer);
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

            value = parser.parse(std::move(tokens));

            if (mode == Mode::Debug) {
                logDebug("AST:");
                value->accept(printer);
            }
        }

    private:
        Mode mode;
        value_ptr value;

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
