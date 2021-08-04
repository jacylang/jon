#ifndef JON_AST_H
#define JON_AST_H

#include <string>
#include <vector>
#include <memory>

namespace jon::ast {
    struct Null;
    struct Bool;
    struct Int;
    struct Float;
    struct String;
    struct Object;
    struct Array;

    struct ValueVisitor {
    public:
        virtual void visit(const Null&) = 0;
        virtual void visit(const Bool&) = 0;
        virtual void visit(const Int&) = 0;
        virtual void visit(const Float&) = 0;
        virtual void visit(const String&) = 0;
        virtual void visit(const Object&) = 0;
        virtual void visit(const Array&) = 0;
    };
}

namespace jon::ast {
    struct Value;
    using value_ptr = std::unique_ptr<Value>;
    using value_list = std::vector<value_ptr>;

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
        virtual ~Value() = default;

        ValueKind kind;

        template<class T>
        static constexpr T * as(value_ptr && val) {
            return static_cast<T*>(val.release());
        }

        virtual void accept(ValueVisitor & visitor) const = 0;
    };

    struct Null : Value {
        Null() : Value(ValueKind::Null) {}

        void accept(ValueVisitor & visitor) const override {
            visitor.visit(*this);
        }
    };

    struct Bool : Value {
        Bool(bool val) : Value(ValueKind::Bool), val(val) {}

        bool val;

        void accept(ValueVisitor & visitor) const override {
            visitor.visit(*this);
        }
    };

    struct Int : Value {
        Int(int64_t val) : Value(ValueKind::Int), val(val) {}

        int64_t val;

        void accept(ValueVisitor & visitor) const override {
            visitor.visit(*this);
        }
    };

    struct Float : Value {
        Float(double val) : Value(ValueKind::Float), val(val) {}

        double val;

        void accept(ValueVisitor & visitor) const override {
            visitor.visit(*this);
        }
    };

    struct String : Value {
        String(const std::string & val) : Value(ValueKind::String), val(val) {}

        std::string val;

        void accept(ValueVisitor & visitor) const override {
            visitor.visit(*this);
        }
    };

    struct KeyValue {
        KeyValue(Ident && key, value_ptr && val) : key(std::move(key)), val(std::move(val)) {}

        Ident key;
        value_ptr val;
    };

    struct Object : Value {
        using Entries = std::vector<KeyValue>;

        Object(Entries && entries) : Value(ValueKind::Object), entries(std::move(entries)) {}

        Entries entries;

        void accept(ValueVisitor & visitor) const override {
            visitor.visit(*this);
        }
    };

    struct Array : Value {
        Array(value_list && values) : Value(ValueKind::Array), values(std::move(values)) {}

        value_list values;

        void accept(ValueVisitor & visitor) const override {
            visitor.visit(*this);
        }
    };
}

#endif // JON_AST_H
