
@import ObjectiveC;

@interface NSMyObject : NSObject

-(NSMyObject *)init;
-(void)print;

@property(retain) id lifetimeTracked;

@end

typedef NSMyObject *MyObject __attribute((language_newtype(struct))) __attribute((language_name("MyObject")));
