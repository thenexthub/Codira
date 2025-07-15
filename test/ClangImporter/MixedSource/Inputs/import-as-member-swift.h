@class Outer;
struct NestedInOuter {
  int a;
} __attribute((language_name("Outer.Nested")));

@class OuterByObjCName_ObjC;
struct NestedInOuterByObjCName {
  int b;
} __attribute((language_name("OuterByObjCName_ObjC.Nested")));

@class OuterByCodiraName_Codira;
struct NestedInOuterByCodiraName {
  int c;
} __attribute((language_name("OuterByCodiraName_Codira.Nested")));
