#ifndef JON_SCHEMA_H
#define JON_SCHEMA_H

#include <algorithm>

#include "jon.h"

namespace jon {
    class Schema {
    public:
        Schema() = default;
        ~Schema() = default;

        jon validate(const jon & value, const jon & schema) {
            const auto & expectedTypeName = schema.at<jon::str_t>("type");
            const auto expectedType = typeNames.at(expectedTypeName);
            const auto nullable = schema.has("nullable") and schema["nullable"].get<jon::bool_t>();

            if (nullable and value.isNull()) {
                return jon {};
            }

            if (value.type() != expectedType) {
                return jon {
                    mstr("Type mismatch: Expected ", jon::typeStr(expectedType), ", got ", value.typeStr())
                };
            } else if (expectedType == jon::Type::Int) {
                auto intValue = value.get<jon::int_t>();

                if (schema.has("mini")) {
                    auto min = schema.at<jon::int_t>("mini");
                    if (intValue < min) {
                        return jon {
                            mstr("Invalid integer size: ", intValue, " is less than ", min)
                        };
                    }
                }

                if (schema.has("maxi")) {
                    auto max = schema.at<jon::int_t>("maxi");
                    if (intValue > max) {
                        return jon {
                            mstr("Invalid integer size: ", intValue, " is greater than ", max)
                        };
                    }
                }
            } else if (expectedType == jon::Type::Float) {
                auto floatValue = value.get<jon::float_t>();

                if (schema.has("minf")) {
                    auto min = schema.at<jon::int_t>("minf");
                    if (floatValue < min) {
                        return jon {
                            mstr("Invalid float size: ", floatValue, " is less than ", min)
                        };
                    }
                }

                if (schema.has("maxf")) {
                    auto max = schema.at<jon::int_t>("maxf");
                    if (floatValue > max) {
                        return jon {
                            mstr("Invalid float size: ", floatValue, " is greater than ", max)
                        };
                    }
                }
            } else if (expectedType == jon::Type::String) {
                const auto & stringValue = value.get<jon::str_t>();

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
            } else if (expectedType == jon::Type::Array) {
                const auto & arrayValue = value.get<jon::arr_t>();

                jon result {jon::arr_t {}};
                const auto & itemsSchema = schema.at("items");
                size_t index{0};
                for (const auto & el : arrayValue) {
                    result[index++] = validate(el, itemsSchema);
                }

                if (not result.empty()) {
                    return result;
                }

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
            } else if (expectedType == jon::Type::Object) {
                const auto & objectValue = value.get<jon::obj_t>();

                jon result {jon::obj_t {}};

                const auto & props = schema.at<jon::obj_t>("props");
                std::vector<std::string> checkedProps;
                for (const auto & entry : objectValue) {
                    // TODO: additionalProperties
                    if (props.find(entry.first) == props.end()) {
                        result[entry.first] = jon {jon::str_t {"Additional property"}};
                    } else {
                        result[entry.first] = validate(entry.second, props.at(entry.first));
                        checkedProps.emplace_back(entry.first);
                    }
                }

                if (checkedProps.size() != props.size()) {
                    for (const auto & prop : props) {
                        if (std::find(checkedProps.begin(), checkedProps.end(), prop.first) != checkedProps.end()) {
                            continue;
                        }
                        result[prop.first] = jon {jon::str_t {"Missing property"}};
                    }
                }

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
            }

            return jon {};
        }

    private:
        static const std::map<std::string, jon::Type> typeNames;
    };

    const std::map<std::string, jon::Type> Schema::typeNames = {
        {"null", jon::Type::Null},
        {"bool", jon::Type::Bool},
        {"int", jon::Type::Int},
        {"float", jon::Type::Float},
        {"string", jon::Type::String},
        {"object", jon::Type::Object},
        {"array", jon::Type::Array},
    };
}

#endif // JON_SCHEMA_H

