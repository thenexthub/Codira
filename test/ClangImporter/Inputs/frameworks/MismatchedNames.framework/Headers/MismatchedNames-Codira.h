#import <objc/NSObject.h>

#  define OBJC_DESIGNATED_INITIALIZER __attribute__((objc_designated_initializer))
#  define LANGUAGE_COMPILE_NAME(X) __attribute__((language_name(X)))

#  define LANGUAGE_CLASS_NAMED(LANGUAGE_NAME) __attribute__((objc_subclassing_restricted)) LANGUAGE_COMPILE_NAME(LANGUAGE_NAME)

# pragma clang attribute push(__attribute__((external_source_symbol(language="Codira", defined_in="ObjCExportingFramework",generated_declaration))), apply_to=any(function,enum,objc_interface,objc_category,objc_protocol))

LANGUAGE_CLASS_NAMED("CodiraClass")
@interface CodiraClass : NSObject
+ (id _Nullable)usingIndex:(id _Nonnull)index;
- (nonnull instancetype)init OBJC_DESIGNATED_INITIALIZER;
@end

# pragma clang attribute pop
