#include "doctest/doctest.h"
#include "jon/jon.h"

using jon = jacylang::jon;

TEST_CASE("constructors") {
    SUBCASE("empty (null_t implicit)") {
        auto empty_ctor = jon {};
        CHECK(empty_ctor.isNull());
    }

    SUBCASE("nullptr_t (explicit)") {
        auto nullptr_t_ctor = jon {nullptr};
        CHECK(nullptr_t_ctor.isNull());
    }

    SUBCASE("empty variant types") {
        SUBCASE ("null_t") {
            auto null_t_ctor = jon {jon::null_t {}};
            CHECK(null_t_ctor.type() == jon::Type::Null);
        }
        SUBCASE ("bool_t") {
            auto bool_t_ctor = jon {jon::bool_t {}};
            CHECK(bool_t_ctor.type() == jon::Type::Bool);
            CHECK(bool_t_ctor == false);
        }
        SUBCASE ("int_t") {
            auto int_t_ctor = jon {jon::int_t {}};
            CHECK(int_t_ctor.type() == jon::Type::Int);
            CHECK(int_t_ctor == 0);
        }
        SUBCASE ("float_t") {
            auto float_t_ctor = jon {jon::float_t {}};
            CHECK(float_t_ctor.type() == jon::Type::Float);
            CHECK(float_t_ctor == 0.0);
        }
        SUBCASE ("str_t") {
            auto str_t_ctor = jon {jon::str_t {}};
            CHECK(str_t_ctor.type() == jon::Type::String);
            CHECK(str_t_ctor == "");
        }
        SUBCASE ("obj_t") {
            auto obj_t_ctor = jon {jon::obj_t {}};
            CHECK(obj_t_ctor.type() == jon::Type::Object);
        }
        SUBCASE ("arr_t") {
            auto arr_t_ctor = jon {jon::arr_t {}};
            CHECK(arr_t_ctor.type() == jon::Type::Array);
        }
    }
}
