#ifndef JON_AST_H
#define JON_AST_H

#include <string>

namespace jon {
    struct Node {};

    struct Ident : Node {
        Ident(const std::string & val) : val(val) {}

        std::string val;
    };

    enum class ValueKind {
        Null,
        Bool,
        Int,
        Float,
        String,
        Object,
        Array,
    };

    struct Value {
        Value(ValueKind kind) : kind(kind) {}

        ValueKind kind;
    };

    struct Null : Value {
        Null() : Value(ValueKind::Null) {}
    };

    struct Bool : Value {
        Bool(bool val) : Value(ValueKind::Bool), val(val) {}

        bool val;
    };
}

#endif // JON_AST_H
