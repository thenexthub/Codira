#if !BAD
typedef int MysteryTypedef;
#else
typedef _Bool MysteryTypedef;
#endif

struct ImportedType {
  int value;
};

typedef MysteryTypedef ImportedTypeAssoc __attribute__((language_name("ImportedType.Assoc")));

#if !BAD
typedef int WrappedInt __attribute__((language_wrapper(struct)));
typedef int UnwrappedInt;
#else
typedef int WrappedInt;
typedef int UnwrappedInt __attribute__((language_wrapper(struct)));
#endif
