#include <codira/codira.h>

#include <catch2/catch_test_macros.hpp>
#include <sstream>

/// Returns the absolute path to the codiralib with the specified name
inline std::string get_codiralib_path(std::string_view name) {
    std::stringstream ss;
    ss << CODIRA_TEST_DIR << name;
    return ss.str();
}

uint32_t internal_function(uint32_t a, uint32_t b) { return a + b; }
uint32_t some_function() { return 0; }

TEST_CASE("functions must be inserted into the runtime", "[extern]") {
    codira::RuntimeOptions options;

    codira::Error err;
    auto runtime =
        codira::make_runtime(get_codiralib_path("codira-extern/target/mod.codiralib"), options, &err);
    REQUIRE(!runtime);
    REQUIRE(err.is_error());
}

TEST_CASE("function must have correct signature", "[extern]") {
    codira::RuntimeOptions options;
    options.functions.emplace_back(codira::RuntimeFunction("extern_fn", some_function));

    codira::Error err;
    auto runtime =
        codira::make_runtime(get_codiralib_path("codira-extern/target/mod.codiralib"), options, &err);
    REQUIRE(!runtime);
    REQUIRE(err.is_error());
}

TEST_CASE("functions can be inserted into the runtime", "[extern]") {
    codira::RuntimeOptions options;
    options.functions.emplace_back(codira::RuntimeFunction("extern_fn", internal_function));

    codira::Error err;
    auto runtime =
        codira::make_runtime(get_codiralib_path("codira-extern/target/mod.codiralib"), options, &err);
    if (!runtime) {
        REQUIRE(err.is_error());
        FAIL(err.message().value());
    }

    REQUIRE(codira::invoke_fn<uint32_t, uint32_t, uint32_t>(*runtime, "main", 90, 2648).unwrap() ==
            90 + 2648);
}
