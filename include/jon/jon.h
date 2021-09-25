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
#include <cmath>
#include <utility>

#include "Lexer.h"
#include "Parser.h"
#include "Printer.h"
#include "ref.h"

namespace jacylang {
    namespace detail {
        enum class Type : uint8_t {
            Null,
            Bool,
            Int,
            Float,
            String,
            Object,
            Array,
        };

        using null_t = std::monostate;
        using bool_t = bool;
        using int_t = int64_t;
        using float_t = double;
        using str_t = std::string;

        template<class JonT>
        using obj_t = std::map<str_t, JonT>;

        template<class JonT>
        using arr_t = std::vector<JonT>;

        // Type Traits //
        template<class T>
        class HasToJon {
        private:
            using true_t = char[1];
            using false_t = char[2];

            template<class U>
            static true_t & check(decltype(&U::toJon));

            template<class U>
            static false_t & check(...);

        public:
            enum {
                value = sizeof(check<T>(0)) == sizeof(true_t)
            };
        };

        template<class T>
        class HasFromJon {
        private:
            using true_t = char[1];
            using false_t = char[2];

            template<class U>
            static true_t & check(decltype(&U::fromJon));

            template<class U>
            static false_t & check(...);

        public:
            enum {
                value = sizeof(check<T>(0)) == sizeof(true_t)
            };
        };
    }

    class jon {
    public:
        using null_t = detail::null_t;
        using bool_t = detail::bool_t;
        using int_t = detail::int_t;
        using float_t = detail::float_t;
        using str_t = detail::str_t;
        using obj_t = detail::obj_t<jon>;
        using arr_t = detail::arr_t<jon>;

        using storage_t = std::variant<null_t, bool_t, int_t, float_t, str_t, obj_t, arr_t>;

        using Type = detail::Type;

        static std::string typeStr(Type type) {
            switch (type) {
                case Type::Null: return "null";
                case Type::Bool: return "bool";
                case Type::Int: return "int";

                case Type::Float: return "float";
                case Type::String: return "string";
                case Type::Object: return "object";
                case Type::Array: return "array";

                default: {
                    throw std::logic_error("[jon bug]: Unhandled `Type` in `jon::typeStr`");
                }
            }
        }

        std::string typeStr() const {
            return typeStr(type());
        }

        // Value //
    private:
        storage_t value;

        // Private Value Interface //
    private:
        void assertType(Type check, const std::string & errorMsg) const {
            if (this->type() != check) {
                throw type_error(errorMsg);
            }
        }

        void assertTypeArray() const {
            assertType(Type::Array, mstr("Cannot access ", typeStr(), " as array"));
        }

        void assertArrayFirstAccess() {
            if (type() == Type::Null) {
                *this = arr_t {};
            } else {
                assertTypeArray();
            }
        }

        void assertTypeObject(const std::string & key) const {
            assertType(Type::Object, mstr("Cannot access property ", key, " of ", typeStr()));
        }

        void assertObjectFirstAccess(const std::string & key) {
            if (type() == Type::Null) {
                *this = obj_t {};
            } else {
                assertTypeObject(key);
            }
        }

        template<class T>
        constexpr void getTypeAssert() const {
            if constexpr (std::is_same<T, null_t>::value) {
                assertType(Type::Null, "called `get<null_t>` with not a `null_t` `jon`");
            }
            else if constexpr (std::is_same<T, bool_t>::value) {
                assertType(Type::Bool, "called `get<bool_t>` with not a `bool_t` `jon`");
            }
            else if constexpr (std::is_same<T, int_t>::value) {
                assertType(Type::Int, "called `get<int_t>` with not a `int_t` `jon`");
            }
            else if constexpr (std::is_same<T, float_t>::value) {
                assertType(Type::Float, "called `get<float_t>` with not a `float_t` `jon`");
            }
            else if constexpr (std::is_same<T, str_t>::value) {
                assertType(Type::String, "called `get<str_t>` with not a `str_t` `jon`");
            }
            else if constexpr (std::is_same<T, obj_t>::value) {
                assertType(Type::Object, "called `get<obj_t>` with not a `obj_t` `jon`");
            }
            else if constexpr (std::is_same<T, arr_t>::value) {
                assertType(Type::Array, "called `get<arr_t>` with not a `arr_t` `jon`");
            }
            else {
                throw type_error("called `get` with invalid type");
            }
        }

        template<class T>
        static constexpr const char * valueAsKey(const T & t) {
            if constexpr (std::is_same<T, null_t>::value) {
                return "null";
            }

            if constexpr (std::is_same<T, bool_t>::value) {
                return t ? "true" : "false";
            }

            if constexpr (std::is_same<T, int_t>::value or std::is_same<T, float_t>::value) {
                return std::to_string(t);
            }

            if constexpr (std::is_same<T, str_t>::value) {
                return t;
            }

            if constexpr (std::is_same<T, obj_t>::value) {
                throw type_error("Unable to use object as object key");
            }

            if constexpr (std::is_same<T, arr_t>::value) {
                throw type_error("Unable to use array as object key");
            }
        }

        // Constructors //
    public:
        explicit jon(std::nullptr_t = nullptr) : value{null_t {}} {}

        jon(Type t) {
            switch (t) {
                case Type::Null: {
                    value.emplace<null_t>();
                    break;
                }
                case Type::Bool: {
                    value.emplace<bool_t>(false);
                    break;
                }
                case Type::Int: {
                    value.emplace<int_t>(0);
                    break;
                }
                case Type::Float: {
                    value.emplace<float_t>(0.0);
                    break;
                }
                case Type::String: {
                    value.emplace<str_t>("");
                    break;
                }
                case Type::Object: {
                    value.emplace<obj_t>(obj_t {});
                    break;
                }
                case Type::Array: {
                    value.emplace<arr_t>(arr_t {});
                    break;
                }
                default: {
                    throw std::logic_error("[jon bug]: Unhandled `Type` in `jon::jon(Type)`");
                }
            }
        }

        template<class T>
        using no_cvr = std::remove_reference<typename std::remove_cv<T>::type>;

        template<class T, class U = typename no_cvr<T>::type>
        jon(const T & val) {
            if constexpr (std::is_same<U, null_t>::value) {
                value.emplace<null_t>();
                return;
            }

            if constexpr (std::is_same<U, bool_t>::value) {
                value.emplace<bool_t>(static_cast<bool_t>(val));
                return;
            }

            if constexpr (std::is_integral<U>::value) {
                value.emplace<int_t>(static_cast<int_t>(val));
                return;
            }

            if constexpr (std::is_floating_point<U>::value) {
                value.emplace<float_t>(static_cast<float_t>(val));
                return;
            }

            if constexpr (std::is_convertible<U, str_t>::value) {
                value.emplace<str_t>(static_cast<str_t>(val));
                return;
            }

            if constexpr (std::is_same<U, obj_t>::value) {
                value.emplace<obj_t>(val);
                return;
            }

            if constexpr (std::is_same<U, arr_t>::value) {
                value.emplace<arr_t>(val);
                return;
            }

            if constexpr (detail::HasToJon<U>::value) {
                *this = U::toJon(val);
                return;
            }

            throw std::logic_error("Invalid type for jon copy constructor");
        }

        template<class T, class U = typename no_cvr<T>::type>
        jon(T && val) {
            if constexpr (std::is_same<U, null_t>::value) {
                value.emplace<null_t>();
                return;
            }

            if constexpr (std::is_same<U, bool_t>::value) {
                value.emplace<bool_t>(static_cast<bool_t>(std::move(val)));
                return;
            }

            if constexpr (std::is_integral<U>::value) {
                value.emplace<int_t>(static_cast<int_t>(std::move(val)));
                return;
            }

            if constexpr (std::is_floating_point<U>::value) {
                value.emplace<float_t>(static_cast<float_t>(std::move(val)));
                return;
            }

            if constexpr (std::is_convertible<U, str_t>::value) {
                value.emplace<str_t>(std::move(val));
                return;
            }

            if constexpr (std::is_same<U, obj_t>::value) {
                value.emplace<obj_t>(std::move(val));
                return;
            } 
            
            if constexpr (std::is_same<U, arr_t>::value) {
                value.emplace<arr_t>(std::move(val));
                return;
            }

            if constexpr (detail::HasToJon<U>::value) {
                *this = U::toJon(std::move(val));
                return;
            }

            throw std::logic_error("Invalid type for jon move constructor");
        }

        jon(const detail::jon_ref<jon> & ref) : jon{ref.get()} {}
        jon(const jon & other) : value(other.value) {}
        jon(jon && other) : value(std::move(other.value)) {}

        jon(std::initializer_list<detail::jon_ref<jon>> init, bool typeDeduction = true, Type type = Type::Array) {
            if (init.size() == 0) {
                *this = obj_t {};
                return;
            }

            bool isObjectProjection = std::all_of(init.begin(), init.end(), [](const detail::jon_ref<jon> & el) {
                return el->isArray() and el->size() == 2 and el->at(0).isString();
            });

            if (not typeDeduction) {
                if (type == Type::Array) {
                    isObjectProjection = false;
                }

                if (not isObjectProjection and type == Type::Object) {
                    throw type_error("Cannot instantiate `jon` object from non-object-like initializer_list");
                }
            }

            if (isObjectProjection) {
                *this = obj_t {};
                for (auto & el : init) {
                    auto pair = el.get().get<arr_t>();
                    get<obj_t>().emplace(pair.at(0).get<str_t>(), std::move(pair.at(1)));
                }
            } else {
                *this = arr_t(init.begin(), init.end());
            }
        }

        // Assignment //
    public:
        jon & operator=(jon other) noexcept (
            std::is_nothrow_move_constructible_v<storage_t> &&
            std::is_nothrow_move_assignable_v<storage_t>
        ) {
            std::swap(value, other.value);
            return *this;
        }

        jon & operator=(std::initializer_list<detail::jon_ref<jon>> init) {
            if (init.size() == 0) {
                *this = obj_t {};
                return *this;
            }

            bool isObjectProjection = std::all_of(init.begin(), init.end(), [](const detail::jon_ref<jon> & el) {
                return el->isArray() and el->size() == 2 and el->at(0).isString();
            });

            if (isObjectProjection) {
                *this = obj_t {};
                for (auto & el : init) {
                    get<obj_t>().emplace(el.get().get<arr_t>().at(0).get<str_t>(), std::move(el.get().get<arr_t>().at(1)));
                }
            } else {
                *this = arr_t(init.begin(), init.end());
            }
            return *this;
        }

        // Custom constructors //
    public:
        static jon fromFile(const std::filesystem::path & path, bool debug = false) {
            std::fstream file(path);

            if (not file.is_open()) {
                throw jon_exception(mstr("File '", path.string(), "' not found"));
            }

            std::stringstream ss;
            ss << file.rdbuf();
            file.close();

            return parse(ss.str(), debug);
        }

        static jon parse(const str_t & source, bool debug = false) {
            Parser parser;
            return fromAst(parser.parse(source, debug));
        }

        // Serialization/Deserialization //
    private:
        static jon fromAst(ast::value_ptr && ast) {
            switch (ast->kind) {
                case ast::ValueKind::Null: {
                    return jon {};
                }
                case ast::ValueKind::Bool: {
                    return jon(ast::Value::as<ast::Bool>(std::move(ast))->val);
                }
                case ast::ValueKind::Int: {
                    return jon(ast::Value::as<ast::Int>(std::move(ast))->val);
                }
                case ast::ValueKind::Float: {
                    return jon(ast::Value::as<ast::Float>(std::move(ast))->val);
                }
                case ast::ValueKind::String: {
                    return jon(ast::Value::as<ast::String>(std::move(ast))->val);
                }
                case ast::ValueKind::Object: {
                    obj_t entries;
                    for (auto && keyVal : ast::Value::as<ast::Object>(std::move(ast))->entries) {
                        entries.emplace(keyVal.key.val, fromAst(std::move(keyVal.val)));
                    }
                    return jon(std::move(entries));
                }
                case ast::ValueKind::Array: {
                    arr_t values;
                    for (auto && val : ast::Value::as<ast::Array>(std::move(ast))->values) {
                        values.emplace_back(fromAst(std::move(val)));
                    }
                    return jon(std::move(values));
                }
                default: {
                    throw std::logic_error("[jon bug]: Unhandled `ast::ValueKind` in `jon::fromAst");
                }
            }
        }

        // Common methods //
    public:
        template<class T, class U = typename no_cvr<T>::type>
        T as() const noexcept(detail::HasFromJon<T>::value) {
            return U::fromJon(*this);
        }

        template<class T>
        T & get() {
            getTypeAssert<T>();
            return std::get<T>(value);
        }

        template<class T>
        const T & get() const {
            getTypeAssert<T>();
            return std::get<T>(value);
        }

        bool_t & getBool() {
            return get<bool_t>();
        }

        const bool_t & getBool() const {
            return get<bool_t>();
        }

        int_t & getInt() {
            return get<int_t>();
        }

        const int_t & getInt() const {
            return get<int_t>();
        }

        float_t & getFloat() {
            return get<float_t>();
        }

        const float_t & getFloat() const {
            return get<float_t>();
        }

        str_t & getStr() {
            return get<str_t>();
        }

        const str_t & getStr() const {
            return get<str_t>();
        }

        obj_t & getObj() {
            return get<obj_t>();
        }

        const obj_t & getObj() const {
            return get<obj_t>();
        }

        arr_t & getArr() {
            return get<arr_t>();
        }

        const arr_t & getArr() const {
            return get<arr_t>();
        }

        bool empty() const {
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

        bool has(const str_t & key) const {
            if (isObject()) {
                const auto & obj = get<obj_t>();
                return obj.find(key) != obj.end();
            }

            return false;
        }

        void clear() {
            *this = jon(type());
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
        template<class T, typename = typename std::enable_if_t<std::is_scalar_v<T>, T>>
        friend bool operator==(const jon & lhs, const T & rhs) {
            return lhs == jon(rhs);
        }

        template<class T, typename = typename std::enable_if_t<std::is_scalar_v<T>, T>>
        friend bool operator==(const T & lhs, const jon & rhs) {
            return jon(lhs) == rhs;
        }

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
                return std::abs(get<float_t>() - other.get<float_t>()) < std::numeric_limits<double>::epsilon();
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
        Type type() const {
            if (std::holds_alternative<null_t>(value)) {
                return Type::Null;
            }

            if (std::holds_alternative<bool_t>(value)) {
                return Type::Bool;
            }

            if (std::holds_alternative<int_t>(value)) {
                return Type::Int;
            }

            if (std::holds_alternative<float_t>(value)) {
                return Type::Float;
            }

            if (std::holds_alternative<str_t>(value)) {
                return Type::String;
            }

            if (std::holds_alternative<obj_t>(value)) {
                return Type::Object;
            }

            if (std::holds_alternative<arr_t>(value)) {
                return Type::Array;
            }

            throw std::logic_error("Unknown type of `jon` value");
        }

        auto check(Type expectedType) const {
            if (type() != expectedType) {
                throw type_error(mstr("`get` expected type ", typeStr(expectedType), " got ", typeStr()));
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

        // Float interface //
    public:
        static float_t getNaN() {
            static const float_t NaN = std::numeric_limits<jon::float_t>::quiet_NaN();
            return NaN;
        }

        bool isNaN() const {
            return type() == Type::Float and std::isnan(get<float_t>());
        }

        bool isInf() const {
            static_assert(std::numeric_limits<float>::is_iec559, "IEEE 754 required to use `jon::isInf`");
            return type() == Type::Float and std::isinf(get<float_t>());
        }

        bool isPosInf() const {
            static_assert(std::numeric_limits<float>::is_iec559, "IEEE 754 required to use `jon::isPosInf`");
            return isInf() and get<float_t>() > std::numeric_limits<float_t>::max();
        }

        bool isNegInf() const {
            static_assert(std::numeric_limits<float>::is_iec559, "IEEE 754 required to use `jon::isNegInf`");
            return isInf() and get<float_t>() < std::numeric_limits<float_t>::lowest();
        }

        // Object interface //
    public:
        static jon obj(std::initializer_list<detail::jon_ref<jon>> init = {}) {
            return jon(init, false, Type::Object);
        }

        jon & operator[](const str_t & key) {
            assertObjectFirstAccess(key);
            return get<obj_t>()[key];
        }

        template<class T>
        jon & operator[](const T & key) {
            return operator[](valueAsKey(key));
        }

        const jon & at(const str_t & key) const {
            assertTypeObject(key);
            auto it = get<obj_t>().find(key);
            if (it == get<obj_t>().end()) {
                throw out_of_range("`at` by key '" + key + "'");
            }
            return (*it).second;
        }

        jon & at(const str_t & key) {
            assertObjectFirstAccess(key);
            auto it = get<obj_t>().find(key);
            if (it == get<obj_t>().end()) {
                throw out_of_range("`at` by key '" + key + "'");
            }
            return (*it).second;
        }

        template<class T>
        const T & at(const str_t & key) const {
            assertTypeObject(key);
            auto it = get<obj_t>().find(key);
            if (it == get<obj_t>().end()) {
                throw out_of_range("`at` by key '" + key + "'");
            }
            return (*it).second.get<T>();
        }

        template<class T>
        T & at(const str_t & key) {
            assertObjectFirstAccess(key);
            auto it = get<obj_t>().find(key);
            if (it == get<obj_t>().end()) {
                throw out_of_range("`at` by key '" + key + "'");
            }
            return (*it).second.get<T>();
        }

        template<class T, class U = typename no_cvr<T>::type>
        T atAs(const str_t & key) const noexcept(detail::HasFromJon<T>::value) {
            return U::fromJon(at(key));
        }

        null_t & nullAt(const str_t & key) {
            return at<null_t>(key);
        }

        const null_t & nullAt(const str_t & key) const {
            return at<null_t>(key);
        }

        bool_t & boolAt(const str_t & key) {
            return at<bool_t>(key);
        }

        const bool_t & boolAt(const str_t & key) const {
            return at<bool_t>(key);
        }

        int_t & intAt(const str_t & key) {
            return at<int_t>(key);
        }

        const int_t & intAt(const str_t & key) const {
            return at<int_t>(key);
        }

        float_t & floatAt(const str_t & key) {
            return at<float_t>(key);
        }

        const float_t & floatAt(const str_t & key) const {
            return at<float_t>(key);
        }

        str_t & strAt(const str_t & key) {
            return at<str_t>(key);
        }

        const str_t & strAt(const str_t & key) const {
            return at<str_t>(key);
        }

        obj_t & objAt(const str_t & key) {
            return at<obj_t>(key);
        }

        const obj_t & objAt(const str_t & key) const {
            return at<obj_t>(key);
        }

        arr_t & arrAt(const str_t & key) {
            return at<arr_t>(key);
        }

        const arr_t & arrAt(const str_t & key) const {
            return at<arr_t>(key);
        }

        jon flatten() const {
            jon result {arr_t {}};
            _flatten("", *this, result);
            return result;
        }

        // Array interface //
    public:
        jon & operator[](size_t idx) {
            if (isObject()) {
                return operator[](str_t {std::to_string(idx)});
            }
            assertArrayFirstAccess();
            return get<arr_t>()[idx];
        }

        const jon & at(size_t idx) const {
            assertTypeArray();
            if (idx >= get<arr_t>().size()) {
                throw out_of_range(mstr("`at` by index '", idx, "'"));
            }
            return get<arr_t>().at(idx);
        }

        jon & at(size_t idx) {
            assertTypeArray();
            if (idx >= get<arr_t>().size()) {
                throw out_of_range(mstr("`at` by index '", idx, "'"));
            }
            return get<arr_t>().at(idx);
        }

        void push(const jon & el) {
            assertArrayFirstAccess();
            get<arr_t>().push_back(el);
        }

        // Utility interface //
    private:
        static void _flatten(const std::string & refString, const jon & value, jon & result) {
            switch (value.type()) {
                case Type::Object: {
                    if (value.empty()) {
                        return;
                    }
                    for (const auto & entry : value.get<obj_t>()) {
                        _flatten(refString + "/" + escstr(entry.first), entry.second, result);
                    }
                    break;
                }
                case Type::Array: {
                    if (value.empty()) {
                        return;
                    }
                    size_t index{0};
                    for (const auto & el : value.get<arr_t>()) {
                        _flatten(refString + "/" + std::to_string(index), el, result);
                    }
                    break;
                }
                default: {
                    result[refString] = value;
                }
            }
        }

        // Serialization //
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
                case Type::Float: {
                    if (isNaN()) {
                        return "nan";
                    }
                    return std::to_string(get<float_t>());
                }
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
                default: {
                    throw std::logic_error("[jon bug]: Unhandled `Type` in `jon::dump`");
                }
            }
        }

        // Schemas //
    public:
        jon validate(const jon & schema) const {
            jon result;
            _validate(schema, "", result);
            return result;
        }

    private:
        void _validate(const jon & schema, const str_t & path, jon & result) const {
            // Check nullability, does not require any other constraints if value is null
            const auto nullable = schema.has("nullable") and schema.schemaAt<bool_t>("nullable", path);
            if (nullable and isNull()) {
                return;
            }

            std::vector<str_t> expectedTypeNames;

            bool anyType = false;
            if (schema.isString()) {
                expectedTypeNames = {schema.get<str_t>()};
            } else if (schema.has("type")) {
                if (schema.at("type").isString()) {
                    expectedTypeNames = {schema.at<str_t>("type")};
                } else if (schema.at("type").isArray()) {
                    for (const auto & typeName : schema.at<arr_t>("type")) {
                        expectedTypeNames.emplace_back(typeName.get<str_t>());
                    }
                    if (expectedTypeNames.empty()) {
                        throw invalid_schema("`type` cannot be an empty array", path + "/type");
                    }
                } else if (schema.at("type").isNull()) {
                    anyType = true;
                } else {
                    throw invalid_schema("`type` must be either string, array or null", path + "/type");
                }
            } else {
                anyType = true;
            }

            const auto valueType = type();

            if (not anyType) {
                bool validType = false;
                for (const auto & typeName : expectedTypeNames) {
                    if (typeName == "any") {
                        anyType = true;
                        validType = true;
                        continue;
                    }

                    validType |= valueType == getTypeByName(typeName, path);
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
                    result[path + "/type"] = jon({
                        {"message", mstr("Type mismatch: Expected ", expectedTypeStr, ", got ", typeStr())},
                        {"data", *this},
                        {"keyword", "type"},
                    });
                }
            }

            if (schema.isString()) {
                // Schema is just a type as string, type is checked above thus just don't run other checks
                return;
            }

            if (valueType == Type::Int) {
                auto intValue = get<int_t>();

                if (schema.has("minInt")) {
                    auto min = schema.schemaAt<int_t>("minInt", path);
                    if (intValue < min) {
                        result[path + "/minInt"] = jon({
                            {"message", mstr("Invalid integer size: ", intValue, " is less than ", min)},
                            {"data", *this},
                            {"keyword", "minInt"},
                        });
                    }
                }

                if (schema.has("maxInt")) {
                    auto max = schema.schemaAt<int_t>("maxInt", path);
                    if (intValue > max) {
                        result[path + "/maxInt"] = jon({
                            {"message", mstr("Invalid integer value: ", intValue, " is greater than ", max)},
                            {"data", *this},
                            {"keyword", "maxInt"},
                        });
                    }
                }
            } else if (valueType == Type::Float) {
                auto floatValue = get<float_t>();

                if (schema.has("minFloat")) {
                    auto min = schema.schemaAt<float_t>("minFloat", path);
                    if (floatValue < min) {
                        result[path + "/minFloat"] = jon({
                            {"message", mstr("Invalid float value: ", floatValue, " is less than ", min)},
                            {"data", *this},
                            {"keyword", "minFloat"},
                        });
                    }
                }

                if (schema.has("maxFloat")) {
                    auto max = schema.schemaAt<float_t>("maxFloat", path);
                    if (floatValue > max) {
                        result[path + "/maxFloat"] = jon({
                            {"message", mstr("Invalid float value: ", floatValue, " is greater than ", max)},
                            {"data", *this},
                            {"keyword", "maxFloat"},
                        });
                    }
                }
            } else if (valueType == Type::String) {
                const auto & stringValue = get<str_t>();

                if (schema.has("minLen")) {
                    auto min = schema.schemaAt<int_t>("minLen", path);
                    if (stringValue.size() < min) {
                        result[path + "/minLen"] = jon({
                            {"message", mstr("Invalid string length: ", stringValue.size(), " is less than ", min)},
                            {"data", *this},
                            {"keyword", "minLen"},
                        });
                    }
                }

                if (schema.has("maxLen")) {
                    auto max = schema.schemaAt<int_t>("maxLen", path);
                    if (stringValue.size() > max) {
                        result[path + "/maxLen"] = jon({
                            {"message", mstr("Invalid string length: ", stringValue.size(), " is greater than ", max)},
                            {"data", *this},
                            {"keyword", "maxLen"},
                        });
                    }
                }

                if (schema.has("pattern")) {
                    // TODO: Return parts failed to match
                    const auto pattern = schema.schemaAt<str_t>("pattern", path);
                    const std::regex regex(pattern);
                    if (not std::regex_match(stringValue, regex)) {
                        result[path + "/pattern"] = jon({
                            {"message", mstr("Invalid string value: '", stringValue, "' does not match pattern '", pattern, "'")},
                            {"data", *this},
                            {"keyword", "pattern"},
                        });
                    }
                }
            } else if (valueType == Type::Array) {
                const auto & arrayValue = get<arr_t>();

                if (schema.has("minSize")) {
                    auto min = schema.schemaAt<int_t>("minSize", path);
                    if (arrayValue.size() < min) {
                        result[path + "/minSize"] = jon({
                            {"message", mstr("Invalid array size: ", arrayValue.size(), " is less than ", min)},
                            {"data",    *this},
                            {"keyword", "minSize"},
                        });
                    }
                }

                if (schema.has("maxSize")) {
                    auto max = schema.schemaAt<int_t>("maxSize", path);
                    if (arrayValue.size() > max) {
                        result[path + "/maxSize"] = jon({
                            {"message", mstr("Invalid array size: ", arrayValue.size(), " is greater than ", max)},
                            {"data", *this},
                            {"keyword", "maxSize"},
                        });
                    }
                }

                if (schema.has("items")) {
                    auto itemsSchema = schema.schemaAt<arr_t>("items", path);
                    size_t index{0};
                    for (const auto & el : arrayValue) {
                        const auto & itemPath = path + "/" + std::to_string(index);
                        el._validate(std::move(itemsSchema), itemPath, result[itemPath]);
                        index++;
                    }
                }
            } else if (valueType == Type::Object) {
                const auto & objectValue = get<obj_t>();

                if (schema.has("minProps")) {
                    auto min = schema.schemaAt<int_t>("minProps", path);
                    if (objectValue.size() < min) {
                        result[path + "/minProps"] = jon({
                            {"message", mstr("Invalid object properties count: ", objectValue.size(), " is less than ", min)},
                            {"data", *this},
                            {"keyword", "minProps"},
                        });
                    }
                }

                if (schema.has("maxProps")) {
                    auto max = schema.schemaAt<int_t>("maxProps", path);
                    if (objectValue.size() > max) {
                        result[path + "/maxProps"] = jon({
                            {"message", mstr("Invalid object properties count: ", objectValue.size(), " is greater than ", max)},
                            {"data", *this},
                            {"keyword", "maxProps"},
                        });
                    }
                }

                bool extras = schema.has("extras") and schema.schemaAt<bool_t>("extras", path);

                if (schema.has("props")) {
                    const auto & props = schema.schemaAt<obj_t>("props", path);

                    std::vector<std::string> checkedProps;

                    for (const auto & entry : objectValue) {
                        const auto & prop = props.find(entry.first);
                        const auto entryPath = path + "/" + entry.first;
                        if (not extras and prop == props.end()) {
                            result[entryPath + "/extras"] = jon({
                                {"message", "Extra property (`extras` are not allowed)"},
                                {"data", entry.second},
                                {"keyword", "extras"},
                            });
                        } else {
                            entry.second._validate(prop->second, entryPath, result[entryPath]);
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
                            result[path + "/" + prop.first] = jon({ 1, 2, 3});
                        }
                    }
                } else if (not extras and not objectValue.empty()) {
                    result[path + "/extras"] = jon({
                        {"message", mstr("No properties allowed in this object as `extras: false` and no `props` specified")},
                        {"data", *this},
                        {"keyword", "extras"},
                    });
                }
            }

            if (schema.has("anyOf")) {
                const auto & anyOf = schema.schemaAt<arr_t>("anyOf", path);

                bool someValid = false;
                for (const auto & subSchema : anyOf) {
                    // Not, use `validate`, but not `_validate` as we don't need an error,
                    //  just the fact that some of `anyOf` variants matched
                    const auto subSchemaResult = validate(subSchema);
                    if (subSchemaResult.isNull()) {
                        someValid = true;
                        break;
                    }
                }
                if (not someValid) {
                    result[path + "/anyOf"] = jon({
                        {"message", "Does not match `anyOf` schemas"},
                        {"data", {}},
                        {"keyword", "anyOf"},
                    });
                }
            }

            if (schema.has("oneOf")) {
                const auto & oneOf = schema.schemaAt<arr_t>("oneOf", path);

                bool oneValid = false;
                for (const auto & subSchema : oneOf) {
                    const auto & subSchemaResult = validate(subSchema);
                    if (subSchemaResult.isNull()) {
                        if (oneValid) {
                            result[path + "/oneOf"] = jon({
                                {"message", "Matches more than `oneOf` schemas"},
                                {"data", {}},
                                {"keyword", "anyOf"},
                            });
                        }
                        oneValid = true;
                        break;
                    }
                }
                if (not oneValid) {
                    result[path + "/oneOf"] = jon({
                        {"message", "Does not match any of `oneOf` schemas"},
                        {"data", {}},
                        {"keyword", "oneOf"},
                    });
                }
            }

            if (schema.has("allOf")) {
                const auto & allOf = schema.schemaAt<arr_t>("allOf", path);

                for (const auto & subSchema : allOf) {
                    const auto & subSchemaResult = validate(subSchema);
                    if (not subSchemaResult.isNull()) {
                        result[path + "/allOf"] = jon({
                            {"message", "Does not `allOf` schemas"},
                            {"data", {}},
                            {"keyword", "allOf"},
                        });
                        break;
                    }
                }
            }

            if (schema.has("not")) {
                if (schema.at("not").isArray()) {
                    for (const auto & subSchema : schema.schemaAt<arr_t>("not", path)) {
                        const auto & subSchemaResult = validate(subSchema);
                        if (subSchemaResult.isNull()) {
                            result[path + "/not"] = jon({
                                {"message", "Matches some of `not` schemas"},
                                {"data", {}},
                                {"keyword", "not"},
                            });
                            break;
                        }
                    }
                } else {
                    if (validate(schema.at("not")).isNull()) {
                        result[path + "/not"] = jon({
                            {"message", "Matches `not` schema"},
                            {"data", {}},
                            {"keyword", "not"},
                        });
                    }
                }
            }
        }

    public:
        jon toErrorList() const {
            if (isNull()) {
                return jon {};
            }

            if (isBool()) {
                throw jon_exception("`bool` is not a schema result type");
            }

            if (isInt()) {
                throw jon_exception("`int` is not a schema result type");
            }

            if (isFloat()) {
                throw jon_exception("`float` is not a schema result type");
            }

            if (isString()) {
                return *this;
            }

            if (isObject()) {
                jon list {arr_t {}};
                for (const auto & entry : flatten().get<obj_t>()) {
                    list.push(jon {str_t {mstr(entry.first, ": ", entry.second.dump())}});
                }
                return list;
            }

            if (isArray()) {
                throw jon_exception("`array` is not a schema result type");
            }

            throw jon_exception("[jon bug]: Unhandled type in `jon::toErrorList`");
        }

    private:
        static Type getTypeByName(const std::string & name, const std::string & path) {
            static const std::map<std::string, jon::Type> typeNames = {
                {"null",   jon::Type::Null},
                {"bool",   jon::Type::Bool},
                {"int",    jon::Type::Int},
                {"float",  jon::Type::Float},
                {"string", jon::Type::String},
                {"object", jon::Type::Object},
                {"array",  jon::Type::Array},
            };

            const auto & foundType = typeNames.find(name);
            if (foundType == typeNames.end()) {
                throw invalid_schema("unknown `type` '" + name + "'", path + "/type");
            }
            return foundType->second;
        }

        template<class T>
        static constexpr const char * typeStrArticle() {
            if constexpr (std::is_same<T, null_t>::value) {
                return "a null";
            }

            if constexpr (std::is_same<T, bool_t>::value) {
                return "a bool";
            }

            if constexpr (std::is_same<T, int_t>::value) {
                return "an int";
            }

            if constexpr (std::is_same<T, float_t>::value) {
                return "a float";
            }

            if constexpr (std::is_same<T, str_t>::value) {
                return "a string";
            }

            if constexpr (std::is_same<T, obj_t>::value) {
                return "an object";
            }

            if constexpr (std::is_same<T, arr_t>::value) {
                return "an array";
            }

            throw std::logic_error("[jon bug] called `typeStr<T>` with non-supported type `T`");
        }

        /// Helper overload for schema validation, throws `invalid_error` instead of `type_error`
        template<class T>
        const T & schemaAt(const str_t & key, const std::string & path) const {
            try {
                return at<T>(key);
            } catch (type_error & te) {
                throw invalid_schema(mstr(key, " must be ", typeStrArticle<T>()), path + "/" + key);
            }
        }
    };

    namespace literal {
        static inline jon operator""_jon(const char * str, std::size_t n) {
            return jon::parse(std::string(str, n));
        }
    }
}

#endif // JON_JON_H
