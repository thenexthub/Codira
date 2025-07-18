#ifdef __OBJC__
#pragma clang assume_nonnull begin

@class ForwardClass; // used by API notes

@protocol ProtoWithVersionedUnavailableMember
- (nullable id)requirement;
@end

@protocol ProtoWithManyRenames
- (instancetype)initWithBoolean:(_Bool)value __attribute__((language_name("init(finalBoolean:)")));
- (void)doImportantThings __attribute__((language_name("finalDoImportantThings()")));
@property (class, nullable) id importantClassProperty __attribute__((language_name("finalClassProperty")));
@end

#pragma clang assume_nonnull end
#endif // __OBJC__
