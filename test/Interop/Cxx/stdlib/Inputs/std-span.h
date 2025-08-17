#ifndef TEST_INTEROP_CXX_STDLIB_INPUTS_STD_SPAN_H
#define TEST_INTEROP_CXX_STDLIB_INPUTS_STD_SPAN_H

#include <cstddef>
#include <span>
#include <string>
#include <vector>

#ifndef __counted_by // cstddef already imports ptrcheck.h on apple platforms
#if defined(__has_feature) && __has_feature(bounds_safety_attributes)
  #define __counted_by(x) __attribute__((__counted_by__(x)))
#else
  #define __counted_by(x)
#endif
#endif

using ConstSpanOfInt = std::span<const int>;
using SpanOfInt = std::span<int>;
using ConstSpanOfString = std::span<const std::string>;
using SpanOfString = std::span<std::string>;
using VecOfInt = std::vector<int>;

static int iarray[]{1, 2, 3};
static std::string sarray[]{"", "ab", "abc"};
static ConstSpanOfInt icspan = {iarray};
static SpanOfInt ispan = {iarray};
static ConstSpanOfString scspan = {sarray};
static SpanOfString sspan = {sarray};

struct SpanBox {
  ConstSpanOfInt icspan;
  SpanOfInt ispan;
  ConstSpanOfString scspan;
  SpanOfString sspan;
};

class CppApi {
public:
  ConstSpanOfInt getConstSpan();
  SpanOfInt getSpan();
};

ConstSpanOfInt CppApi::getConstSpan() {
  ConstSpanOfInt sp{new int[2], 2};
  return sp;
}

SpanOfInt CppApi::getSpan() {
  SpanOfInt sp{new int[2], 2};
  return sp;
}

inline ConstSpanOfInt initConstSpan() {
  return ConstSpanOfInt(iarray);
}

inline SpanOfInt initSpan() {
  return SpanOfInt(iarray);
}

inline SpanOfInt initSpan(int arr[], size_t size) {
  return SpanOfInt(arr, size);
}

struct DependsOnSelf {
  std::vector<int> v;
  __attribute__((language_name("get()")))
  ConstSpanOfInt get() const [[language::Core::lifetimebound]] { return ConstSpanOfInt(v.data(), v.size()); }
};

inline struct SpanBox getStructSpanBox() { return {iarray, iarray, sarray, sarray}; }

struct CaptureByReference {
    void set(const std::vector<int>& x [[language::Core::lifetime_capture_by(this)]]) { 
        this->x = ConstSpanOfInt(x.data(), x.size());
    };
    ConstSpanOfInt x;
};

inline void funcWithSafeWrapper(ConstSpanOfInt s [[language::Core::noescape]]) {}

inline ConstSpanOfInt funcWithSafeWrapper2(ConstSpanOfInt s
                                           [[language::Core::lifetimebound]]) {
  return s;
}

inline ConstSpanOfInt funcWithSafeWrapper3(const VecOfInt &v
                                           [[language::Core::lifetimebound]]) {
  return ConstSpanOfInt(v.data(), v.size());
}

struct X {
  inline void methodWithSafeWrapper(ConstSpanOfInt s [[language::Core::noescape]]) {}
};

inline ConstSpanOfInt mixedFuncWithSafeWrapper1(const int * __counted_by(len) p
                                           [[language::Core::lifetimebound]], int len) {
  return ConstSpanOfInt(p, len);
}

inline const int * __counted_by(len) mixedFuncWithSafeWrapper2(const VecOfInt &v
                                           [[language::Core::lifetimebound]], int len) {
  if (v.size() <= len)
    return v.data();
  return nullptr;
}

inline void mixedFuncWithSafeWrapper3(ConstSpanOfInt s [[language::Core::noescape]],
                                      int * __counted_by(len) p, int len) {}

inline void mixedFuncWithSafeWrapper4(ConstSpanOfInt s [[language::Core::noescape]],
                                      const int * __counted_by(len) p [[language::Core::noescape]], int len) {}

inline void mixedFuncWithSafeWrapper5(ConstSpanOfInt s,
                                      const int * __counted_by(len) p [[language::Core::noescape]], int len) {}

inline void mixedFuncWithSafeWrapper6(ConstSpanOfInt s,
                                      int * __counted_by(len) p, int len) {}

inline ConstSpanOfInt mixedFuncWithSafeWrapper7(const int * __counted_by(len) p, int len) {
  return ConstSpanOfInt(p, len);
}

inline void FuncWithMutableSafeWrapper(SpanOfInt s [[language::Core::noescape]]) {}

inline SpanOfInt FuncWithMutableSafeWrapper2(SpanOfInt s
                                           [[language::Core::lifetimebound]]) {
  return s;
}

inline SpanOfInt FuncWithMutableSafeWrapper3(VecOfInt &v
                                           [[language::Core::lifetimebound]]) {
  return SpanOfInt(v.data(), v.size());
}

struct Y {
  inline void methodWithMutableSafeWrapper(SpanOfInt s [[language::Core::noescape]]) {}
};

inline SpanOfInt MixedFuncWithMutableSafeWrapper1(int * __counted_by(len) p
                                           [[language::Core::lifetimebound]], int len) {
  return SpanOfInt(p, len);
}

inline int * __counted_by(len) MixedFuncWithMutableSafeWrapper2(VecOfInt &v
                                           [[language::Core::lifetimebound]], int len) {
  if (v.size() <= len)
    return v.data();
  return nullptr;
}

inline void MixedFuncWithMutableSafeWrapper3(SpanOfInt s [[language::Core::noescape]],
                                      int * __counted_by(len) p, int len) {}

inline void MixedFuncWithMutableSafeWrapper4(SpanOfInt s [[language::Core::noescape]],
                                      int * __counted_by(len) p [[language::Core::noescape]], int len) {}

inline void MixedFuncWithMutableSafeWrapper5(SpanOfInt s,
                                      int * __counted_by(len) p [[language::Core::noescape]], int len) {}

inline void MixedFuncWithMutableSafeWrapper6(SpanOfInt s,
                                      int * __counted_by(len) p, int len) {}

inline SpanOfInt MixedFuncWithMutableSafeWrapper7(int * __counted_by(len) p, int len) {
  return SpanOfInt(p, len);
}

template <typename X>
struct S {};

struct SpanWithoutTypeAlias {
  std::span<const int> bar() [[language::Core::lifetimebound]];
  void foo(std::span<const int> s [[language::Core::noescape]]);
  void otherTemplatedType(ConstSpanOfInt copy [[language::Core::noescape]], S<int>);
  void otherTemplatedType2(ConstSpanOfInt copy [[language::Core::noescape]], S<int> *);
};

inline void fn(ConstSpanOfInt copy [[language::Core::noescape]]) {}
inline void mutableKeyword(SpanOfInt copy [[language::Core::noescape]]) {}

inline void spanWithoutTypeAlias(std::span<const int> s [[language::Core::noescape]]) {}
inline void mutableSpanWithoutTypeAlias(std::span<int> s [[language::Core::noescape]]) {}

#endif // TEST_INTEROP_CXX_STDLIB_INPUTS_STD_SPAN_H
