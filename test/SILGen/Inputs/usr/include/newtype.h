@import Foundation;

typedef NSString *_Nonnull SNTErrorDomain __attribute((language_newtype(struct)))
__attribute((language_name("ErrorDomain")));
extern const SNTErrorDomain SNTErrTwo;
extern const SNTErrorDomain SNTErrorDomainThree;
extern const SNTErrorDomain SNTFourErrorDomain;

typedef NSInteger MyInt __attribute((language_newtype(struct)));

typedef NSString *MyString __attribute__((language_newtype(struct)));
