//===--- CacheTest.cpp ----------------------------------------------------===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//

#include "language/Basic/Cache.h"
#include "gtest/gtest.h"

#if defined(__APPLE__)
#define USES_LIBCACHE 1
#else
#define USES_LIBCACHE 0
#endif

namespace {
struct Counter {
  mutable int enter = 0;
  mutable int exit = 0;
};
}

namespace language {
namespace sys {
template <>
struct CacheValueInfo<Counter>{
  static void *enterCache(const Counter &value) {
    return const_cast<Counter *>(&value);
  }
  static void retain(void *ptr) {
    static_cast<Counter*>(ptr)->enter+= 1;
  }
  static void release(void *ptr) {
    static_cast<Counter*>(ptr)->exit += 1;
  }
  static const Counter &getFromCache(void *ptr) {
    return *static_cast<Counter *>(ptr);
  }
  static size_t getCost(const Counter &value) {
    return 0;
  }
};
}
}

namespace {
struct KeyCounter {
  int key = 0;
  mutable int enter = 0;
  mutable int exit = 0;
};
}

namespace language {
namespace sys {
template <>
struct CacheKeyInfo<KeyCounter> {
  static uintptr_t getHashValue(const KeyCounter &value) {
    return toolchain::DenseMapInfo<int>::getHashValue(value.key);
  }
  static bool isEqual(void *lhs, void *rhs) {
    return static_cast<KeyCounter *>(lhs)->key ==
        static_cast<KeyCounter *>(rhs)->key;
  }
  static void *enterCache(const KeyCounter &value) {
    value.enter += 1;
    return const_cast<KeyCounter *>(&value);
  }
  static void exitCache(void *value) {
    static_cast<KeyCounter *>(value)->exit += 1;
  }
  static const void *getLookupKey(const KeyCounter *value) {
    return value;
  }
  static const KeyCounter &getFromCache(void *value) {
    return *static_cast<KeyCounter *>(value);
  }
};
}
}


namespace {
struct RefCntToken : toolchain::RefCountedBase<RefCntToken> {
  bool &freed;
  RefCntToken(bool &freed) : freed(freed) {}
  ~RefCntToken() { freed = true; }
};
}

TEST(Cache, sameKey) {
  Counter c1, c2;
  language::sys::Cache<const char *, Counter> cache(__func__);
  cache.set("a", c1);
  EXPECT_EQ(1, c1.enter);
  EXPECT_EQ(0, c1.exit);

  cache.set("a", c2);
  EXPECT_EQ(1, c1.enter);
  EXPECT_EQ(1, c1.exit);
  EXPECT_EQ(1, c2.enter);
  EXPECT_EQ(0, c2.exit);
}

TEST(Cache, sameValue) {
  Counter c;
  language::sys::Cache<const char *, Counter> cache(__func__);
  cache.set("a", c);
  EXPECT_EQ(1, c.enter);
  EXPECT_EQ(0, c.exit);

  cache.set("b", c);
#if USES_LIBCACHE
  EXPECT_EQ(1, c.enter); // value is shared.
#else
  EXPECT_EQ(2, c.enter);
#endif
  EXPECT_EQ(0, c.exit);

  cache.remove("a");
#if USES_LIBCACHE
  EXPECT_EQ(1, c.enter); // value is shared.
  EXPECT_EQ(0, c.exit);
#else
  EXPECT_EQ(2, c.enter);
  EXPECT_EQ(1, c.exit);
#endif

  cache.remove("b");
  EXPECT_EQ(c.enter, c.exit);
}

TEST(Cache, sameKeyValue) {
  Counter c;
  language::sys::Cache<const char *, Counter> cache(__func__);
  cache.set("a", c);
  EXPECT_EQ(1, c.enter);
  EXPECT_EQ(0, c.exit);

  cache.set("a", c);
  EXPECT_EQ(1, c.enter);
  EXPECT_EQ(0, c.exit);

  cache.remove("a");
  EXPECT_EQ(1, c.enter);
  EXPECT_EQ(1, c.exit);
}

TEST(Cache, sameKeyValueDestroysKey) {
  KeyCounter k1, k2;
  Counter c;
  language::sys::Cache<KeyCounter, Counter> cache(__func__);
  cache.set(k1, c);
  cache.set(k2, c);
  EXPECT_EQ(1, k1.enter);
  EXPECT_EQ(1, k1.exit);
  EXPECT_EQ(1, k2.enter);
  EXPECT_EQ(0, k2.exit);
}

TEST(Cache, sameKeyIntrusiveRefCountPter) {
  bool freed1 = false;
  bool freed2 = false;
  language::sys::Cache<const char *, toolchain::IntrusiveRefCntPtr<RefCntToken>> cache(__func__);
  {
    toolchain::IntrusiveRefCntPtr<RefCntToken> c1(new RefCntToken(freed1));
    toolchain::IntrusiveRefCntPtr<RefCntToken> c2(new RefCntToken(freed2));
    cache.set("a", c1);
    cache.set("a", c2);
  }
  EXPECT_TRUE(freed1);
  EXPECT_FALSE(freed2);
  cache.remove("a");
  EXPECT_TRUE(freed2);
}

TEST(Cache, sameValueIntrusiveRefCountPter) {
  bool freed = false;
  language::sys::Cache<const char *, toolchain::IntrusiveRefCntPtr<RefCntToken>> cache(__func__);
  {
    toolchain::IntrusiveRefCntPtr<RefCntToken> c(new RefCntToken(freed));
    cache.set("a", c);
    EXPECT_FALSE(freed);

    cache.set("b", c);
    EXPECT_FALSE(freed);

    cache.remove("a");
    EXPECT_FALSE(freed);

    cache.remove("b");
    EXPECT_FALSE(freed);
  }
  EXPECT_TRUE(freed);
}

TEST(Cache, sameKeyValueIntrusiveRefCountPter) {
  bool freed = false;
  language::sys::Cache<const char *, toolchain::IntrusiveRefCntPtr<RefCntToken>> cache(__func__);
  {
    toolchain::IntrusiveRefCntPtr<RefCntToken> c(new RefCntToken(freed));
    cache.set("a", c);
    EXPECT_FALSE(freed);
    cache.set("a", c);
    EXPECT_FALSE(freed);
  }
  EXPECT_FALSE(freed);
  cache.remove("a");
  EXPECT_TRUE(freed);
}

TEST(Cache, copyValue) {
  struct S {
    int ident, copy;
    S(int ident) : ident(ident), copy(0) {}
    S(const S &other) : ident(other.ident), copy(other.copy+1) {}
    S(S &&other) : ident(other.ident), copy(other.copy) {}
  };
  language::sys::Cache<const char *, struct S> cache(__func__);
  S s{0};
  EXPECT_EQ(0, s.ident);
  EXPECT_EQ(0, s.copy);
  cache.set("a", s);
  EXPECT_EQ(0, cache.get("a")->ident);
  EXPECT_EQ(2, cache.get("a")->copy); // return by value causes 2nd copy
  cache.set("b", *cache.get("a"));
  EXPECT_EQ(0, cache.get("b")->ident);
  EXPECT_EQ(4, cache.get("b")->copy); // return by value causes 2nd copy
}
