#ifndef JON_SERDES_H
#define JON_SERDES_H

#include "ast.h"

namespace jon {
    // Serializer/Deserializer
    class SerDes {
    public:
        SerDes() {}
        ~SerDes() = default;

        static val::Value fromAst(ast::value_ptr && ast) {
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
                    val::Value::obj_t entries;
                    for (auto && keyVal : ast::Value::as<ast::Object>(std::move(ast))->entries) {
                        entries.emplace(keyVal.key.val, fromAst(std::move(keyVal.val)));
                    }
                    return {entries};
                }
                case ast::ValueKind::Array: {
                    val::Value::arr_t values;
                    for (auto && val : ast::Value::as<ast::Array>(std::move(ast))->values) {
                        values.emplace_back(fromAst(std::move(val)));
                    }
                    return {values};
                }
            }
        }
    };
}

#endif // JON_SERDES_H
