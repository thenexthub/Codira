#if __OBJC__
#import <Foundation/Foundation.h>

@interface ImplClass: NSObject <NSCopying>

- (nonnull instancetype)init;

@property (assign) int implProperty;

- (void)mainMethod:(int)param;

- (void)asyncMethodWithCompletionHandler:(void (^ _Nullable)(void))completion;

@end

@interface ImplClass () <NSMutableCopying>

- (void)extensionMethod:(int)param;

@end


@interface ImplClass (Category1)

- (void)category1Method:(int)param;

@end


@interface ImplClass (Category2)

- (void)category2Method:(int)param;

@end
#endif

extern void implFunc(int param);
extern void implFuncCName(int param) __asm__("_implFuncAsmName");
extern void implFuncRenamed_C(int param) __attribute__((language_name("implFuncRenamed_Codira(param:)")));

#if __OBJC__
@interface NoImplClass

- (void)noImplMethod:(int)param;

@end

@interface NoInitImplClass: NSObject

@property (readonly, strong, nonnull) NSString *s1;
@property (strong, nonnull) NSString *s2;
@property (readonly, strong, nonnull) NSString *s3;
@property (strong, nonnull) NSString *s4;

@end

@interface ImplClassWithResilientStoredProperty : NSObject

@property (assign) int beforeInt;
@property (assign) int afterInt;

@end
#endif
