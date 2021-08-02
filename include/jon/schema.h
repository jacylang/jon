#ifndef JON_SCHEMA_H
#define JON_SCHEMA_H

#include "jon.h"

namespace jon {
    class Schema {
    public:
        Schema() = default;
        ~Schema() = default;

        void validate(const jon & value, const jon & schema) {
            std::string type = schema.at("type").check(jon::Type::String).get<jon::str_t>();
            
        }
    };
}

#endif // JON_SCHEMA_H

