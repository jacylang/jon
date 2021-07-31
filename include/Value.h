#ifndef JON_VALUE_H
#define JON_VALUE_H

#include <variant>
#include <memory>

namespace jon {
    struct Value;
    using value_ptr = std::unique_ptr<Value>;

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
        Value(Type type) : type(type) {}

        Type type;
    };
}

#endif // JON_VALUE_H
