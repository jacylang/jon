#ifndef JON_VALUE_H
#define JON_VALUE_H

#include <variant>
#include <memory>
#include <string>
#include <assert.h>
#include <map>

#include "Lexer.h"

namespace jon::val {
    enum class Type {
        Null,
        Bool,
        Int,
        Float,
        String,
        Object,
        Array,
    };

    class Value {
    public:
        using null_t = std::monostate;
        using bool_t = bool;
        using int_t = int64_t;
        using float_t = double;
        using str_t = std::string;
        using obj_t = std::map<str_t, Value>;
        using arr_t = std::vector<Value>;

    public:
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

    private:
        Type t;
        std::variant<null_t, bool_t, int_t, float_t, str_t, obj_t, arr_t> v;
    };
}

#endif // JON_VALUE_H
