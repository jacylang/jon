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
    public:
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

        static std::string typeStr(Type type) {
            switch (type) {
                case Type::Null: return "null";
                case Type::Bool: return "boolean";
                case Type::Int: return "integer";
                case Type::Float: return "float";
                case Type::String: return "string";
                case Type::Object: return "object";
                case Type::Array: return "array";
            }
        }

        std::string typeStr() {
            switch (type()) {
                case Type::Null: return "null";
                case Type::Bool: return "boolean";
                case Type::Int: return "integer";
                case Type::Float: return "float";
                case Type::String: return "string";
                case Type::Object: return "object";
                case Type::Array: return "array";
            }
        }

    private:
        struct Value {
            explicit Value() = default;
            explicit Value(bool_t v) noexcept : v(v), t(Type::Bool) {}
            explicit Value(int_t v) noexcept : v(v), t(Type::Int) {}
            explicit Value(float_t v) noexcept : v(v), t(Type::Float) {}

            explicit Value(const str_t & v) noexcept : v(v), t(Type::String) {}
            explicit Value(str_t && v) noexcept : v(std::move(v)), t(Type::String) {}

            explicit Value(const obj_t & v) noexcept : v(v), t(Type::Object) {}
            explicit Value(obj_t && v) noexcept : v(std::move(v)), t(Type::Object) {}

            explicit Value(const arr_t & v) noexcept : v(v), t(Type::Array) {}
            explicit Value(arr_t && v) noexcept : v(std::move(v)), t(Type::Array) {}

            explicit Value(Type t) noexcept : t(t) {
                switch (t) {
                    case Type::Null: {
                        v = std::monostate {};
                        break;
                    }
                    case Type::Bool: {
                        v = false;
                        break;
                    }
                    case Type::Int: {
                        v = 0L;
                        break;
                    }
                    case Type::Float: {
                        v = 0.0;
                        break;
                    }
                    case Type::String: {
                        v = "";
                        break;
                    }
                    case Type::Object: {
                        v = obj_t {};
                        break;
                    }
                    case Type::Array: {
                        v = arr_t {};
                        break;
                    }
                }
            }

            Type type() const noexcept {
                return t;
            }

            void assertType(Type check, const std::string & errorMsg) const {
                if (this->type() != check) {
                    throw std::runtime_error(mstr("Type mismatch: ", errorMsg));
                }
            }

            void assertTypeArray() const {
                assertType(Type::Array, mstr("Cannot access ", typeStr(), " as array"));
            }

            void assertArrayFirstAccess() {
                if (t == Type::Null) {
                    v = arr_t {};
                } else {
                    assertTypeArray();
                }
            }

            void assertTypeObject(const std::string & key) const {
                assertType(Type::Object, mstr("Cannot access property ", key, " of ", typeStr()));
            }

            void assertObjectFirstAccess(const std::string & key) {
                if (t == Type::Null) {
                    v = obj_t {};
                } else {
                    assertTypeObject(key);
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
                return jon::typeStr(t);
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

            return parse(ss.str());
        }

        static jon parse(const str_t & source) {
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

        bool empty() const noexcept {
            if (isNull()) {
                return true;
            }

            if (isObject()) {
                return get<obj_t>().empty();
            }

            if (isArray()) {
                return get<arr_t>().empty();
            }

            return false;
        }

        bool has(const str_t & key) const noexcept {
            if (isObject()) {
                const auto & obj = get<obj_t>();
                return obj.find(key) != obj.end();
            }

            return false;
        }

        void clear() noexcept {
            *this = Value {type()};
        }

        size_t size() const noexcept {
            if (isNull()) {
                return 0;
            }

            if (isString()) {
                return get<str_t>().size();
            }

            if (isObject()) {
                return get<obj_t>().size();
            }

            if (isArray()) {
                return get<arr_t>().size();
            }

            return 1;
        }

        // Type checks //
    public:
        Type type() const noexcept {
            return value.type();
        }

        auto check(Type expectedType) const noexcept {
            if (type() != expectedType) {
                throw std::runtime_error(mstr("`jon::get` expected type ", typeStr(expectedType), " got ", value.typeStr()));
            }
            return *this;
        }

        bool isNull() const noexcept {
            return type() == Type::Null;
        }

        bool isBool() const noexcept {
            return type() == Type::Bool;
        }

        bool isInt() const noexcept {
            return type() == Type::Int;
        }

        bool isFloat() const noexcept {
            return type() == Type::Float;
        }

        bool isString() const noexcept {
            return type() == Type::String;
        }

        bool isObject() const noexcept {
            return type() == Type::Object;
        }

        bool isArray() const noexcept {
            return type() == Type::Array;
        }

        // Object interface //
    public:
        const jon & operator[](const str_t & key) const {
            value.assertTypeObject(key);
            return value.get<obj_t>().at(key);
        }

        jon & operator[](const str_t & key) {
            value.assertObjectFirstAccess(key);
            return value.get<obj_t>()[key];
        }

        const jon & at(const str_t & key) const {
            value.assertTypeObject(key);
            return value.get<obj_t>().at(key);
        }

        jon & at(const str_t & key) {
            value.assertObjectFirstAccess(key);
            return value.get<obj_t>().at(key);
        }

        template<class T>
        const T & at(const str_t & key) const {
            value.assertTypeObject(key);
            return value.get<obj_t>().at(key).get<T>();
        }

        template<class T>
        T & at(const str_t & key) {
            value.assertObjectFirstAccess(key);
            return value.get<obj_t>().at(key).get<T>();
        }

        // Array interface //
    public:
        const jon & operator[](size_t idx) const {
            value.assertTypeArray();
            return value.get<arr_t>().at(idx);
        }

        jon & operator[](size_t idx) {
            value.assertArrayFirstAccess();
            return value.get<arr_t>()[idx];
        }

        const jon & at(size_t idx) const {
            value.assertTypeArray();
            return value.get<arr_t>().at(idx);
        }

        jon & at(size_t idx) {
            value.assertTypeArray();
            return value.get<arr_t>().at(idx);
        }

        // Serialization/Deserialization //
    private:
        static Value fromAst(ast::value_ptr && ast) {
            switch (ast->kind) {
                case ast::ValueKind::Null: {
                    return Value {};
                }
                case ast::ValueKind::Bool: {
                    return Value {ast::Value::as<ast::Bool>(std::move(ast))->val};
                }
                case ast::ValueKind::Int: {
                    return Value {ast::Value::as<ast::Int>(std::move(ast))->val};
                }
                case ast::ValueKind::Float: {
                    return Value {ast::Value::as<ast::Float>(std::move(ast))->val};
                }
                case ast::ValueKind::String: {
                    return Value {ast::Value::as<ast::String>(std::move(ast))->val};
                }
                case ast::ValueKind::Object: {
                    obj_t entries;
                    for (auto && keyVal : ast::Value::as<ast::Object>(std::move(ast))->entries) {
                        entries.emplace(keyVal.key.val, jon {fromAst(std::move(keyVal.val))});
                    }
                    return Value {entries};
                }
                case ast::ValueKind::Array: {
                    arr_t values;
                    for (auto && val : ast::Value::as<ast::Array>(std::move(ast))->values) {
                        values.emplace_back(jon {fromAst(std::move(val))});
                    }
                    return Value {values};
                }
            }
        }

    public:
        std::string stringify(const std::string & indentStr) const {
            return stringify(Indent {indentStr, 0});
        }

        std::string stringify(const Indent & indent = {"", -1}) const {
            bool pretty = indent.size != -1;

            // TODO: Support multi-line strings

            switch (type()) {
                case Type::Null: return "null";
                case Type::Bool: return get<bool_t>() ? "true" : "false";
                case Type::Int: return std::to_string(get<int_t>());
                case Type::Float: return std::to_string(get<float_t>());
                case Type::String: {
                    if (not pretty) {
                        return "'" + escstr(get<str_t>()) + "'";
                    }
                    return "'" + get<str_t>() + "'";
                }
                case Type::Object: {
                    std::stringstream ss;
                    ss << "{";
                    if (pretty) {
                        ss << "\n";
                    }
                    const auto & obj = get<obj_t>();
                    for (auto it = obj.begin(); it != obj.end(); it++) {
                        if (pretty) {
                            ss << indent + 1;
                        }
                        ss << it->first << ":";
                        if (pretty) {
                            ss << " ";
                        }
                        ss << it->second.stringify(indent + 1);

                        if (it == std::prev(obj.end())) {
                            continue;
                        }

                        if (pretty) {
                            ss << "\n";
                        } else {
                            ss << ",";
                        }
                    }
                    if (pretty) {
                        ss << "\n" << indent;
                    }
                    ss << "}";
                    return ss.str();
                }
                case Type::Array: {
                    std::stringstream ss;
                    ss << "[";
                    if (pretty) {
                        ss << "\n";
                    }
                    for (const auto & el : get<arr_t>()) {
                        if (pretty) {
                            ss << indent + 1;
                        }
                        ss << el.stringify(indent + 1);
                        if (pretty) {
                            ss << "\n";
                        } else {
                            ss << ",";
                        }
                    }
                    if (pretty) {
                        ss << "\n" << indent;
                    }
                    ss << "]";
                    return ss.str();
                }
            }
        }

    private:
        Value value;
    };

    namespace literal {
        jon operator""_jon(const char * str, std::size_t n) {
            return jon::parse(std::string(str, n));
        }
    }
}

#endif // JON_JON_H
