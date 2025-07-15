#import <Foundation/Foundation.h>

@interface A: NSObject
- (id <NSObject>)foo __attribute__((language_attr("@Sendable")));
@end
