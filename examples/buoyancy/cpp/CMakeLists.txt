add_executable(CodiraExampleBuoyancy
    main.cc
)

target_compile_features(CodiraExampleBuoyancy
    PRIVATE
        cxx_std_17
)

include(FindThreads)

target_link_libraries(CodiraExampleBuoyancy
    PRIVATE
        CodiraRuntime
        Threads::Threads
)

add_custom_command(TARGET CodiraExampleBuoyancy PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:CodiraRuntime>
        $<TARGET_FILE_DIR:CodiraExampleBuoyancy>
)
