include(FetchContent)

FetchContent_Declare(doctest
        GIT_REPOSITORY "https://github.com/onqtam/doctest"
        GIT_TAG "4d8716f1efc1d14aa736ef52ee727bd4204f4c40")

FetchContent_MakeAvailable(doctest)

add_executable(test src/unit-typeStr.cpp)
target_link_libraries(test doctest)
target_link_libraries(test jon)
