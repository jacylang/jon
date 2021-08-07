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

        // Constructors //
        template<Type> struct jonCtor {};

        template<>
        struct jonCtor<Type::Bool> {
            template<class JonT>
            static void make(JonT & j, typename JonT::bool_t val) noexcept {
                j.value = val;
            }
        };

        template<>
        struct jonCtor<Type::Int> {
            template<class JonT>
            static void make(JonT & j, typename JonT::int_t val) noexcept {
                j.value = val;
            }
        };

        template<>
        struct jonCtor<Type::Float> {
            template<class JonT>
            static void make(JonT & j, typename JonT::float_t val) noexcept {
                j.value = val;
            }
        };

        template<>
        struct jonCtor<Type::String> {
            template<class JonT>
            static void make(JonT & j, const typename JonT::str_t & val) noexcept {
                j.value = val;
            }

            template<class JonT>
            static void make(JonT & j, typename JonT::str_t && val) noexcept {
                j.value = std::move(val);
            }
        };

        template<>
        struct jonCtor<Type::Object> {
            template<class JonT>
            static void make(JonT & j, const typename JonT::obj_t & val) noexcept {
                j.value = val;
            }

            template<class JonT>
            static void make(JonT & j, typename JonT::obj_t && val) noexcept {
                j.value = std::move(val);
            }
        };

        template<>
        struct jonCtor<Type::Array> {
            template<class JonT>
            static void make(JonT & j, const typename JonT::arr_t & val) noexcept {
                j.value = val;
            }

            template<class JonT>
            static void make(JonT & j, typename JonT::arr_t && val) noexcept {
                j.value = std::move(val);
            }
        };

        // Converters //
        template<typename JonT, typename T, std::enable_if_t<std::is_same_v<T, bool_t>, int> = 0>
        void toJon(JonT & j, T val) noexcept {
            jonCtor<Type::Bool>::make(j, val);
        }

        template<typename JonT, typename T, std::enable_if_t<std::is_same_v<T, str_t>, int> = 0>
        void toJon(JonT & j, const T & val) noexcept {
            jonCtor<Type::String>::make(j, val);
        }

        template<typename JonT>
        void toJon(JonT & j, str_t && val) noexcept {
            jonCtor<Type::String>::make(j, std::move(val));
        }

        template<typename JonT, class T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
        void toJon(JonT & j, T val) noexcept {
            jonCtor<Type::Float>::make(j, static_cast<float_t>(val));
        }

        template<typename JonT, class T, std::enable_if_t<std::is_integral_v<T> && !std::is_same_v<T, bool>, int> = 0>
        void toJon(JonT & j, T val) noexcept {
            jonCtor<Type::Int>::make(j, static_cast<int_t>(val));
        }

        template<typename JonT>
        void toJon(JonT & j, const obj_t<JonT> & val) noexcept {
            jonCtor<Type::Object>::make(j, val);
        }

        template<typename JonT>
        void toJon(JonT & j, obj_t<JonT> && val) noexcept {
            jonCtor<Type::Object>::make(j, std::move(val));
        }

        template<typename JonT>
        void toJon(JonT & j, const arr_t<JonT> & val) noexcept {
            jonCtor<Type::Array>::make(j, val);
        }

        template<typename JonT>
        void toJon(JonT & j, arr_t<JonT> && val) noexcept {
            jonCtor<Type::Array>::make(j, std::move(val));
        }

        template<typename JonT, typename T1, typename T2, std::enable_if_t<
            std::is_constructible_v<JonT, T1> &&
            std::is_constructible_v<JonT, T2>, int> = 0>
        void toJon(JonT & j, const std::pair<T1, T2> & p)
        {
            j = {p.first, p.second};
        }

        struct toJonFunc {
            template<typename JonT, typename T>
            auto operator()(JonT & j, T && val) const noexcept(noexcept(toJon(j, std::forward<T>(val))))
            -> decltype(toJon(j, std::forward<T>(val)), void())
            {
                return toJon(j, std::forward<T>(val));
            }
        };
    }

    namespace {
        template<typename T>
        struct static_const
        {
            static constexpr T value{};
        };

        constexpr const auto & toJon = static_const<detail::toJonFunc>::value;
    }

    namespace detail {
        // Serializer //
        template<class VT>
        struct Serializer {
            template<class JonT, class TT = VT>
            static auto toJon(JonT & j, TT && val) noexcept(
            noexcept(::jacylang::toJon(j, std::forward<TT>(val)))
            ) -> decltype(::jacylang::toJon(j, std::forward<TT>(val)), void()) {
                ::jacylang::toJon(j, std::forward<TT>(val));
            }
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

        using obj_el_t = std::pair<str_t, jon>;
        using storage_t = std::variant<null_t, bool_t, int_t, float_t, str_t, obj_t, arr_t>;

        using Type = detail::Type;

        template<detail::Type> friend struct detail::jonCtor;

        static std::string typeStr(Type type) {
            switch (type) {
                case Type::Null: return "null";
                case Type::Bool: return "bool";
                case Type::Int: return "int";
                case Type::Float: return "float";
                case Type::String: return "string";
                case Type::Object: return "object";
                case Type::Array: return "array";
            }
        }

        std::string typeStr() const {
            return typeStr(type());
        }

        // Value //
    private:
        storage_t value;

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
                value = arr_t {};
            } else {
                assertTypeArray();
            }
        }

        void assertTypeObject(const std::string & key) const {
            assertType(Type::Object, mstr("Cannot access property ", key, " of ", typeStr()));
        }

        void assertObjectFirstAccess(const std::string & key) {
            if (type() == Type::Null) {
                value = obj_t {};
            } else {
                assertTypeObject(key);
            }
        }

        template<class T>
        constexpr void getTypeAssert() const {
            if constexpr (std::is_same_v<T, null_t>) {
                assertType(Type::Null, "called `get<null_t>` with not a `null_t` `jon`");
            }
            else if constexpr (std::is_same_v<T, bool_t>) {
                assertType(Type::Bool, "called `get<bool_t>` with not a `bool_t` `jon`");
            }
            else if constexpr (std::is_same_v<T, int_t>) {
                assertType(Type::Int, "called `get<int_t>` with not a `int_t` `jon`");
            }
            else if constexpr (std::is_same_v<T, float_t>) {
                assertType(Type::Float, "called `get<float_t>` with not a `float_t` `jon`");
            }
            else if constexpr (std::is_same_v<T, str_t>) {
                assertType(Type::String, "called `get<str_t>` with not a `str_t` `jon`");
            }
            else if constexpr (std::is_same_v<T, obj_t>) {
                assertType(Type::Object, "called `get<obj_t>` with not a `obj_t` `jon`");
            }
            else if constexpr (std::is_same_v<T, arr_t>) {
                assertType(Type::Array, "called `get<arr_t>` with not a `arr_t` `jon`");
            }
            else {
                throw type_error("called `get` with invalid type");
            }
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

        // Constructors //
    public:
        jon(std::nullptr_t = nullptr) noexcept : value(null_t {}) {}

        jon(Type t) noexcept {
            switch (t) {
                case Type::Null: {
                    value = null_t {};
                    break;
                }
                case Type::Bool: {
                    value = bool_t {};
                    break;
                }
                case Type::Int: {
                    value = int_t {};
                    break;
                }
                case Type::Float: {
                    value = float_t {};
                    break;
                }
                case Type::String: {
                    value = str_t {};
                    break;
                }
                case Type::Object: {
                    value = obj_t {};
                    break;
                }
                case Type::Array: {
                    value = arr_t {};
                    break;
                }
            }
        }

        template<class T, class U = std::remove_cv<std::remove_reference_t<T>>,
            std::enable_if_t<!std::is_same_v<U, jon>, int> = 0>
        jon(T && val) {
            detail::toJon(*this, std::forward<T>(val));
        }

        jon(const detail::jon_ref<jon> & ref) : jon(ref.get()) {}

        jon(const jon & other) noexcept : value(other.value) {}
        jon(jon && other) noexcept : value(std::move(other.value)) {}

        jon(std::initializer_list<detail::jon_ref<jon>> init, bool typeDeduction = true, Type type = Type::Array) {
            if (init.size() == 0) {
                value = obj_t {};
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
                value = obj_t {};
                for (auto & el : init) {
                    auto pair = el.get().get<arr_t>();
                    get<obj_t>().emplace(pair.at(0).get<str_t>(), std::move(pair.at(1)));
                }
            } else {
                value = arr_t(init.begin(), init.end());
            }
        }

        jon & operator=(jon other) noexcept (
            std::is_nothrow_move_constructible_v<storage_t> &&
            std::is_nothrow_move_assignable_v<storage_t> &&
            std::is_nothrow_move_constructible_v<storage_t> &&
            std::is_nothrow_move_assignable_v<storage_t>
        ) {
            std::swap(value, other.value);
            return *this;
        }

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
            return fromAst(parser.parse(source));
        }

        // Common methods //
    public:
        template<class T>
        constexpr T & get() {
            getTypeAssert<T>();
            return std::get<T>(value);
        }

        template<class T>
        constexpr const T & get() const {
            getTypeAssert<T>();
            return std::get<T>(value);
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
        friend bool operator==(const jon & lhs, const T & rhs) noexcept {
            return lhs == jon(rhs);
        }

        template<class T, typename = typename std::enable_if_t<std::is_scalar_v<T>, T>>
        friend bool operator==(const T & lhs, const jon & rhs) noexcept {
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
        Type type() const noexcept {
            return static_cast<Type>(value.index());
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
            auto obj = get<obj_t>();
            auto it = obj.find(key);
            if (it == obj.end()) {
                throw out_of_range("`at` by key '" + key + "'");
            }
            return (*it).second;
        }

        jon & at(const str_t & key) {
            assertObjectFirstAccess(key);
            auto & obj = get<obj_t>();
            auto it = obj.find(key);
            if (it == obj.end()) {
                throw out_of_range("`at` by key '" + key + "'");
            }
            return (*it).second;
        }

        template<class T>
        const T & at(const str_t & key) const {
            assertTypeObject(key);
            const auto & obj = get<obj_t>();
            auto it = obj.find(key);
            if (it == obj.end()) {
                throw out_of_range("`at` by key '" + key + "'");
            }
            return (*it).second.get<T>();
        }

        template<class T>
        T & at(const str_t & key) {
            assertObjectFirstAccess(key);
            auto obj = get<obj_t>();
            auto it = obj.find(key);
            if (it == obj.end()) {
                throw out_of_range("`at` by key '" + key + "'");
            }
            return (*it).second.get<T>();
        }

        jon flatten() const {
            jon result {arr_t {}};
            _flatten("", *this, result);
            return result;
        }

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
            if (idx > get<arr_t>().size()) {
                throw out_of_range(mstr("`at` by index '", idx, "'"));
            }
            return get<arr_t>().at(idx);
        }

        jon & at(size_t idx) {
            assertTypeArray();
            if (idx > get<arr_t>().size()) {
                throw out_of_range(mstr("`at` by index '", idx, "'"));
            }
            return get<arr_t>().at(idx);
        }

        void push(const jon & el) {
            assertArrayFirstAccess();
            get<arr_t>().push_back(el);
        }

        // Serialization/Deserialization //
    private:
        static jon fromAst(ast::value_ptr && ast) {
            switch (ast->kind) {
                case ast::ValueKind::Null: {
                    return jon {};
                }
                case ast::ValueKind::Bool: {
                    return jon {ast::Value::as<ast::Bool>(std::move(ast))->val};
                }
                case ast::ValueKind::Int: {
                    return jon {ast::Value::as<ast::Int>(std::move(ast))->val};
                }
                case ast::ValueKind::Float: {
                    return jon {ast::Value::as<ast::Float>(std::move(ast))->val};
                }
                case ast::ValueKind::String: {
                    return jon {ast::Value::as<ast::String>(std::move(ast))->val};
                }
                case ast::ValueKind::Object: {
                    obj_t entries;
                    for (auto && keyVal : ast::Value::as<ast::Object>(std::move(ast))->entries) {
                        entries.emplace(keyVal.key.val, fromAst(std::move(keyVal.val)));
                    }
                    return jon {entries};
                }
                case ast::ValueKind::Array: {
                    arr_t values;
                    for (auto && val : ast::Value::as<ast::Array>(std::move(ast))->values) {
                        values.emplace_back(fromAst(std::move(val)));
                    }
                    return jon {values};
                }
                default: {
                    throw std::logic_error("[jon bug]: Unhandled `ast::ValueKind` in `jon::fromAst");
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

            jon i = {{
                {"aasdas", 123}
            }};

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
                            {"data", *this},
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
                    const auto & itemsSchema = schema.schemaAt<arr_t>("items", path);
                    size_t index{0};
                    for (const auto & el : arrayValue) {
                        const auto & itemPath = path + "/" + std::to_string(index);
                        el._validate(itemsSchema, itemPath, result[itemPath]);
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
                            result[path + "/" + prop.first] = jon({
                                {"message", "Missing property"},
                                {"data", {}},
                                {"keyword", "!optional"},
                            });
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
            if constexpr (std::is_same_v<T, null_t>) {
                return "a null";
            }
            else if constexpr (std::is_same_v<T, bool_t>) {
                return "a bool";
            }
            else if constexpr (std::is_same_v<T, int_t>) {
                return "an int";
            }
            else if constexpr (std::is_same_v<T, float_t>) {
                return "a float";
            }
            else if constexpr (std::is_same_v<T, str_t>) {
                return "a string";
            }
            else if constexpr (std::is_same_v<T, obj_t>) {
                return "an object";
            }
            else if constexpr (std::is_same_v<T, arr_t>) {
                return "an array";
            }
            else {
                throw std::logic_error("[jon bug] called `typeStr<T>` with non-supported type `T`");
            }
        }

        bool schemaCheck(Type type, const std::string & errorMsg, const std::string & path) const {
            try {
                check(type);
            } catch (type_error & te) {
                throw invalid_schema(errorMsg, path);
            }
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
