include(ExternalProject)

set(codira_executable_url "" CACHE FILEPATH "URL to an archive on the web containing the Codira executable. This is required for building the Codira tests.")
set(codira_examples_path "${CMAKE_SOURCE_DIR}/examples" CACHE FILEPATH "Location of Codira examples. This is required for building the Codira tests.")

Include(FetchContent)

FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.2.0 # or a later release
)

FetchContent_MakeAvailable(Catch2)

add_executable(codira_executable IMPORTED)
if (codira_binaries_path)
    set_target_properties(codira_executable PROPERTIES IMPORTED_LOCATION ${codira_binaries_path}/codira${CMAKE_EXECUTABLE_SUFFIX})
elseif (codira_executable_url)
    # Download the codira executable
    ExternalProject_Add(
        codira_executable_download
        PREFIX ${CMAKE_CURRENT_BINARY_DIR}/codira
        URL ${codira_executable_url}
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
    )

    ExternalProject_Get_Property(codira_executable_download source_dir)
    set_target_properties(codira_executable PROPERTIES IMPORTED_LOCATION ${source_dir}/codira${CMAKE_EXECUTABLE_SUFFIX})
    add_dependencies(codira_executable codira_executable_download)
else ()
    message(FATAL_ERROR "You must specify the `codira_binaries_path` or `codira_executable_url` to be able to use the Codira Runtime")
endif ()

if (codira_examples_path)
    if (NOT IS_DIRECTORY ${codira_binaries_path})
        message(FATAL_ERROR "The provided location of Codira examples is invalid.")
    endif ()
else ()
    message(FATAL_ERROR "You must specify the codira_examples_path to be able to test the Codira Runtime")
endif ()

# Sanitize the path, since we're forwarding this to C++
file(TO_CMAKE_PATH ${codira_examples_path} codira_examples_path)

# Find all codira files
set(codira_folder ${CMAKE_CURRENT_SOURCE_DIR}/..)
file(GLOB_RECURSE codira_project_manifests RELATIVE ${codira_examples_path} ${codira_examples_path}/codira.toml)
set(codira_tests)

foreach (codira_project_manifest ${codira_project_manifests})
    get_filename_component(codira_project_manifest_dir ${codira_project_manifest} DIRECTORY)
    set(abs_package_dir ${codira_examples_path}/${codira_project_manifest_dir})
    set(abs_toml_file ${abs_package_dir}/codira.toml)
    set(codira_executable_file mod.codiralib)
    set(codira_executable_path ${abs_package_dir}/target/${codira_executable_file})
    add_custom_command(
            OUTPUT ${codira_executable_path}
            COMMAND codira_executable build --manifest-path "${abs_toml_file}"
            WORKING_DIRECTORY ${abs_package_dir}
            MAIN_DEPENDENCY ${abs_toml_file})
    list(APPEND codira_tests ${codira_executable_path})
endforeach ()

add_custom_target(codira_test_codiralibs ALL
        DEPENDS ${codira_tests})

# Add the tests
add_executable(CodiraRuntimeTests
    marshal.cc
    runtime.cc
    extern.cc
)

target_include_directories(CodiraRuntimeTests PRIVATE ${codira_folder}/include)
target_link_libraries(CodiraRuntimeTests PRIVATE CodiraRuntime Catch2::Catch2WithMain)
add_dependencies(CodiraRuntimeTests codira_test_codiralibs)
target_compile_definitions(CodiraRuntimeTests PRIVATE -DCODIRA_TEST_DIR="${codira_examples_path}/")
set_property(TARGET CodiraRuntimeTests PROPERTY CXX_STANDARD 17)

add_custom_command(TARGET CodiraRuntimeTests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:CodiraRuntime>
        $<TARGET_FILE_DIR:CodiraRuntimeTests>)

include(CTest)
include(Catch)
catch_discover_tests(CodiraRuntimeTests)
