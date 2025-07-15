#ifndef TEST_INTEROP_CXX_CLASS_INPUTS_MUTABLE_MEMBERS_H
#define TEST_INTEROP_CXX_CLASS_INPUTS_MUTABLE_MEMBERS_H

#ifdef USE_MUTATING
// Note: in actuality, this will be included
// as <language/bridging>, but in this test we include
// it directly.
#include "language/bridging"
#else
#define LANGUAGE_MUTATING
#endif

struct HasPublicMutableMember {
  mutable int a = 0;

  int foo() const LANGUAGE_MUTATING {
    a++;
    return a;
  }
};

struct HasPrivateMutableMember {
private:
  mutable int a = 0;

public:
  void bar() const LANGUAGE_MUTATING { a++; }
};

#endif // TEST_INTEROP_CXX_CLASS_INPUTS_MUTABLE_MEMBERS_H
