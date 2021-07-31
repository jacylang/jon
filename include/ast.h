#ifndef JON_AST_H
#define JON_AST_H

#include <string>
#include <vector>
#include <memory>

namespace jon {
    struct Null;
    struct Bool;
    struct Int;
    struct Float;
    struct String;
    struct NValue;
    struct Array;

    struct ValueVisitor {
    public:
        virtual void visit(const Null&) = 0;
        virtual void visit(const Bool&) = 0;
        virtual void visit(const Int&) = 0;
        virtual void visit(const Float&) = 0;
        virtual void visit(const String&) = 0;
        virtual void visit(const NValue&) = 0;
        virtual void visit(const Array&) = 0;
    };
}

namespace jon {
    struct NValue;
    using n_value_ptr = std::unique_ptr<NValue>;
    using n_value_list = std::vector<n_value_ptr>;

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

    struct NValue {
        NValue(ValueKind kind) : kind(kind) {}

        ValueKind kind;

        virtual void accept(ValueVisitor & visitor) const = 0;
    };

    struct Null : NValue {
        Null() : NValue(ValueKind::Null) {}
        
        void accept(ValueVisitor & visitor) const override {
            visitor.visit(*this);
        }
    };

    struct Bool : NValue {
        Bool(bool val) : NValue(ValueKind::Bool), val(val) {}

        bool val;

        void accept(ValueVisitor & visitor) const override {
            visitor.visit(*this);
        }
    };

    struct Int : NValue {
        Int(const std::string & val) : NValue(ValueKind::Int), val(val) {}

        std::string val;

        void accept(ValueVisitor & visitor) const override {
            visitor.visit(*this);
        }
    };

    struct Float : NValue {
        Float(const std::string & val) : NValue(ValueKind::Float), val(val) {}

        std::string val;

        void accept(ValueVisitor & visitor) const override {
            visitor.visit(*this);
        }
    };

    struct String : NValue {
        String(const std::string & val) : NValue(ValueKind::String), val(val) {}

        std::string val;

        void accept(ValueVisitor & visitor) const override {
            visitor.visit(*this);
        }
    };

    struct KeyValue {
        KeyValue(Ident && key, n_value_ptr && val) : key(std::move(key)), val(std::move(val)) {}

        Ident key;
        n_value_ptr val;
    };

    struct NValue : NValue {
        using Entries = std::vector<KeyValue>;

        NValue(Entries && entries) : NValue(ValueKind::Object), entries(std::move(entries)) {}

        Entries entries;

        void accept(ValueVisitor & visitor) const override {
            visitor.visit(*this);
        }
    };

    struct Array : NValue {
        Array(n_value_list && values) : NValue(ValueKind::Array), values(std::move(values)) {}

        n_value_list values;

        void accept(ValueVisitor & visitor) const override {
            visitor.visit(*this);
        }
    };
}

#endif // JON_AST_H
