@import Foundation;
@import CoreFoundation;

typedef NSString *_Nonnull SNTErrorDomain __attribute((language_newtype(struct)))
__attribute((language_name("ErrorDomain")));

extern void SNTErrorDomainProcess(SNTErrorDomain d)
    __attribute((language_name("ErrorDomain.process(self:)")));

typedef struct {} Food;

extern const SNTErrorDomain SNTErrOne
    __attribute((language_name("ErrorDomain.one")));
extern const SNTErrorDomain SNTErrTwo;
extern const SNTErrorDomain SNTErrorDomainThree;
extern const SNTErrorDomain SNTFourErrorDomain;
extern const SNTErrorDomain SNTFive
    __attribute((language_name("stillAMember")));
extern const SNTErrorDomain SNTElsewhere
    __attribute((language_name("Food.err")));

typedef NSString *_Nullable SNTClosedEnum __attribute((language_newtype(enum)))
__attribute((language_name("ClosedEnum")));

extern const SNTClosedEnum SNTFirstClosedEntryEnum;
extern const SNTClosedEnum SNTSecondEntry;
extern const SNTClosedEnum SNTClosedEnumThirdEntry;

typedef NSString * IUONewtype __attribute((language_newtype(struct)));

typedef float MyFloat __attribute((language_newtype(struct)));
extern const MyFloat globalFloat;
extern const MyFloat kPI;
extern const MyFloat kVersion;

typedef int MyInt __attribute((language_newtype(struct)));
extern const MyInt kMyIntZero;
extern const MyInt kMyIntOne;
extern const int kRawInt;
extern void takesMyInt(MyInt);

typedef NSString * NSURLResourceKey __attribute((language_newtype(struct)));
extern NSURLResourceKey const NSURLIsRegularFileKey;
extern NSURLResourceKey const NSURLIsDirectoryKey;
extern NSURLResourceKey const NSURLLocalizedNameKey;

// Special case: Notifications
extern const NSString *FooNotification;
extern const NSString *kBarNotification;
extern const NSString *NSWibbleNotification;

// But not just 'Notification'
extern const NSString *kNotification;
extern const NSString *Notification;

// Nor when explicitly language_name-ed
extern const NSString *kSNNotification
    __attribute((language_name("languageNamedNotification")));

// Test CFStringRef
typedef CFStringRef CFNewType __attribute((language_newtype(struct)));

// CF audited
_Pragma("clang arc_cf_code_audited begin")
extern const CFNewType MyCFNewTypeValue;
extern CFNewType FooAudited(void);
_Pragma("clang arc_cf_code_audited end")
extern const CFNewType MyCFNewTypeValueUnauditedButConst;

// un-audited CFStringRef
extern CFNewType MyCFNewTypeValueUnaudited;
extern CFNewType FooUnaudited(void);

// Tests to show identical calling convention / binary representation for
// new_type and non-new_type
typedef CFStringRef MyABINewType __attribute((language_newtype(struct)));
typedef CFStringRef MyABIOldType;
_Pragma("clang arc_cf_code_audited begin")
extern const MyABINewType kMyABINewTypeGlobal;
extern const MyABIOldType kMyABIOldTypeGlobal;
extern MyABINewType getMyABINewType(void);
extern MyABIOldType getMyABIOldType(void);
extern void takeMyABINewType(MyABINewType);
extern void takeMyABIOldType(MyABIOldType);

extern void takeMyABINewTypeNonNull(_Nonnull MyABINewType);
extern void takeMyABIOldTypeNonNull(_Nonnull MyABIOldType);
_Pragma("clang arc_cf_code_audited end")

typedef NSString *MyABINewTypeNS __attribute((language_newtype(struct)));
typedef NSString *MyABIOldTypeNS;
extern MyABINewTypeNS getMyABINewTypeNS(void);
extern MyABIOldTypeNS getMyABIOldTypeNS(void);
extern void takeMyABINewTypeNonNullNS(_Nonnull MyABINewTypeNS);
extern void takeMyABIOldTypeNonNullNS(_Nonnull MyABIOldTypeNS);

// Nested types
typedef struct {int i;} NSSomeContext;

typedef NSString *NSSomeContextName __attribute((language_newtype(struct)))
__attribute((language_name("NSSomeContext.Name")));

extern const NSSomeContextName NSMyContextName;


typedef NSError *ErrorNewType __attribute((language_newtype(struct)));

void testErrorDictionary(NSDictionary<NSError *, NSString *> * _Nonnull);
void testErrorDictionaryNewtype(NSDictionary<ErrorNewType, NSString *> * _Nonnull);

typedef NSUInteger NSUIntegerNewType __attribute((language_newtype(struct)));
extern const NSUIntegerNewType NSUIntegerNewTypeConstant;
