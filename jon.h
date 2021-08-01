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

namespace jon {
    class jon {
        using null_t = std::monostate;
        using bool_t = bool;
        using int_t = int64_t;
        using float_t = double;
        using str_t = std::string;
        using obj_t = std::map<str_t, jon>;
        using arr_t = std::vector<jon>;

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

            Type type() const noexcept {
                return t;
            }

            void assertType(Type check, const std::string & errorMsg) const {
                if (this->type() != check) {
                    throw std::runtime_error(mstr("Type mismatch: ", errorMsg));
                }
            }

            void assertObjectFirstAccess(const std::string & key) {
                if (t == Type::Null) {
                    v = obj_t {};
                } else {
                    assertType(Type::Object, mstr("cannot access property ", key, " of ", typeStr()));
                }
            }

            template<class T>
            T & get() noexcept {
                return std::get<T>(v);
            }

            template<class T>
            const T & get() const noexcept {
                return std::get<T>(v);
            }

            std::string typeStr() const {
                switch (t) {
                    case Type::Null: return "null";
                    case Type::Bool: return "boolean";
                    case Type::Int: return "integer";
                    case Type::Float: return "float";
                    case Type::String: return "string";
                    case Type::Object: return "object";
                    case Type::Array: return "array";
                }
            }

            Type t;
            std::variant<null_t, bool_t, int_t, float_t, str_t, obj_t, arr_t> v;
        };

    private:
        jon(Value && value) : value(std::move(value)) {}

        // Constructors //
    public:
        explicit jon() = default;
        explicit jon(null_t) {}
        explicit jon(bool_t v) noexcept : value(v) {}
        explicit jon(int_t v) noexcept : value(v) {}
        explicit jon(float_t v) noexcept : value(v) {}

        explicit jon(const str_t & v) noexcept : value(v) {}
        explicit jon(str_t && v) noexcept : value(std::move(v)) {}

        explicit jon(const obj_t & v) noexcept : value(v) {}
        explicit jon(obj_t && v) noexcept : value(std::move(v)) {}

        explicit jon(const arr_t & v) noexcept : value(v) {}
        explicit jon(arr_t && v) noexcept : value(std::move(v)) {}

        // Custom constructors //
    public:
        static jon fromFile(const std::filesystem::path & path) {
            std::fstream file(path);

            if (not file.is_open()) {
                throw std::runtime_error(mstr("File '", path.string(), "' not found"));
            }

            std::stringstream ss;
            ss << file.rdbuf();
            file.close();

            return fromSource(ss.str());
        }

        static jon fromSource(const std::string & source) {
            Lexer lexer;
            Parser parser;

            auto tokens = lexer.lex(source);
            auto ast = parser.parse(std::move(tokens));

            return jon {fromAst(std::move(ast))};
        }

        // Common methods //
    public:
        template<class T>
        T get() const noexcept {
            return value.get<T>();
        }

    public:
        // Object interface //
        const jon & operator[](const std::string & key) const {
            value.assertObjectFirstAccess(key);
            return value.get<obj_t>().at(key);
        }

        jon & operator[](const std::string & key) {
            value.assertObjectFirstAccess(key);
            return value.get<obj_t>()[key];
        }

        const jon & at(const std::string & key) const {
            value.assertObjectFirstAccess(key);
            return value.get<obj_t>().at(key);
        }

        jon & at(const std::string & key) {
            value.assertObjectFirstAccess(key);
            return value.get<obj_t>().at(key);
        }

        // Array interface //
        const jon & operator[](size_t idx) const {

        }

        // Serialization/Deserialization //
    private:
        static Value fromAst(ast::value_ptr && ast) {
            switch (ast->kind) {
                case ast::ValueKind::Null: {
                    return {};
                }
                case ast::ValueKind::Bool: {
                    return {ast::Value::as<ast::Bool>(std::move(ast))->val};
                }
                case ast::ValueKind::Int: {
                    return {ast::Value::as<ast::Int>(std::move(ast))->val};
                }
                case ast::ValueKind::Float: {
                    return {ast::Value::as<ast::Float>(std::move(ast))->val};
                }
                case ast::ValueKind::String: {
                    return {ast::Value::as<ast::String>(std::move(ast))->val};
                }
                case ast::ValueKind::Object: {
                    obj_t entries;
                    for (auto && keyVal : ast::Value::as<ast::Object>(std::move(ast))->entries) {
                        entries.emplace(keyVal.key.val, jon {fromAst(std::move(keyVal.val))});
                    }
                    return {entries};
                }
                case ast::ValueKind::Array: {
                    arr_t values;
                    for (auto && val : ast::Value::as<ast::Array>(std::move(ast))->values) {
                        values.emplace_back(jon {fromAst(std::move(val))});
                    }
                    return {values};
                }
            }
        }

    private:
        Value value;
    };

    namespace literals {
        jon operator""_jon(const char * str, std::size_t n) {
            return jon::fromSource(std::string(str, n));
        }
    }
}

#endif // JON_JON_H
