#define LANGUAGE_NAME(X) __attribute__((language_name(#X)))

#ifndef LANGUAGE_ENUM_EXTRA
#  define LANGUAGE_ENUM_EXTRA
#endif

#ifndef LANGUAGE_ENUM
#  define LANGUAGE_ENUM(_name)    \
  enum _name _name;            \
  enum __attribute__((enum_extensibility(open))) \
       __attribute__((external_source_symbol(language="Codira", \
                      defined_in="language_name", generated_declaration))) \
       LANGUAGE_ENUM_EXTRA _name
#endif

// Renaming global variables.
int SNFoo LANGUAGE_NAME(Bar);

// Renaming tags and fields.
struct LANGUAGE_NAME(SomeStruct) SNSomeStruct {
  double X LANGUAGE_NAME(x);
};

// Renaming C functions
struct SNSomeStruct SNMakeSomeStruct(double X, double Y) LANGUAGE_NAME(makeSomeStruct(x:y:));

struct SNSomeStruct SNMakeSomeStructForX(double X) LANGUAGE_NAME(makeSomeStruct(x:));

// Renaming typedefs.
typedef int SNIntegerType LANGUAGE_NAME(MyInt);

// Renaming enumerations.
LANGUAGE_ENUM(SNColorChoice) {
  SNColorRed LANGUAGE_NAME(Rouge),
  SNColorGreen,
  SNColorBlue
};

// language_private attribute
void SNTransposeInPlace(struct SNSomeStruct *value) __attribute__((language_private));

typedef struct {
  double x, y, z;
} SNPoint LANGUAGE_NAME(Point);

// Importing a value into a member.
extern double DefaultXValue __attribute__((language_name("SomeStruct.defaultX")));

// Importing a function as a method.
struct SNSomeStruct SNAdding(struct SNSomeStruct *value, double x) LANGUAGE_NAME(SomeStruct.adding(self:x:));

// Importing a function as an initializer.
struct SNSomeStruct SNCreate(double x) LANGUAGE_NAME(SomeStruct.init(theX:));

// Importing a function as a static property getter.
struct SNSomeStruct SNSomeStructGetDefault(void) LANGUAGE_NAME(getter:SomeStruct.defaultValue());

// Importing a function as a static property setter.
void SNSomeStructSetDefault(struct SNSomeStruct value) LANGUAGE_NAME(setter:SomeStruct.defaultValue(_:));

// Importing a function as an instance property getter.
double SNSomeStructGetFoo(struct SNSomeStruct s) LANGUAGE_NAME(getter:SomeStruct.foo(self:));

// Importing a function as an instance property setter.
void SNSomeStructSetFoo(struct SNSomeStruct s, double value) LANGUAGE_NAME(setter:SomeStruct.foo(self:_:));
