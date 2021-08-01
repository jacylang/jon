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
        using null_t = std::monostate;
        using bool_t = bool;
        using int_t = int64_t;
        using float_t = double;
        using str_t = std::string;
        using obj_t = std::map<str_t, Value>;
        using arr_t = std::vector<Value>;

    public:
        Value() = default;
        Value(bool_t v) noexcept : storage(v) {}
        Value(int_t v) noexcept : storage(v) {}
        Value(float_t v) noexcept : storage(v) {}

        Value(const str_t & v) noexcept : storage(v) {}
        Value(str_t && v) noexcept : storage(std::move(v)) {}

        Value(const obj_t & v) noexcept : storage(v) {}
        Value(obj_t && v) noexcept : storage(std::move(v)) {}

        Value(const arr_t & v) noexcept : storage(v) {}
        Value(arr_t && v) noexcept : storage(std::move(v)) {}

    private:
        std::variant<null_t, bool_t, int_t, float_t, str_t, obj_t, arr_t> storage;
    };
}

#endif // JON_VALUE_H
