@protocol TestProto
@property (nonatomic, readonly, getter=getFoo) int foo __attribute__((language_name("languageFoo")));
@property (nonatomic, getter=getTheBar, setter=setTheBar:) int bar __attribute__((language_name("languageBar")));
@end
