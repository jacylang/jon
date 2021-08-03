#ifndef JON_JON_H
#define JON_JON_H

#include <filesystem>
#include <fstream>
#include <variant>
#include <memory>
#include <string>
#include <assert.h>
#include <map>
#include <algorithm>
#include <regex>

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

        std::string typeStr() const {
            return typeStr(type());
        }

    private:
        struct Value {
            explicit Value() : t(Type::Null) {}
            explicit Value(null_t) : t(Type::Null) {}
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
                    throw type_error(errorMsg);
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
            constexpr T & get() {
                getTypeAssert<T>();
                return std::get<T>(v);
            }

            template<class T>
            constexpr const T & get() const {
                getTypeAssert<T>();
                return std::get<T>(v);
            }

            std::string typeStr() const {
                return jon::typeStr(t);
            }

            template<class T>
            static constexpr const char * valueAsKey(const T & t) {
                if constexpr (std::is_same_v<T, null_t>) {
                    return "null";
                } else if constexpr (std::is_same_v<T, bool_t>) {
                    return t ? "true" : "false";
                } else if constexpr (std::is_same_v<T, int_t> or std::is_same_v<T, float_t>) {
                    return std::to_string(t);
                } else if constexpr (std::is_same_v<T, str_t>) {
                    return t;
                } else if constexpr (std::is_same_v<T, obj_t>) {
                    throw type_error("Unable to use object as object key");
                } else if constexpr (std::is_same_v<T, arr_t>) {
                    throw type_error("Unable to use array as object key");
                }
            }

            Type t;
            std::variant<null_t, bool_t, int_t, float_t, str_t, obj_t, arr_t> v;

        private:
            template<class T>
            constexpr void getTypeAssert() const {
                if constexpr (std::is_same_v<T, null_t>) {
                    assertType(jon::Type::Null, "called `jon::get<null_t>` with not a `null_t` `jon`");
                }
                else if constexpr (std::is_same_v<T, bool_t>) {
                    assertType(jon::Type::Bool, "called `jon::get<bool_t>` with not a `bool_t` `jon`");
                }
                else if constexpr (std::is_same_v<T, int_t>) {
                    assertType(jon::Type::Int, "called `jon::get<int_t>` with not a `int_t` `jon`");
                }
                else if constexpr (std::is_same_v<T, float_t>) {
                    assertType(jon::Type::Float, "called `jon::get<float_t>` with not a `float_t` `jon`");
                }
                else if constexpr (std::is_same_v<T, str_t>) {
                    assertType(jon::Type::String, "called `jon::get<str_t>` with not a `str_t` `jon`");
                }
                else if constexpr (std::is_same_v<T, obj_t>) {
                    assertType(jon::Type::Object, "called `jon::get<obj_t>` with not a `obj_t` `jon`");
                }
                else if constexpr (std::is_same_v<T, arr_t>) {
                    assertType(jon::Type::Array, "called `jon::get<arr_t>` with not a `arr_t` `jon`");
                }
                else {
                    throw type_error("called `jon::get` with invalid type");
                }
            }
        };

    private:
        Value value;
        jon(Value && value) : value(std::move(value)) {}

        // Constructors //
    public:
        explicit jon() : value(null_t {}) {}
        explicit jon(null_t) noexcept : value(null_t {}) {}
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
                throw jon_exception(mstr("File '", path.string(), "' not found"));
            }

            std::stringstream ss;
            ss << file.rdbuf();
            file.close();

            return parse(ss.str());
        }

        static jon parse(const str_t & source) {
            Parser parser;

            return jon {
                fromAst(parser.parse(source))
            };
        }

        // Common methods //
    public:
        template<class T>
        constexpr T & get() noexcept {
            return value.get<T>();
        }

        template<class T>
        constexpr const T & get() const noexcept {
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

        // Operators //
    public:
        bool operator==(const jon & other) const {
            if (other.type() != type()) {
                return false;
            }

            if (size() != other.size()) {
                return false;
            }

            if (isNull()) {
                return true;
            }

            if (isBool()) {
                return get<bool_t>() == other.get<bool_t>();
            }

            if (isInt()) {
                return get<int_t>() == other.get<int_t>();
            }

            if (isFloat()) {
                return get<float_t>() == other.get<float_t>();
            }

            if (isString()) {
                return get<str_t>() == other.get<str_t>();
            }

            if (isObject()) {
                return std::equal(get<obj_t>().begin(), get<obj_t>().end(), other.get<obj_t>().begin());
            }

            if (isArray()) {
                return std::equal(get<arr_t>().begin(), get<arr_t>().end(), other.get<arr_t>().begin());
            }

            return false;
        }

        // Type checks //
    public:
        Type type() const noexcept {
            return value.type();
        }

        auto check(Type expectedType) const {
            if (type() != expectedType) {
                throw type_error(mstr("`jon::get` expected type ", typeStr(expectedType), " got ", value.typeStr()));
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
        jon & operator[](const str_t & key) {
            value.assertObjectFirstAccess(key);
            return get<obj_t>()[key];
        }

        template<class T>
        jon & operator[](const T & key) {
            return operator[](Value::valueAsKey(key));
        }

        const jon & at(const str_t & key) const {
            value.assertTypeObject(key);
            const auto & obj = get<obj_t>();
            const auto & it = obj.find(key);
            if (it == obj.end()) {
                throw out_of_range("`jon::at` by key '" + key + "'");
            }
            return it->second;
        }

        jon & at(const str_t & key) {
            value.assertObjectFirstAccess(key);
            auto obj = get<obj_t>();
            auto it = obj.find(key);
            if (it == obj.end()) {
                throw out_of_range("`jon::at` by key '" + key + "'");
            }
            return it->second;
        }

        template<class T>
        T at(const str_t & key) const {
            value.assertTypeObject(key);
            const auto & obj = get<obj_t>();
            const auto & it = obj.find(key);
            if (it == obj.end()) {
                throw out_of_range("`jon::at` by key '" + key + "'");
            }
            return it->second.get<T>();
        }

        template<class T>
        T & at(const str_t & key) {
            value.assertObjectFirstAccess(key);
            auto obj = get<obj_t>();
            auto it = obj.find(key);
            if (it == obj.end()) {
                throw out_of_range("`jon::at` by key '" + key + "'");
            }
            return it->second.get<T>();
        }

        // Array interface //
    public:
        jon & operator[](size_t idx) {
            if (isObject()) {
                return operator[](str_t {std::to_string(idx)});
            }
            value.assertArrayFirstAccess();
            return get<arr_t>()[idx];
        }

        const jon & at(size_t idx) const {
            value.assertTypeArray();
            if (idx > get<arr_t>().size()) {
                throw out_of_range(mstr("`jon::at` by index '", idx, "'"));
            }
            return get<arr_t>()[idx];
        }

        jon & at(size_t idx) {
            value.assertTypeArray();
            if (idx > get<arr_t>().size()) {
                throw out_of_range(mstr("`jon::at` by index '", idx, "'"));
            }
            return get<arr_t>()[idx];
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
        std::string dump(const std::string & indentStr) const {
            return dump(Indent{indentStr, 0});
        }

        std::string dump(uint16_t spaceSize) const {
            return dump(Indent{std::string(spaceSize, ' '), 0});
        }

        std::string dump(const Indent & indent = {"", -1}) const {
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
                        ss << it->second.dump(indent + 1);

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
                    const auto & arr = get<arr_t>();
                    for (size_t i = 0; i < arr.size(); i++) {
                        const auto & el = arr.at(i);
                        if (pretty) {
                            ss << indent + 1;
                        }
                        ss << el.dump(indent + 1);

                        if (i == arr.size() - 1) {
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
                    ss << "]";
                    return ss.str();
                }
            }
        }

        // Schema //
        jon validate(const jon & schema) const {
            // Check nullability, does not require any other constraints if value is null
            const auto nullable = schema.has("nullable") and schema.at<jon::bool_t>("nullable");
            if (nullable and isNull()) {
                return jon {};
            }

            std::vector<jon::str_t> expectedTypeNames;

            if (schema.isString()) {
                expectedTypeNames = {schema.get<jon::str_t>()};
            } else if (schema.has("type") and schema.at("type").isString()) {
                expectedTypeNames = {schema.at<jon::str_t>("type")};
            } else if (schema.has("type") and schema.at("type").isArray()) {
                for (const auto & typeName : schema.at<jon::arr_t>("type")) {
                    expectedTypeNames.emplace_back(typeName.get<str_t>());
                }
                if (expectedTypeNames.empty()) {
                    throw invalid_schema("`type` cannot be an empty array");
                }
            } else {
                throw invalid_schema("`type` must be specified");
            }

            const auto valueType = type();

            bool validType = false;
            for (const auto & typeName : expectedTypeNames) {
                const auto & foundType = typeNames.find(typeName);
                if (foundType == typeNames.end()) {
                    throw invalid_schema("unknown `type` '" + typeName + "'");
                }
                validType |= valueType == foundType->second;
            }

            if (not validType) {
                std::string expectedTypeStr;
                if (expectedTypeNames.size() == 1) {
                    expectedTypeStr = expectedTypeNames.at(0);
                } else {
                    for (size_t i = 0; i < expectedTypeNames.size(); i++) {
                        expectedTypeStr += expectedTypeNames.at(i);
                        if (expectedTypeNames.size() > 2 and i < expectedTypeNames.size() - 2) {
                            expectedTypeStr += ", ";
                        } else if (i < expectedTypeNames.size() - 1) {
                            expectedTypeStr += " or ";
                        }
                    }
                }
                return jon {
                    mstr("Type mismatch: Expected ", expectedTypeStr, ", got ", typeStr())
                };
            } else if (valueType == jon::Type::Int) {
                auto intValue = get<jon::int_t>();

                if (schema.has("minInt")) {
                    auto min = schema.at<jon::int_t>("minInt");
                    if (intValue < min) {
                        return jon {
                            mstr("Invalid integer size: ", intValue, " is less than ", min)
                        };
                    }
                }

                if (schema.has("maxInt")) {
                    auto max = schema.at<jon::int_t>("maxInt");
                    if (intValue > max) {
                        return jon {
                            mstr("Invalid integer size: ", intValue, " is greater than ", max)
                        };
                    }
                }
            } else if (valueType == jon::Type::Float) {
                auto floatValue = get<jon::float_t>();

                if (schema.has("minFloat")) {
                    auto min = schema.at<jon::int_t>("minFloat");
                    if (floatValue < min) {
                        return jon {
                            mstr("Invalid float size: ", floatValue, " is less than ", min)
                        };
                    }
                }

                if (schema.has("maxFloat")) {
                    auto max = schema.at<jon::int_t>("maxFloat");
                    if (floatValue > max) {
                        return jon {
                            mstr("Invalid float size: ", floatValue, " is greater than ", max)
                        };
                    }
                }
            } else if (valueType == jon::Type::String) {
                const auto & stringValue = get<jon::str_t>();

                if (schema.has("minLen")) {
                    auto min = schema.at<jon::int_t>("minLen");
                    if (stringValue.size() < min) {
                        return jon {
                            mstr("Invalid string size: ", stringValue.size(), " is less than ", min)
                        };
                    }
                }

                if (schema.has("maxLen")) {
                    auto max = schema.at<jon::int_t>("maxLen");
                    if (stringValue.size() > max) {
                        return jon {
                            mstr("Invalid string size: ", stringValue.size(), " is greater than ", max)
                        };
                    }
                }

                if (schema.has("pattern")) {
                    // TODO: Return parts failed to match
                    const auto pattern = schema.at<jon::str_t>("pattern");
                    const std::regex regex(pattern);
                    if (std::regex_match(stringValue, regex)) {
                        return jon {};
                    }
                    return jon {
                        mstr("Invalid string: Failed to match pattern '", pattern, "'")
                    };
                }
            } else if (valueType == jon::Type::Array) {
                const auto & arrayValue = get<jon::arr_t>();

                if (schema.has("minSize")) {
                    auto min = schema.at<jon::int_t>("minSize");
                    if (arrayValue.size() < min) {
                        return jon {
                            mstr("Invalid array size: ", arrayValue.size(), " is less than ", min)
                        };
                    }
                }

                if (schema.has("maxSize")) {
                    auto max = schema.at<jon::int_t>("maxSize");
                    if (arrayValue.size() > max) {
                        return jon {
                            mstr("Invalid array size: ", arrayValue.size(), " is greater than ", max)
                        };
                    }
                }

                if (not schema.has("items")) {
                    throw invalid_schema("array schema must specify `items`");
                }

                jon result {jon::obj_t {}};
                const auto & itemsSchema = schema.at("items");
                size_t index{0};
                for (const auto & el : arrayValue) {
                    const auto elValidation = el.validate(itemsSchema);
                    if (not elValidation.isNull()) {
                        result[index] = elValidation;
                    }
                    index++;
                }

                return result.empty() ? jon {} : result;
            } else if (valueType == jon::Type::Object) {
                const auto & objectValue = get<jon::obj_t>();

                if (schema.has("minProps")) {
                    auto min = schema.at<jon::int_t>("minProps");
                    if (objectValue.size() < min) {
                        return jon {
                            mstr("Invalid object properties count: ", objectValue.size(), " is less than ", min)
                        };
                    }
                }

                if (schema.has("maxProps")) {
                    auto max = schema.at<jon::int_t>("maxProps");
                    if (objectValue.size() > max) {
                        return jon {
                            mstr("Invalid object properties count: ", objectValue.size(), " is greater than ", max)
                        };
                    }
                }

                jon result {jon::obj_t {}};

                const auto & props = schema.at<jon::obj_t>("props");
                bool extras = schema.has("extras") and schema.at<jon::bool_t>("extras");

                std::vector<std::string> checkedProps;

                for (const auto & entry : objectValue) {
                    // TODO: additionalProperties
                    const auto & prop = props.find(entry.first);
                    if (not extras and prop == props.end()) {
                        result[entry.first] = jon {jon::str_t {"Additional property"}};
                    } else {
                        const auto & entryValidation = entry.second.validate(prop->second);
                        if (not entryValidation.isNull()) {
                            result[entry.first] = entryValidation;
                        }
                        checkedProps.emplace_back(entry.first);
                    }
                }

                if (checkedProps.size() != props.size()) {
                    for (const auto & prop : props) {
                        if (prop.second.has("optional")) {
                            continue;
                        }
                        if (std::find(checkedProps.begin(), checkedProps.end(), prop.first) != checkedProps.end()) {
                            continue;
                        }
                        result[prop.first] = jon {jon::str_t {"Missing property"}};
                    }
                }

                return result.empty() ? jon {} : result;
            }

            return jon {};
        }

    private:
        static const std::map<std::string, jon::Type> typeNames;
    };

    const std::map<std::string, jon::Type> jon::typeNames = {
        {"null", jon::Type::Null},
        {"bool", jon::Type::Bool},
        {"int", jon::Type::Int},
        {"float", jon::Type::Float},
        {"string", jon::Type::String},
        {"object", jon::Type::Object},
        {"array", jon::Type::Array},
    };

    namespace literal {
        jon operator""_jon(const char * str, std::size_t n) {
            return jon::parse(std::string(str, n));
        }
    }
}

#endif // JON_JON_H
