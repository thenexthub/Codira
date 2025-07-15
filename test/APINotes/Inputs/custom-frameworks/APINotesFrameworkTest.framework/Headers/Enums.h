#pragma clang assume_nonnull begin

enum {
  AnonymousEnumValue,
  AnonymousEnumRenamed __attribute__((language_name("AnonymousEnumRenamedCodiraUnversioned")))
};

enum UnknownEnum {
  UnknownEnumValue,
  UnknownEnumRenamed __attribute__((language_name("UnknownEnumRenamedCodiraUnversioned")))
};

enum __attribute__((enum_extensibility(open))) TrueEnum {
  TrueEnumValue,
  TrueEnumRenamed __attribute__((language_name("renamedCodiraUnversioned"))),
  TrueEnumAliasRenamed __attribute__((language_name("aliasRenamedCodiraUnversioned")))
};

enum __attribute__((flag_enum)) OptionyEnum {
  OptionyEnumValue = 1,
  OptionyEnumRenamed __attribute__((language_name("renamedCodiraUnversioned"))) = 2
};

#pragma clang assume_nonnull end
