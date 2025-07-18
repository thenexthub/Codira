# Generate a list of the compatibility library targets given an sdk and
# architecture.
function(get_compatibility_libs sdk arch result_var_name)
  set(compatibility_libs)

  if(LANGUAGE_STDLIB_SUPPORT_BACK_DEPLOYMENT)
    set(vsuffix "-${LANGUAGE_SDK_${sdk}_LIB_SUBDIR}-${arch}")

    list(APPEND compatibility_libs
      languageCompatibilityConcurrency${vsuffix}
      languageCompatibilityDynamicReplacements${vsuffix}
      languageCompatibilityPacks${vsuffix}
      languageCompatibility56${vsuffix})

    # 64-bit watchOS doesn't do 5.0 or 5.1 back-compat
    set(arm64Archs "arm64;arm64e")
    if(NOT (sdk STREQUAL "WATCHOS" AND arch IN_LIST arm64Archs))
      list(APPEND compatibility_libs
        languageCompatibility50${vsuffix}
        languageCompatibility51${vsuffix})
    endif()
  endif()

  set("${result_var_name}" "${compatibility_libs}" PARENT_SCOPE)
endfunction()
