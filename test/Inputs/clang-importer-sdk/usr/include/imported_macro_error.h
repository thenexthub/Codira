#if __LANGUAGE_ATTR_SUPPORTS_MACROS
#define ERROR_MACRO __attribute__((language_attr("@macro_library.ExpandTypeError")))
#else
#define ERROR_MACRO
#endif

void foo() ERROR_MACRO;
