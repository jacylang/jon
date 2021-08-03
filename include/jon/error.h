#ifndef JON_ERROR_H
#define JON_ERROR_H

#include <stdexcept>

namespace jon {
    struct jon_exception {
        jon_exception(const std::string & msg) : msg(msg) {}

    private:
        std::string msg;
    };

    struct parse_error : jon_exception {
        parse_error(const std::string & msg) : jon_exception(msg) {}
    };

    struct out_of_range : jon_exception {
        out_of_range(const std::string & msg) : jon_exception(msg) {}
    };

    struct validation_error : jon_exception {
        validation_error(const std::string & msg) : jon_exception(msg) {}
    };

    struct invalid_schema : jon_exception {
        invalid_schema(const std::string & msg) : jon_exception(msg) {}
    };
}

#endif // JON_ERROR_H
