void doSomethingInHead(int arg);
#define NS_REFINED_FOR_LANGUAGE __attribute__((language_private))
@interface BaseInHead
- (void)doIt:(int)arg;
- (void)otherThing:(int)arg NS_REFINED_FOR_LANGUAGE;
@end

/// Awesome name.
/// ��������������
/// Awesome name.
@interface SameName
@end

// random comment.

@protocol SameName
@end
