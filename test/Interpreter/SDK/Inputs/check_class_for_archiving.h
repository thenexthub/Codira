#import <Foundation/Foundation.h>

@interface NSKeyedUnarchiver (CodiraAdditions)
+ (int)_language_checkClassAndWarnForKeyedArchiving:(Class)cls operation:(int)operation
    NS_LANGUAGE_NAME(_language_checkClassAndWarnForKeyedArchiving(_:operation:));
@end

