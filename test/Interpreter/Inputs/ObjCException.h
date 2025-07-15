#import <Foundation/Foundation.h>

@interface ExceptionCatcher : NSObject
- (NSException* _Nullable)tryBlock:(__attribute__((noescape)) void(^ _Nonnull)(void))unsafeBlock NS_LANGUAGE_NAME(tryBlock(_:));
@end
