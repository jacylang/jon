#ifndef JON_JON_H
#define JON_JON_H

#include <filesystem>
#include <fstream>
#include <variant>
#include <memory>
#include <string>
#include <assert.h>
#include <map>


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

        // Object access //
        const jon & operator[](const std::string & key) const {
            value.assertObjectFirstAccess(key);
        }

        jon & operator[](const std::string & key) {
            value.assertObjectFirstAccess(key);
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

        enum class Type {
            Null,
            Bool,
            Int,
            Float,
            String,
            Object,
            Array,
        };

        struct Value {
            using null_t = std::monostate;
            using bool_t = bool;
            using int_t = int64_t;
            using float_t = double;
            using str_t = std::string;
            using obj_t = std::map<str_t, Value>;
            using arr_t = std::vector<Value>;

            Value() = default;
            Value(bool_t v) noexcept : v(v), t(Type::Bool) {}
            Value(int_t v) noexcept : v(v), t(Type::Int) {}
            Value(float_t v) noexcept : v(v), t(Type::Float) {}

            Value(const str_t & v) noexcept : v(v), t(Type::String) {}
            Value(str_t && v) noexcept : v(std::move(v)), t(Type::String) {}

            Value(const obj_t & v) noexcept : v(v), t(Type::Object) {}
            Value(obj_t && v) noexcept : v(std::move(v)), t(Type::Object) {}

            Value(const arr_t & v) noexcept : v(v), t(Type::Array) {}
            Value(arr_t && v) noexcept : v(std::move(v)), t(Type::Array) {}

            Type type() const {
                return t;
            }

            void assertType(Type check, const std::string & errorMsg) const {
                if (this->type() != check) {
                    throw std::runtime_error(mstr("Type mismatch: ", errorMsg));
                }
            }

            void assertObjectFirstAccess(const std::string & key) const {
                assertType(val::Type::Object, mstr("cannot access property ", key, " of ", typeStr()));
            }

            std::string typeStr() const {
                switch (t) {
                    case Type::Null: {
                        return "null";
                    }
                    case Type::Bool:
                        return "boolean";
                    case Type::Int:
                        return "integer";
                    case Type::Float:
                        return "float";
                    case Type::String:
                        return "string";
                    case Type::Object:
                        return "object";
                    case Type::Array:
                        return "array";
                }
            }

            Type t;
            std::variant<null_t, bool_t, int_t, float_t, str_t, obj_t, arr_t> v;
        };

    private:
        Mode mode;

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
