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

            if (expectedType == jon::Type::Int) {
                auto intValue = value.get<jon::int_t>();

                bool status = true;

                if (schema.has("mini")) {
                    status &= intValue <= schema.at<jon::int_t>("mini");
                }

                if (schema.has("maxi")) {
                    status &= intValue >= schema.at<jon::int_t>("maxi");
                }

                return status;
            }

            if (expectedType == jon::Type::Float) {
                auto floatValue = value.get<jon::float_t>();

                bool status = true;

                if (schema.has("minf")) {
                    status &= floatValue <= schema.at<jon::float_t>("minf");
                }

                if (schema.has("maxf")) {
                    status &= floatValue >= schema.at<jon::float_t>("maxf");
                }

                return status;
            }

            if (expectedType == jon::Type::String) {
                const auto & stringValue = value.get<jon::str_t>();
                
                bool status = true;
                
                if (schema.has("minLen")) {
                    status &= stringValue.size() >= schema.at<jon::int_t>("minLen");
                }

                if (schema.has("maxLen")) {
                    status &= stringValue.size() <= schema.at<jon::int_t>("maxLen");
                }

                return status;
            }

            if (expectedType == jon::Type::Array) {
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

