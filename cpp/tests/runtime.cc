#include <codira/codira.h>

#include <catch2/catch_test_macros.hpp>
#include <sstream>

/// Returns the absolute path to the codiralib with the specified name
inline std::string get_codiralib_path(std::string_view name) {
    std::stringstream ss;
    ss << CODIRA_TEST_DIR << name;
    return ss.str();
}

TEST_CASE("runtime can be constructed", "[runtime]") {
    codira::Error err;
    if (auto runtime =
            codira::make_runtime(get_codiralib_path("fibonacci/codira/target/mod.codiralib"), {}, &err)) {
        REQUIRE(err.is_ok());
    } else {
        REQUIRE(err.is_error());
        FAIL(err.message().value());
    }
}

TEST_CASE("runtime can find `FunctionInfo`", "[runtime]") {
    codira::Error err;
    if (auto runtime =
            codira::make_runtime(get_codiralib_path("fibonacci/codira/target/mod.codiralib"), {}, &err)) {
        REQUIRE(err.is_ok());
        REQUIRE(runtime.has_value());

        if (auto function_info = runtime->find_function_info("fibonacci", &err)) {
            REQUIRE(err.is_ok());
        } else {
            REQUIRE(err.is_error());
            FAIL(err.message().value());
        }
    } else {
        REQUIRE(err.is_error());
        FAIL(err.message().value());
    }
}

// TODO: Test hot reloading
TEST_CASE("runtime can update", "[runtime]") {
    codira::Error err;
    if (auto runtime =
            codira::make_runtime(get_codiralib_path("fibonacci/codira/target/mod.codiralib"), {}, &err)) {
        REQUIRE(err.is_ok());

        runtime->update(&err);
        if (err.is_error()) {
            FAIL(err.message().value());
        }
        REQUIRE(err.is_ok());
    } else {
        REQUIRE(err.is_error());
        FAIL(err.message().value());
    }
}

TEST_CASE("runtime can garbage collect", "[runtime]") {
    codira::Error err;
    if (auto runtime = codira::make_runtime(get_codiralib_path("codira-marshal/target/mod.codiralib"), {}, &err)) {
        REQUIRE(err.is_ok());
        {
            auto res = codira::invoke_fn<codira::StructRef>(*runtime, "new_bool", true, false);
            REQUIRE(res.is_ok());
            REQUIRE(!runtime->gc_collect());
        }
        REQUIRE(runtime->gc_collect());
    } else {
        REQUIRE(err.is_error());
        FAIL(err.message().value());
    }
}
