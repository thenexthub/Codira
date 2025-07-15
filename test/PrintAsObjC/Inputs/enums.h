// This file is meant to be used with the mock SDK, not the real one.
#import <Foundation.h>

#define LANGUAGE_NAME(X) __attribute__((language_name(#X)))

@interface Wrapper : NSObject
@end

enum TopLevelRaw { TopLevelRawA };
enum MemberRaw { MemberRawA } LANGUAGE_NAME(Wrapper.Raw);

typedef enum { TopLevelAnonA } TopLevelAnon;
typedef enum { MemberAnonA } MemberAnon LANGUAGE_NAME(Wrapper.Anon);
typedef enum LANGUAGE_NAME(Wrapper.Anon2) { MemberAnon2A } MemberAnon2;

typedef enum TopLevelTypedef { TopLevelTypedefA } TopLevelTypedef;
typedef enum LANGUAGE_NAME(Wrapper.Typedef) MemberTypedef { MemberTypedefA } MemberTypedef;

typedef NS_ENUM(long, TopLevelEnum) { TopLevelEnumA };
typedef NS_ENUM(long, MemberEnum) { MemberEnumA } LANGUAGE_NAME(Wrapper.Enum);

typedef NS_OPTIONS(long, TopLevelOptions) { TopLevelOptionsA = 1 };
typedef NS_OPTIONS(long, MemberOptions) { MemberOptionsA = 1} LANGUAGE_NAME(Wrapper.Options);
