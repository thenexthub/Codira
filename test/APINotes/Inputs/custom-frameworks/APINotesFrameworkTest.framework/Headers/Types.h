#pragma clang assume_nonnull begin

struct __attribute__((language_name("VeryImportantCStruct"))) SomeCStruct {
    int field;
};

typedef int __attribute__((language_name("VeryImportantCAlias"))) SomeCAlias;

struct NormallyUnchangedWrapper {
  int value;
};
struct NormallyChangedOriginalWrapper{
  int value;
} __attribute__((language_name("NormallyChangedWrapper")));

#pragma clang assume_nonnull end
