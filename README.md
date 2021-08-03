# JON 

JON (Jacy Object Notation) is an alternative to JSON used by Jacy programming
 language.

**This is a C++ library to work with JON**

### Features

- Simple API to work with JON values
- Serialization / Deserialization
- JON string literals (`"..."_jon`)
- Built-in JON schema validator [Schemas](#schemas)

### API 

##### `jon::jon::fromFile`

```c++
static jon fromFile(const std::filesystem::path & path);
```

Read contents of file by path, parses it and returns `jon` value.

##### `jon::jon::parse`

```c++
static jon parse(const str_t & source);
```

Parses source string and returns `jon` value.

##### `jon::jon::get`

```c++
template<class T>
T get() const noexcept;
```

Returns stored value by type.

| JON Type | Return type |
|:--------:|:-----------:|
| `null` | `std::monostate` |
| `bool` | `bool` |
| `int` | `int64_t` |
| `float` | `double` |
| `string` | `std::string` |
| `object` | `std::map<std::string, jon>` |
| `array` | `std::vector<jon>` |

##### `jon::jon::empty`

```c++
bool empty() const noexcept;
```

Checks if value is empty, only collection-like types have dynamic size, `null
` is zero-sized and `bool`, `int` and `double` are 1-sized.


##### `jon::jon::has`

```c++
bool has(const str_t & key) const noexcept;
```

Checks if JON object contains key-value pair with given key, if value is not
 an object - returns `false`.

##### `jon::jon::clear`

```c++
void clear() noexcept;
```

Clears value, resetting it to default-constructed.

Default values for JON types are:
| JON Type | Default value |
|:--------:|:-------------:|
| `null` | `null` |
| `bool` | `false` |
| `int` | `0` |
| `float` | `0.0` |
| `string` | `""` |
| `object` | `{}` |
| `array` | `[]` |

##### `jon::jon::size`

```c++
size_t size() const noexcept;
```

Returns size of value, `null` is always zero-sized, `bool`, `int` and `double
` are 1-sized and `string`, `object` and `array` are dynamically-sized.

| JON Type | Size |
|:--------:|:----:|
| `null` | `0` |
| `bool` | `1` |
| `int` | `1` |
| `float` | `1` |
| `string` | Result of `std::string::size()` |
| `object` | Result of `std::map<std::string, jon>::size()` |
| `array` | Result of `std::vector<jon>::size()` |

##### `jon::jon::type`

Returns `jon::Type` of value.

```c++
enum class Type {
    Null,
    Bool,
    Int,
    Float,
    String,
    Object,
    Array,
}
```

##### `jon::typeStr`

```c++
static std::string typeStr(Type type);
std::string typeStr() const;
```

Returns type as string.

##### `jon::jon::is*`

```c++
bool isNull() const noexcept;
bool isBool() const noexcept;
bool isInt() const noexcept;
bool isFloat() const noexcept;
bool isString() const noexcept;
bool isObject() const noexcept;
bool isArray() const noexcept;
```

Simple type-checking methods.

##### `jon::jon::check`

```c++
jon & check(Type expectedType) const;
```

Checks that value is of given type and returns `*this`, otherwise throws an
 `std::runtime_error` exception.

##### `jon::jon::operator[]`

```c++
const jon & operator[](const str_t & key) const;
jon & operator[](const str_t & key);

const jon & operator[](size_t idx) const;
jon & operator[](size_t idx);
```

Element-access operators without bound checks, those that receive `str_t` are
 used for objects, those that receive `size_t` are used for arrays.

##### `jon::jon::at`

```c++
(1) const jon & at(const str_t & key) const;
(2) jon & at(const str_t & key);

(3) const jon & at(size_t idx) const;
(4) jon & at(size_t idx);

template<class T>
(5) const T & at(const str_t & key) const;

template<class T>
(6) T & at(const str_t & key);
```

(1), (2), (3), (4) are same as `operator[]` but with bound checks, throw an
 exception if nothing found.

(5), (6) are shortcuts for `at` + `get` and are equivalents to:
```c++
at(key).get<T>()
```

##### `jon::jon::dump`

```c++
(1) std::string dump(const Indent & indent = {"", -1}) const;
(2) std::string dump(const std::string & indentStr) const;
(3) std::string dump(uint16_t spaceSize) const;
```

Converts JON value to string.

(2) is an equivalent to:
```c++
dump(Indent {indentStr, 0});
```

(3) is an equivalent to:
```c++
dump(Indent {std::string(spaceSize, ' '), 0});
```

By default, `dump` returns escaped string, if no indent provided and
 default one chosen.
`Indent` with `size = -1` means that no indentation will be applied and
 output string won't be prettified.

##### `jon::jon::validate`

```c++
jon validate(const jon & schema) const;
```

Validates JON value by schema, returns `null` jon value if value is valid, otherwise returns schema-like error structure.
Read more about schemas [Schemas](#schemas)

##### `jon::literals::operator""_jon`

```c++
jon operator""_jon(const char * str, std::size_t n);
```

Instantiates JON literal. Requires `using namespace jon::literal`

Examples:
```c++
auto jon1 = "hello: 'world'"_jon;
auto jon2 = R"(
    project-name: 'Jacy'
    version: '1.0.0'
    dependencies: {
        library: '2.1.15'
    }
)"_jon;
```

#### Classes

##### `Indent`

```c++
using size_type = int32_t;
Indent(const std::string & val, size_type size = 0);
```

Simple `struct` used for indentation. Holds indent value as `std::string` and
 size (count of repetitions of string).
If `size` is `-1` then indent won't be written to `std::ostream`.

### <a name="schemas"></a> Schemas

Example:
```c++
const auto schema = R"(
    type: 'object'
    props: {
        title: {
            type: 'string'
            minLen: 8
        }
        values: {
            type: 'array'
            items: {
                type: 'int'
                minInt: 0
            }
        }
    }
)"_jon;

const auto first = R"(
    title: 'short'
    values: [
        1
        2
        3
        -100
    ]
)"_jon;

std::cout << first.validate(schema).dump(2);

/* Prints:
{
    title: 'Invalid string size: 5 is less than 8'
    values: {
        3: 'Invalid integer size: -100 is less than 0'
    }
}
*/
```
#### Type shortcuts

There're 4 ways to specify type of value in schema.

**#1**
```yaml
field: {
    type: '{TYPE}'
}
```

This one allows placing more constraints with other keywords.

**#2**
```yaml
field: '{TYPE}'
```

Shortcut for variant above and is simpler, but you cannot specify constraints for value.

**#3**
```yaml
field: {
    type: ['{TYPE_1}', '{TYPE_2}', ...]
}
```

Here `field` can be of any of given type.

**#4**
```yaml
field: ['{TYPE_1}', '{TYPE_3}', ...]
```

Similar to **#2** variant, but shortcut for **#4** one.

If type is an array you still can specify any constraints for each of these types.
This is why keywords like `minInt`/`maxInt` are not `min`/`max`, because if you have `type: ['int', 'string']` then `min`/`max` would be ambiguous -- are these bounds for `int` value or for `string` size?


**Possible type names**
| JON Type |
|:--------:|
| `null` |
| `bool` |
| `int` |
| `float` |
| `string` |
| `object` |
| `array` |

#### Common Keywords

This keywords can be applied to schema for any type.

- `type` [`string`]: Type of value. (REQUIRED)
- `nullable` [`bool`]: Allows value to be `null`, similar to `type: ['{TYPE}', 'null']`. (Default: `false`)

#### Integer Keywords

- `minInt` [`int`]: Minimum integer value. (Optional)
- `maxInt` [`int`]: Maximum integer value. (Optional)

#### Float Keywords

- `minFloat` [`float`]: Minimum float value. (Optional)
- `maxFloat` [`float`]: Maximum float value. (Optional)

#### String Keywords

- `minLen` [`int`]: Minimum length of string value. (Optional)
- `maxLen` [`int`]: Maximum length of string value. (Optional)
- `pattern` [`string`]: Regular expression to match against a value. (Optional)

#### Object Keywords

- `minProps` [`int`]: Minimum count of properties in object. (Optional)
- `maxProps` [`int`]: Maximum count of properties in object. (Optional)
- `props` [`object`]: Schema object for properties validation. (REQUIRED)
- `extras` [`array`]: Allows additional properties in object. (Default: `[]`)

#### Object properties Keywords

This keywords only allowed for schema which is a property of an object schema.

- `required` [`bool`]: Requires object to contain this property. (Default: `false`)

#### Array Keywords

- `minSize` [`int`]: Minimum size of array. (Optional)
- `maxSize` [`int`]: Maximum size of array. (Optional)
- `items` [`object`]: Schema object for items validation. (REQUIRED)
