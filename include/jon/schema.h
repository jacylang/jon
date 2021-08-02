#ifndef JON_SCHEMA_H
#define JON_SCHEMA_H

#include "jon.h"

namespace jon {
    class Schema {
    public:
        Schema(jon && schema) : schema(std::move(schema)) {}
        ~Schema() = default;

        void validate(const jon & value) {
            
        }

    private:
        jon schema;
    };
}

#endif // JON_SCHEMA_H

