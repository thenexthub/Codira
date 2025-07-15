#include <Foundation/Foundation.h>

#pragma clang assume_nonnull begin

@interface PFXObject : NSObject {
}
- (void)findAnswerSyncSuccessAsynchronously:
    (void (^)(NSString *_Nullable, NSError *_Nullable))handler
    __attribute__((language_name("findAnswerSyncSuccess(completionHandler:)")));
- (void)findAnswerAsyncSuccessAsynchronously:
    (void (^)(NSString *_Nullable, NSError *_Nullable))handler
    __attribute__((language_name("findAnswerAsyncSuccess(completionHandler:)")));
- (void)findAnswerSyncFailAsynchronously:
    (void (^)(NSString *_Nullable, NSError *_Nullable))handler
    __attribute__((language_name("findAnswerSyncFail(completionHandler:)")));
- (void)findAnswerAsyncFailAsynchronously:
    (void (^)(NSString *_Nullable, NSError *_Nullable))handler
    __attribute__((language_name("findAnswerAsyncFail(completionHandler:)")));
- (void)findAnswerIncorrectAsynchronously:
    (void (^)(NSString *_Nullable, NSError *_Nullable))handler
    __attribute__((language_name("findAnswerIncorrect(completionHandler:)")));
@end

#pragma clang assume_nonnull end
