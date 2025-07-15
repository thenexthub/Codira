@import Foundation;

@interface AsyncImports : NSObject

-(void)methodWithCompletion:(void (^)(void))completionHandler;

-(void)propWithCompletion:(void (^)(BOOL))completionHandler
  __attribute__((language_async_name("getter:asyncProp()")));

@end
