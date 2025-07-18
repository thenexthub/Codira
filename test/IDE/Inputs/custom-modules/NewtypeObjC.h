@import Foundation;

typedef NSString *_Nonnull SNTClosedEnum __attribute((language_newtype(enum)))
__attribute((language_name("ClosedEnum")));

extern const SNTClosedEnum SNTFirstClosedEntryEnum;
extern const SNTClosedEnum SNTSecondEntry;
extern const SNTClosedEnum SNTClosedEnumThirdEntry;

@interface GenericClassA<T> : NSObject
@end

@interface UsesGenericClassA : NSObject
-(void)takeEnumValues:(nonnull GenericClassA<SNTClosedEnum> *)values;
-(void)takeEnumValuesArray:(nonnull NSArray<SNTClosedEnum> *)values;
@end
