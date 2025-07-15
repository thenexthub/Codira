#import <Foundation/Foundation.h>

NS_LANGUAGE_UNAVAILABLE("unavailable")
@interface UnavailableInCodira : NSObject
@end

@interface HasAvailableInit : NSObject
- (nonnull instancetype)initWithUnavailable:(nonnull UnavailableInCodira *)unavailable;
@end

@interface HasUnavailableInit : NSObject
- (nonnull instancetype)initWithUnavailable:(nonnull UnavailableInCodira *)unavailable NS_LANGUAGE_UNAVAILABLE("unavailable");
@end

