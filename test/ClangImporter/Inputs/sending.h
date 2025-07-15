
// Make sure that we have set the -D flag appropriately.
#ifdef __LANGUAGE_ATTR_SUPPORTS_SENDING
#if !__LANGUAGE_ATTR_SUPPORTS_SENDING
#error "Compiler should have set __LANGUAGE_ATTR_SUPPORTS_SENDING to 1"
#endif
#else
#error "Compiler should have defined __LANGUAGE_ATTR_SUPPORTS_SENDING"
#endif

#define LANGUAGE_SENDING __attribute__((language_attr("sending")))

#pragma clang assume_nonnull begin

#ifdef __OBJC__

@import Foundation;

@interface MyType : NSObject
- (NSObject *)getSendingResult LANGUAGE_SENDING;
- (NSObject *)getSendingResultWithArgument:(NSObject *)arg LANGUAGE_SENDING;
- (NSObject *)getResultWithSendingArgument:(NSObject *)LANGUAGE_SENDING arg;
@end

LANGUAGE_SENDING
@interface DoesntMakeSense : NSObject
@end

NSObject *returnNSObjectFromGlobalFunction(NSObject *other);
NSObject *sendNSObjectFromGlobalFunction(NSObject *other) LANGUAGE_SENDING;
void sendNSObjectToGlobalFunction(NSObject *arg LANGUAGE_SENDING);

#endif

typedef struct {
  int state;
} NonSendableCStruct;

NonSendableCStruct
returnUserDefinedFromGlobalFunction(NonSendableCStruct other);
NonSendableCStruct
sendUserDefinedFromGlobalFunction(NonSendableCStruct other) LANGUAGE_SENDING;
void sendUserDefinedIntoGlobalFunction(
    NonSendableCStruct arg LANGUAGE_SENDING);

void sendingWithCompletionHandler(void (^completion)(LANGUAGE_SENDING NonSendableCStruct arg));
LANGUAGE_SENDING NonSendableCStruct sendingWithLazyReturn(LANGUAGE_SENDING NonSendableCStruct (^makeLazily)(void));

#pragma clang assume_nonnull end
