@interface NSObject
- (void) init;
@end;

@interface Horse<T> : NSObject
@end

@interface Barn : NSObject
@end

typedef int Hay __attribute__((language_name("Horse.Hay")));
