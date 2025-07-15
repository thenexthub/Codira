add_library(languageConcurrencyInternalShims INTERFACE)
target_include_directories(languageConcurrencyInternalShims INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

install(TARGETS languageConcurrencyInternalShims
  EXPORT CodiraCoreTargets
  COMPONENT CodiraCore_runtime)

