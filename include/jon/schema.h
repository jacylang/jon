#ifndef JON_SCHEMA_H
#define JON_SCHEMA_H

#include "jon.h"

namespace jon {
    class Schema {
    public:
        Schema() = default;
        ~Schema() = default;

        bool validate(const jon & value, const jon & schema) {
            const auto & expectedTypeName = schema.at<jon::str_t>("type");
            const auto expectedType = typeNames.at(expectedTypeName);
            const auto nullable = schema.has("nullable") and schema["nullable"].get<jon::bool_t>();

            if (nullable and value.isNull()) {
                return true;
            }

            if (value.type() != expectedType) {
                return false;
            }

            if (expectedType == jon::Type::String) {
                const auto & stringValue = value.get<jon::str_t>();
                
                bool status = true;
                if (schema.has("maxLen")) {
                    status &= stringValue.size() <= schema.at<jon::int_t>("maxLen");
                }

                if (schema.has("minLen")) {
                    status &= stringValue.size() >= schema.at<jon::int_t>("minLen");
                }

                return status;
            }

            if (expectedType == jon::Type::Array) {
                const auto & arrayValue = value.get<jon::arr_t>();

                bool status = true;
                if (schema.has("maxSize")) {
                    status &= arrayValue.size() <= schema.at<jon::int_t>("maxSize");
                }

                if (schema.has("minSize")) {
                    status &= arrayValue.size() >= schema.at<jon::int_t>("minSize");
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

