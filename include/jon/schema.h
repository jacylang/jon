#ifndef JON_SCHEMA_H
#define JON_SCHEMA_H

#include "jon.h"

namespace jon {
    class Schema {
    public:
        Schema() = default;
        ~Schema() = default;

        void validate(const jon & value, const jon & schema) {
            std::string typeName = schema.at("type").check(jon::Type::String).get<jon::str_t>();

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

