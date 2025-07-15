@import Foundation;

@interface ClassWithHandlerMethod
-(void)methodWithHandler:(NSString *)operation completionHandler:(void (^)(NSInteger))handler;

-(void)mainActorMethod __attribute__((__language_attr__("@UIActor")));
@end
