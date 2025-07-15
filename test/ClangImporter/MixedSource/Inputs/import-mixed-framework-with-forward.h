@class CodiraClass, CodiraClassWithCustomName;

@interface BridgingHeader
+ (void)takeForward:(CodiraClass *)class;
+ (void)takeRenamedForward:(CodiraClassWithCustomName *)class;
@end
