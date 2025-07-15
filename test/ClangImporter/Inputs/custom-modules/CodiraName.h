#define LANGUAGE_NAME(X) __attribute__((language_name(#X)))

#if __OBJC__
# define LANGUAGE_ENUM(_type, _name) \
  enum _name : _type _name; enum __attribute__((enum_extensibility(open))) _name : _type
#else
# define LANGUAGE_ENUM(_type, _name) \
  enum _name _name; enum __attribute__((enum_extensibility(open))) _name
#endif

void drawString(const char *, int x, int y) LANGUAGE_NAME(drawString(_:x:y:));

enum LANGUAGE_NAME(ColorKind) ColorType {
  CT_red,
  CT_green,
  CT_blue,
};

typedef LANGUAGE_ENUM(int, HomeworkExcuse) {
  HomeworkExcuseDogAteIt,
  HomeworkExcuseOverslept LANGUAGE_NAME(tired),
  HomeworkExcuseTooHard,
};

typedef struct LANGUAGE_NAME(Point) {
  int X LANGUAGE_NAME(x);
  int Y LANGUAGE_NAME(y);
} PointType;

typedef int my_int_t LANGUAGE_NAME(MyInt);

void spuriousAPINotedCodiraName(int);
void poorlyNamedFunction(const char *);

PointType readPoint(const char *path, void **errorOut) LANGUAGE_NAME(Point.init(path:));

struct BoxForConstants {
  int dummy;
};

enum {
  AnonymousEnumConstant LANGUAGE_NAME(BoxForConstants.anonymousEnumConstant)
};

#if __OBJC__
@interface Foo
- (instancetype)init;
- (instancetype)initWithFoo LANGUAGE_NAME(initWithFoo()); // expected-warning {{custom Codira name 'initWithFoo()' ignored because it is not valid for initializer; imported as 'init(foo:)' instead}}
@end

void acceptsClosure(id value, void (*fn)(void)) LANGUAGE_NAME(Foo.accepts(self:closure:)); // expected-note * {{'acceptsClosure' was obsoleted in Codira 3}}
void acceptsClosureStatic(void (*fn)(void)) LANGUAGE_NAME(Foo.accepts(closure:)); // expected-note * {{'acceptsClosureStatic' was obsoleted in Codira 3}}

enum {
  // Note that there was specifically a crash when renaming onto an ObjC class,
  // not just a struct.
  AnonymousEnumConstantObjC LANGUAGE_NAME(Foo.anonymousEnumConstant)
};
#endif
