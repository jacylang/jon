#ifndef JON_SCHEMA_H
#define JON_SCHEMA_H

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

                bool status = true;

                if (schema.has("minLen")) {
                    status &= stringValue.size() >= schema.at<jon::int_t>("minLen");
                }

                if (schema.has("maxLen")) {
                    status &= stringValue.size() <= schema.at<jon::int_t>("maxLen");
                }

                return status;
            } else if (expectedType == jon::Type::Array) {
                const auto & arrayValue = value.get<jon::arr_t>();

                bool status = true;

                if (schema.has("minSize")) {
                    status &= arrayValue.size() >= schema.at<jon::int_t>("minSize");
                }

                if (schema.has("maxSize")) {
                    status &= arrayValue.size() <= schema.at<jon::int_t>("maxSize");
                }

                const auto & itemsSchema = schema.at("items");

                for (const auto & value : arrayValue) {
                    status &= validate(value, itemsSchema);
                }

                return status;
            } else if (expectedType == jon::Type::Object) {
                const auto & objectValue = value.get<jon::obj_t>();

                bool status = true;

                if (schema.has("minProps")) {
                    status &= objectValue.size() >= schema.at<jon::int_t>("minProps");
                }

                if (schema.has("maxProps")) {
                    status &= objectValue.size() <= schema.at<jon::int_t>("maxProps");
                }

                const auto & props = schema.at<jon::obj_t>("props");
                std::vector<std::string> checkedProps;
                for (const auto & entry : objectValue) {
                    if (props.find(entry.first) == props.end()) {
                        status = false;
                    } else {
                        status &= validate(entry.second, props.at(entry.first));
                        checkedProps.emplace_back(entry.first);
                    }
                }

                if (checkedProps.size() != props.size()) {
                    return false;
                }

                return status;
            }
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

