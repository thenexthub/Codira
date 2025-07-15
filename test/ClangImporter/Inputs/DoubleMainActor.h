@import Foundation;

#pragma clang assume_nonnull begin

#define LANGUAGE_MAIN_ACTOR __attribute__((language_attr("@MainActor")))
#define LANGUAGE_UI_ACTOR __attribute__((language_attr("@UIActor")))

// NOTE: If you ever end up removing support for the "@UIActor" alias,
// just change both to be @MainActor and it won't change the purpose of
// this test.

LANGUAGE_UI_ACTOR LANGUAGE_MAIN_ACTOR @protocol DoubleMainActor
@required
- (NSString *)createSeaShanty:(NSInteger)number;
@end

#pragma clang assume_nonnull end
