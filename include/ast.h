#ifndef JON_AST_H
#define JON_AST_H

namespace jon {
    struct Node {};

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
}

#endif // JON_AST_H
