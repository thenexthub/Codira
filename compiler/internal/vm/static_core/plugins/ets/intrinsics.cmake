set(ETS_RUNTIME_YAML ${PANDA_ETS_PLUGIN_SOURCE}/runtime/ets_libbase_runtime.yaml)

if (PANDA_ETS_INTEROP_JS)
    list(APPEND ETS_RUNTIME_YAML ${PANDA_ETS_PLUGIN_SOURCE}/runtime/interop_js/intrinsics/std_js_jsruntime.yaml)
endif()

if (PANDA_WITH_COMPILER)
    list(APPEND ETS_RUNTIME_YAML ${PANDA_ETS_PLUGIN_SOURCE}/runtime/ets_compiler_intrinsics.yaml)
endif()