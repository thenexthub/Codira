struct PureClangType {
  int x;
  int y;
};

#ifndef LANGUAGE_CLASS_EXTRA
#  define LANGUAGE_CLASS_EXTRA
#endif
#ifndef LANGUAGE_PROTOCOL_EXTRA
#  define LANGUAGE_PROTOCOL_EXTRA
#endif

#ifndef LANGUAGE_CLASS
#  define LANGUAGE_CLASS(LANGUAGE_NAME) LANGUAGE_CLASS_EXTRA
#endif

#ifndef LANGUAGE_CLASS_NAMED
#  define LANGUAGE_CLASS_NAMED(LANGUAGE_NAME) \
    __attribute__((language_name(LANGUAGE_NAME))) LANGUAGE_CLASS_EXTRA
#endif

#ifndef LANGUAGE_PROTOCOL_NAMED
#  define LANGUAGE_PROTOCOL_NAMED(LANGUAGE_NAME) \
    __attribute__((language_name(LANGUAGE_NAME))) LANGUAGE_PROTOCOL_EXTRA
#endif

#pragma clang attribute push( \
  __attribute__((external_source_symbol(language="Codira", \
                 defined_in="Mixed",generated_declaration))), \
  apply_to=any(function,enum,objc_interface,objc_category,objc_protocol))

LANGUAGE_CLASS("CodiraClass")
__attribute__((objc_root_class))
@interface CodiraClass
@end

LANGUAGE_PROTOCOL_NAMED("CustomNameType")
@protocol CodiraProtoWithCustomName
@end

LANGUAGE_CLASS_NAMED("CustomNameClass")
__attribute__((objc_root_class))
@interface CodiraClassWithCustomName<CodiraProtoWithCustomName>
@end

id<CodiraProtoWithCustomName> _Nonnull
convertToProto(CodiraClassWithCustomName *_Nonnull obj);

LANGUAGE_CLASS("BOGUS")
@interface BogusClass
@end

# pragma clang attribute pop

@interface CodiraClass (Category)
- (void)categoryMethod:(struct PureClangType)arg;
@end
