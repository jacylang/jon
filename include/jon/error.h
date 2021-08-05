#ifndef JON_ERROR_H
#define JON_ERROR_H

#include <stdexcept>

namespace jon {
    struct jon_exception : std::exception {
        jon_exception(const std::string & msg) : msg(msg) {}

        const char * what() const noexcept override {
            return msg.c_str();
        }

    private:
        std::string msg;
    };

    struct parse_error : jon_exception {
        parse_error(const std::string & msg) : jon_exception("[jon::parse_error]: " + msg) {}
    };

    struct type_error : jon_exception {
        type_error(const std::string & msg) : jon_exception("[jon::type_error]: " + msg) {}
    };

    struct out_of_range : jon_exception {
        out_of_range(const std::string & msg) : jon_exception("[jon::out_of_range] " + msg) {}
    };

    struct validation_error : jon_exception {
        validation_error(const std::string & msg) : jon_exception("[jon::validation_error]" + msg) {}
    };

    struct invalid_schema : jon_exception {
        invalid_schema(const std::string & msg, const std::string & path) : jon_exception("[jon::invalid_schema]: " + msg + " '" + path + "'") {}
    };
}

#endif // JON_ERROR_H
