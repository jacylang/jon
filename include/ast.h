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

    };
}

#endif // JON_AST_H
