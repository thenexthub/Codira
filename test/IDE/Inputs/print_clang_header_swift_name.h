@import Foundation;

#ifndef LANGUAGE_ENUM_EXTRA
# define LANGUAGE_ENUM_EXTRA
#endif

#define LANGUAGE_COMPILE_NAME(X) __attribute__((language_name(X)))

#define LANGUAGE_ENUM(_type, _name) enum _name : _type _name; enum __attribute__((enum_extensibility(open))) LANGUAGE_ENUM_EXTRA _name : _type

#pragma clang attribute push( \
  __attribute__((external_source_symbol(language="Codira", \
                 defined_in="Mixed",generated_declaration))), \
  apply_to=any(function,enum,objc_interface,objc_category,objc_protocol))

typedef LANGUAGE_ENUM(NSInteger, Normal) {
    NormalOne = 0,
    NormalTwo,
    NormalThree
};

#define LANGUAGE_ENUM_NAMED(_type, _name, _languageName) enum _name : _type _name LANGUAGE_COMPILE_NAME(_languageName); enum LANGUAGE_COMPILE_NAME(_languageName) __attribute__((enum_extensibility(open))) LANGUAGE_ENUM_EXTRA _name : _type

typedef LANGUAGE_ENUM_NAMED(NSInteger, ObjCEnum, "CodiraEnum") {
    ObjCEnumOne = 0,
    ObjCEnumTwo,
    ObjCEnumThree
};

typedef LANGUAGE_ENUM_NAMED(NSInteger, ObjCEnumTwo, "CodiraEnumTwo") {
    // the following shouldn't have their prefixes stripped
    CodiraEnumTwoA,
    CodiraEnumTwoB,
    CodiraEnumTwoC
};

#pragma clang attribute pop
