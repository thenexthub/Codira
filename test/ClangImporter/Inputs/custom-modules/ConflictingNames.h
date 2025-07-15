@import Foundation;

@protocol OMWWiggle
-(void)conflicting1 __attribute__((language_name("wiggle1()"))); // expected-error {{'language_name' and 'language_name' attributes are not compatible}}
@end

@protocol OMWWaggle
-(void)conflicting1 __attribute__((language_name("waggle1()"))); // expected-note {{conflicting attribute is here}}
@end

@interface OMWSuper : NSObject <OMWWiggle>
@end

@interface OMWSub : OMWSuper <OMWWaggle>
-(void)conflicting1;
@end
