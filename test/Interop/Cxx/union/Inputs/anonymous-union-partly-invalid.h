@class C;
@interface C
{}
@end

struct __attribute__((language_attr("import_unsafe"))) S {
  union {
    C *t;
    char c;
  };
  S(const S &s) {}
  ~S() { }
  int f() { return 42; }
};

S *getSPtr();
