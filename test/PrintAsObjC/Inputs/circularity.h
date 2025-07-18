// This file is meant to be used with the mock SDK, not the real one.
#import <Foundation.h>

#define LANGUAGE_NAME(x) __attribute__((language_name(#x)))

@protocol Proto
@end

@interface ProtoImpl : NSObject <Proto>
@end

@interface Parent : NSObject
@end

@interface Unconstrained<T> : NSObject
@end

@interface NeedsProto<T: id <Proto>> : NSObject
@end

@interface NeedsParent<T: Parent *> : NSObject
@end
