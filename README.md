# JON 

JON (Jacy Object Notation) is an alternative to JSON used by Jacy programming
 language.

**This is a C++ library to work with JON**

### API 

##### `jon::jon::fromFile`

```c++
static jon fromFile(const std::filesystem::path & path);
```

Read contents of file by path, parses it and returns `jon` value.

##### `jon::jon::fromSource`

```c++
static jon fromSource(const str_t & source);
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


##### `jon::jon::contains`

```c++
bool contains(const str_t & key) const noexcept;
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
const jon & at(const str_t & key) const;
jon & at(const str_t & key);

const jon & at(size_t idx) const;
jon & at(size_t idx);
```

Same as `operator[]` but with bound checks.

##### `jon::jon::stringify`

```c++
std::string stringify(const Indent & indent = {"", -1}) const;
std::string stringify(const std::string & indentStr) const;
```

Converts JON value to string.
Second variant (with `indentStr`) is a shortcut for first one, and is an
 equivalent to:
```c++
stringify(Indent {indentStr, 0});
```

By default, `stringify` returns escaped string, if no indent provided and
 default one chosen.
`Indent` with `size = -1` means that no indentation will be applied and
 output string won't be prettified.

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