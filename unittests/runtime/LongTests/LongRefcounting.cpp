//===--- LongRefcounting.cpp - Slow reference-counting tests for Codira ----===//
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

#include <functional>

#include "language/Runtime/HeapObject.h"
#include "language/Runtime/Metadata.h"
#include "language/Demangling/ManglingMacros.h"
#include "gtest/gtest.h"

#ifdef __APPLE__
// FIXME: is EXPECT_UNALLOCATED reliable enough for CI?
// EXPECT_ALLOCATED may fail falsely if the memory is re-allocated.
# include <malloc/malloc.h>
# define EXPECT_ALLOCATED(p)    EXPECT_NE(0u, malloc_size(p))
# define EXPECT_UNALLOCATED(p)  EXPECT_EQ(0u, malloc_size(p))
#else
// FIXME: heap assertion for other platforms?
# define EXPECT_ALLOCATED(p) (void)(p)
# define EXPECT_UNALLOCATED(p) (void)(p)
#endif

using namespace language;

struct TestObject : HeapObject {
  // *Addr = Value during deinit
  size_t *Addr;
  size_t Value;
  
  // Check lifecycle state DEINITING during deinit
  bool CheckLifecycle;

  // Weak variable to check in CheckLifecycle. nullptr skips the check.
  // On entry to deinit: must point to object
  // On exit from deinit: is destroyed
  WeakReference *WeakRef;
  
  // Callback invoked during the object's deinit.
  std::function<void()> DeinitCallback;

  TestObject(size_t *addr, size_t value)
    : Addr(addr), Value(value), CheckLifecycle(false), WeakRef(nullptr), DeinitCallback(nullptr)
  { }
};

static LANGUAGE_CC(language) void deinitTestObject(LANGUAGE_CONTEXT HeapObject *_object) {
  auto object = static_cast<TestObject*>(_object);
  assert(object->Addr && "object already deallocated");

  if (object->CheckLifecycle) {
    // RC ok
    language_retain(object);
    language_retain(object);
    language_release(object);
    language_release(object);
    // FIXME: RC underflow during deinit?

    // URC load crashes
    // URC increment OK
    // URC decrement OK
    ASSERT_DEATH(language_unownedCheck(object),
                 "Attempted to read an unowned reference");
    language_unownedRetain(object);
    language_unownedRetain(object);
    language_unownedRelease(object);
    language_unownedRelease(object);
    
    if (object->WeakRef) {
      // WRC load is nil
      // WRC increment is nil
      // WRC decrement OK

      // WRC -1
      auto weak_value = language_weakLoadStrong(object->WeakRef);
      EXPECT_EQ(nullptr, weak_value);
      language_weakDestroy(object->WeakRef);

      // WRC no change
      language_weakInit(object->WeakRef, object);
      weak_value = language_weakLoadStrong(object->WeakRef);
      EXPECT_EQ(nullptr, weak_value);

      // WRC no change
      language_weakInit(object->WeakRef, object);
      weak_value = language_weakLoadStrong(object->WeakRef);
      EXPECT_EQ(nullptr, weak_value);
    }
  }

  if (object->DeinitCallback != nullptr) {
    object->DeinitCallback();
  }

  *object->Addr = object->Value;
  object->Addr = nullptr;
  object->~TestObject();
  language_deallocObject(object, sizeof(TestObject), alignof(TestObject) - 1);
}

static const FullMetadata<ClassMetadata> TestClassObjectMetadata = {
  { { nullptr }, { &deinitTestObject }, { &VALUE_WITNESS_SYM(Bo) } },
  { { nullptr }, ClassFlags::UsesCodiraRefcounting, 0, 0, 0, 0, 0, 0 }
};

/// Create an object that, when deinited, stores the given value to
/// the given pointer.
static TestObject *allocTestObject(size_t *addr, size_t value) {
  auto buf = language_allocObject(&TestClassObjectMetadata,
                               sizeof(TestObject),
                               alignof(TestObject) - 1);

  return new (buf) TestObject(addr, value);
}


///////////////////////////////////////////////////
// Max strong retain count and overflow checking //
///////////////////////////////////////////////////

template <bool atomic>
static void retainALot(TestObject *object, size_t &deinited,
                       uint64_t count) {
  for (uint64_t i = 0; i < count; i++) {
    if (atomic) language_retain(object);
    else language_nonatomic_retain(object);
    EXPECT_EQ(0u, deinited);
  }
}

template <bool atomic>
static void releaseALot(TestObject *object, size_t &deinited,
                        uint64_t count) {
  for (uint64_t i = 0; i < count; i++) {
    if (atomic) language_release(object);
    else language_nonatomic_release(object);
    EXPECT_EQ(0u, deinited);
  }
}

// Maximum legal retain count.
// 32-2 bits of extra retain count, plus 1 for the implicit retain.
const uint64_t maxRC = 1ULL << (32 - 2);

TEST(LongRefcountingTest, retain_max) {
  // Don't generate millions of failures if something goes wrong.
  ::testing::FLAGS_gtest_break_on_failure = true;

  size_t deinited = 0;
  auto object = allocTestObject(&deinited, 1);

  // RC is 1.
  // Retain to maxRC, release back to 1, then release and verify deallocation.
  EXPECT_EQ(language_retainCount(object), 1u);
  retainALot<true>(object, deinited, maxRC - 1);
  EXPECT_EQ(language_retainCount(object), maxRC);
  releaseALot<true>(object, deinited, maxRC - 1);
  EXPECT_EQ(language_retainCount(object), 1u);
  EXPECT_EQ(0u, deinited);
  language_release(object);
  EXPECT_EQ(1u, deinited);
}

TEST(LongRefcountingTest, retain_overflow_DeathTest) {
  // Don't generate millions of failures if something goes wrong.
  ::testing::FLAGS_gtest_break_on_failure = true;

  size_t deinited = 0;
  auto object = allocTestObject(&deinited, 1);

  // RC is 1. Retain to maxRC, then retain again and verify overflow error.
  retainALot<true>(object, deinited, maxRC - 1);
  EXPECT_EQ(0u, deinited);
  ASSERT_DEATH(language_retain(object),
               "Object was retained too many times");
}

TEST(LongRefcountingTest, nonatomic_retain_max) {
  // Don't generate millions of failures if something goes wrong.
  ::testing::FLAGS_gtest_break_on_failure = true;

  size_t deinited = 0;
  auto object = allocTestObject(&deinited, 1);

  // RC is 1.
  // Retain to maxRC, release back to 1, then release and verify deallocation.
  EXPECT_EQ(language_retainCount(object), 1u);
  retainALot<false>(object, deinited, maxRC - 1);
  EXPECT_EQ(language_retainCount(object), maxRC);
  releaseALot<false>(object, deinited, maxRC - 1);
  EXPECT_EQ(language_retainCount(object), 1u);
  EXPECT_EQ(0u, deinited);
  language_nonatomic_release(object);
  EXPECT_EQ(1u, deinited);
}

TEST(LongRefcountingTest, nonatomic_retain_overflow_DeathTest) {
  // Don't generate millions of failures if something goes wrong.
  ::testing::FLAGS_gtest_break_on_failure = true;

  size_t deinited = 0;
  auto object = allocTestObject(&deinited, 1);

  // RC is 1. Retain to maxRC, then retain again and verify overflow error.
  retainALot<false>(object, deinited, maxRC - 1);
  EXPECT_EQ(0u, deinited);
  ASSERT_DEATH(language_nonatomic_retain(object),
               "Object was retained too many times");
}


///////////////////////////////////////////////////
// Max unowned retain count and overflow checking //
///////////////////////////////////////////////////

template <bool atomic>
static void unownedRetainALot(TestObject *object, uint64_t count) {
  for (uint64_t i = 0; i < count; i++) {
    if (atomic) language_unownedRetain(object);
    else language_nonatomic_unownedRetain(object);
    EXPECT_ALLOCATED(object);
  }
}

template <bool atomic>
static void unownedReleaseALot(TestObject *object, uint64_t count) {
  for (uint64_t i = 0; i < count; i++) {
    if (atomic) language_unownedRelease(object);
    else language_nonatomic_unownedRelease(object);
    EXPECT_ALLOCATED(object);
  }
}

// Maximum legal unowned retain count. 31 bits minus one with no implicit +1.
const uint64_t maxURC = (1ULL << (32 - 1)) - 2;

TEST(LongRefcountingTest, unowned_retain_max) {
  // Don't generate millions of failures if something goes wrong.
  ::testing::FLAGS_gtest_break_on_failure = true;

  size_t deinited = 0;
  auto object = allocTestObject(&deinited, 1);

  // RC is 1. URC is 1.
  // Unowned-retain to maxURC.
  // Release and verify deinited and not deallocated.
  // Unowned-release back to 1, then unowned-release and verify deallocated.
  EXPECT_EQ(language_retainCount(object), 1u);
  EXPECT_EQ(language_unownedRetainCount(object), 1u);
  unownedRetainALot<true>(object, maxURC - 1);
  EXPECT_EQ(language_unownedRetainCount(object), maxURC);

  EXPECT_EQ(0u, deinited);
  EXPECT_ALLOCATED(object);
  language_release(object);
  EXPECT_EQ(1u, deinited);
  EXPECT_ALLOCATED(object);
  // Strong release decremented unowned count by 1.
  EXPECT_EQ(language_unownedRetainCount(object), maxURC - 1);

  unownedReleaseALot<true>(object, maxURC - 2);
  EXPECT_EQ(language_unownedRetainCount(object), 1u);

  EXPECT_ALLOCATED(object);
  language_unownedRelease(object);
  EXPECT_UNALLOCATED(object);
}

TEST(LongRefcountingTest, unowned_retain_overflow_DeathTest) {
  // Don't generate millions of failures if something goes wrong.
  ::testing::FLAGS_gtest_break_on_failure = true;

  size_t deinited = 0;
  auto object = allocTestObject(&deinited, 1);

  // URC is 1. Retain to maxURC, then retain again and verify overflow error.
  unownedRetainALot<true>(object, maxURC - 1);
  EXPECT_EQ(0u, deinited);
  EXPECT_ALLOCATED(object);
  ASSERT_DEATH(language_unownedRetain(object),
               "Object's unowned reference was retained too many times");
}

TEST(LongRefcountingTest, nonatomic_unowned_retain_max) {
  // Don't generate millions of failures if something goes wrong.
  ::testing::FLAGS_gtest_break_on_failure = true;

  size_t deinited = 0;
  auto object = allocTestObject(&deinited, 1);

  // RC is 1. URC is 1.
  // Unowned-retain to maxURC.
  // Release and verify deinited and not deallocated.
  // Unowned-release back to 1, then unowned-release and verify deallocated.
  EXPECT_EQ(language_retainCount(object), 1u);
  EXPECT_EQ(language_unownedRetainCount(object), 1u);
  unownedRetainALot<false>(object, maxURC - 1);
  EXPECT_EQ(language_unownedRetainCount(object), maxURC);

  EXPECT_EQ(0u, deinited);
  EXPECT_ALLOCATED(object);
  language_release(object);
  EXPECT_EQ(1u, deinited);
  EXPECT_ALLOCATED(object);
  // Strong release decremented unowned count by 1.
  EXPECT_EQ(language_unownedRetainCount(object), maxURC - 1);

  unownedReleaseALot<false>(object, maxURC - 2);
  EXPECT_EQ(language_unownedRetainCount(object), 1u);

  EXPECT_ALLOCATED(object);
  language_unownedRelease(object);
  EXPECT_UNALLOCATED(object);
}

TEST(LongRefcountingTest, nonatomic_unowned_retain_overflow_DeathTest) {
  // Don't generate millions of failures if something goes wrong.
  ::testing::FLAGS_gtest_break_on_failure = true;

  size_t deinited = 0;
  auto object = allocTestObject(&deinited, 1);

  // URC is 1. Retain to maxURC, then retain again and verify overflow error.
  unownedRetainALot<false>(object, maxURC - 1);
  EXPECT_EQ(0u, deinited);
  EXPECT_ALLOCATED(object);
  ASSERT_DEATH(language_nonatomic_unownedRetain(object),
               "Object's unowned reference was retained too many times");
}


/////////////////////////////////////////////////
// Max weak retain count and overflow checking //
/////////////////////////////////////////////////

static HeapObjectSideTableEntry *weakRetainALot(TestObject *object, uint64_t count) {
  if (count == 0) return nullptr;
  
  auto side = object->refCounts.formWeakReference();
  for (uint64_t i = 1; i < count; i++) {
    side = side->incrementWeak();
    EXPECT_ALLOCATED(object);
  }
  return side;
}

template <bool atomic>
static void weakReleaseALot(HeapObjectSideTableEntry *side, uint64_t count) {
  for (uint64_t i = 0; i < count; i++) {
    if (atomic) side->decrementWeak();
    else side->decrementWeakNonAtomic();
  }
}

// Maximum legal weak retain count. 32 bits with no implicit +1.
const uint64_t maxWRC = (1ULL << 32) - 1;

TEST(LongRefcountingTest, weak_retain_max) {
  // Don't generate millions of failures if something goes wrong.
  ::testing::FLAGS_gtest_break_on_failure = true;

  size_t deinited = 0;
  auto object = allocTestObject(&deinited, 1);

  // RC is 1. WRC is 1.
  // Weak-retain to maxWRC.
  // Release and verify deallocated object and live side table.
  // Weak-release back to 1, then weak-release and verify deallocated.
  EXPECT_EQ(language_retainCount(object), 1u);
  EXPECT_EQ(object->refCounts.getWeakCount(), 1u);
  auto side = weakRetainALot(object, maxWRC - 1);
  EXPECT_EQ(side->getWeakCount(), maxWRC);
  
  EXPECT_EQ(0u, deinited);
  EXPECT_ALLOCATED(object);
  EXPECT_ALLOCATED(side);
  language_release(object);
  EXPECT_EQ(1u, deinited);
  EXPECT_UNALLOCATED(object);
  EXPECT_ALLOCATED(side);

  weakReleaseALot<true>(side, maxWRC - 2);
  EXPECT_EQ(side->getWeakCount(), 1u);

  EXPECT_ALLOCATED(side);
  side->decrementWeak();
  EXPECT_UNALLOCATED(side);
}

TEST(LongRefcountingTest, weak_retain_overflow_DeathTest) {
  // Don't generate millions of failures if something goes wrong.
  ::testing::FLAGS_gtest_break_on_failure = true;

  size_t deinited = 0;
  auto object = allocTestObject(&deinited, 1);

  // URC is 1. Retain to maxURC, then retain again and verify overflow error.
  weakRetainALot(object, maxWRC - 1);
  EXPECT_EQ(0u, deinited);
  EXPECT_ALLOCATED(object);
  ASSERT_DEATH(weakRetainALot(object, 1),
               "Object's weak reference was retained too many times");
}

TEST(LongRefcountingTest, nonatomic_weak_retain_max) {
  // Don't generate millions of failures if something goes wrong.
  ::testing::FLAGS_gtest_break_on_failure = true;

  size_t deinited = 0;
  auto object = allocTestObject(&deinited, 1);

  // RC is 1. WRC is 1.
  // Weak-retain to maxWRC.
  // Release and verify deallocated object and live side table.
  // Weak-release back to 1, then weak-release and verify deallocated.
  EXPECT_EQ(language_retainCount(object), 1u);
  EXPECT_EQ(object->refCounts.getWeakCount(), 1u);
  auto side = weakRetainALot(object, maxWRC - 1);
  EXPECT_EQ(side->getWeakCount(), maxWRC);
  
  EXPECT_EQ(0u, deinited);
  EXPECT_ALLOCATED(object);
  EXPECT_ALLOCATED(side);
  language_release(object);
  EXPECT_EQ(1u, deinited);
  EXPECT_UNALLOCATED(object);
  EXPECT_ALLOCATED(side);

  weakReleaseALot<false>(side, maxWRC - 2);
  EXPECT_EQ(side->getWeakCount(), 1u);

  EXPECT_ALLOCATED(side);
  side->decrementWeak();
  EXPECT_UNALLOCATED(side);
}


//////////////////////
// Object lifecycle //
//////////////////////

// FIXME: use the real WeakReference definition
namespace language {

class WeakReference {
  uintptr_t value;
  
 public:
  void *getSideTable() {
    return (void*)(value & ~3ULL);
  }
};

} // namespace language

// Lifecycle paths. One test each.
// 
// LIVE -> DEINITING                      -> DEAD, no side table
// LIVE -> DEINITING -> DEINITED          -> DEAD, no side table
// 
// LIVE -> DEINITING                      -> DEAD, with side table
// LIVE -> DEINITING -> DEINITED          -> DEAD, with side table
// LIVE -> DEINITING             -> FREED -> DEAD, with side table
// LIVE -> DEINITING -> DEINITED -> FREED -> DEAD, with side table


// LIVE -> DEINITING -> DEAD, no side table
TEST(LongRefcountingTest, lifecycle_live_deiniting_no_side_DeathTest) {
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  size_t deinited = 0;
  auto object = allocTestObject(&deinited, 1);
  object->CheckLifecycle = true;

  // Object is LIVE
  
  EXPECT_ALLOCATED(object);
  // RC tested elsewhere
  
  // URC load OK
  // URC increment OK
  // URC decrement OK
  language_unownedCheck(object);
  language_unownedRetain(object);   language_unownedCheck(object);
  language_unownedRetain(object);   language_unownedCheck(object);  
  language_unownedRetain(object);   language_unownedCheck(object);
  language_unownedRelease(object);  language_unownedCheck(object);
  language_unownedRelease(object);  language_unownedCheck(object);
  language_unownedRelease(object);  language_unownedCheck(object);

  // WRC load can't happen
  // WRC increment adds side table which is tested elsewhere
  // WRC decrement can't happen

  // RC == 1
  // URC == 1
  // WRC == 1

  language_release(object);  // DEINITING is in here
  
  // Object is DEAD
  // RC == 0
  // URC == 0
  // WRC == 0

  EXPECT_UNALLOCATED(object);
}


// LIVE -> DEINITING -> DEINITED -> DEAD, no side table
TEST(LongRefcountingTest, lifecycle_live_deiniting_deinited_no_side_DeathTest) {
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  size_t deinited = 0;
  auto object = allocTestObject(&deinited, 1);
  object->CheckLifecycle = true;

  // Object is LIVE
  
  EXPECT_ALLOCATED(object);
  // RC tested elsewhere
  
  // URC load OK
  // URC increment OK
  // URC decrement OK
  language_unownedCheck(object);
  language_unownedRetain(object);   language_unownedCheck(object);
  language_unownedRetain(object);   language_unownedCheck(object);  
  language_unownedRetain(object);   language_unownedCheck(object);
  language_unownedRelease(object);  language_unownedCheck(object);

  // WRC load can't happen
  // WRC increment adds side table which is tested elsewhere
  // WRC decrement can't happen

  // RC == 1
  // URC == 3
  // WRC == 1

  language_release(object);  // DEINITING is in here
  
  // Object is DEINITED
  // RC == 0
  // URC == 2
  // WRC == 1

  EXPECT_EQ(1u, deinited);
  EXPECT_ALLOCATED(object);

  // RC can't happen

  // WRC can't happen

  // URC load crashes
  // URC increment can't happen
  // URC decrement OK
  ASSERT_DEATH(language_unownedCheck(object),
               "Attempted to read an unowned reference");
  language_unownedRelease(object);
  EXPECT_ALLOCATED(object);

  // RC == 0
  // URC == 1
  // WRC == 1

  language_unownedRelease(object);
  
  // Object is DEAD
  // RC == 0
  // URC == 0
  // WRC == 0

  EXPECT_UNALLOCATED(object);
}


// LIVE -> DEINITING -> DEAD, with side table
TEST(LongRefcountingTest, lifecycle_live_deiniting_with_side_DeathTest) {
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  size_t deinited = 0;
  auto object = allocTestObject(&deinited, 1);
  object->CheckLifecycle = true;

  // Object is LIVE
  
  EXPECT_ALLOCATED(object);
  // RC tested elsewhere
  
  // URC load OK
  // URC increment OK
  // URC decrement OK
  language_unownedCheck(object);
  language_unownedRetain(object);   language_unownedCheck(object);
  language_unownedRetain(object);   language_unownedCheck(object);  
  language_unownedRetain(object);   language_unownedCheck(object);
  language_unownedRelease(object);  language_unownedCheck(object);
  // Remaining releases are performed after the side table is allocated.

  // WRC load can't happen
  // WRC increment adds side table
  // WRC decrement can't happen

  WeakReference w;
  language_weakInit(&w, object);
  
  // Object is LIVE with side table

  void *side = w.getSideTable();
  EXPECT_ALLOCATED(side);

  WeakReference w_deinit;
  language_weakInit(&w_deinit, object);
  object->WeakRef = &w_deinit;
  // destroyed during deinit
  
  // RC increment ok
  // RC decrement ok
  language_retain(object);
  language_retain(object);
  language_retain(object);
  language_release(object);
  language_release(object);
  language_release(object);
  
  // URC load OK
  // URC increment OK
  // URC decrement OK
  language_unownedCheck(object);
  language_unownedRetain(object);   language_unownedCheck(object);
  language_unownedRetain(object);   language_unownedCheck(object);  
  language_unownedRetain(object);   language_unownedCheck(object);
  language_unownedRelease(object);  language_unownedCheck(object);
  language_unownedRelease(object);  language_unownedCheck(object);
  language_unownedRelease(object);  language_unownedCheck(object);
  // ...and balancing from previously...
  language_unownedRelease(object);  language_unownedCheck(object);
  language_unownedRelease(object);  language_unownedCheck(object);

  // WRC load OK
  // WRC increment OK
  // WRC decrement OK

  WeakReference w2;
  language_weakInit(&w2, object);
  HeapObject *weakValue = language_weakTakeStrong(&w2);
  EXPECT_EQ(weakValue, object);
  language_release(weakValue);

  weakValue = language_weakTakeStrong(&w);
  EXPECT_EQ(weakValue, object);
  language_release(weakValue);
  
  // RC == 1
  // URC == 1
  // WRC == 1

  language_release(object);  // DEINITING is in here
  
  // Object is DEAD
  // RC == 0
  // URC == 0
  // WRC == 0

  EXPECT_UNALLOCATED(side);
  EXPECT_UNALLOCATED(object);
}


// LIVE -> DEINITING -> DEINITED -> DEAD, with side table
TEST(LongRefcountingTest, lifecycle_live_deiniting_deinited_with_side_DeathTest) {
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  size_t deinited = 0;
  auto object = allocTestObject(&deinited, 1);
  object->CheckLifecycle = true;

  // Object is LIVE
  
  EXPECT_ALLOCATED(object);
  // RC tested elsewhere
  
  // URC load OK
  // URC increment OK
  // URC decrement OK
  language_unownedCheck(object);
  language_unownedRetain(object);   language_unownedCheck(object);
  language_unownedRetain(object);   language_unownedCheck(object);  
  language_unownedRetain(object);   language_unownedCheck(object);
  language_unownedRelease(object);  language_unownedCheck(object);
  // Remaining releases are performed during DEINITED.

  // WRC load can't happen
  // WRC increment adds side table
  // WRC decrement can't happen

  WeakReference w;
  language_weakInit(&w, object);
  
  // Object is LIVE with side table

  void *side = w.getSideTable();
  EXPECT_ALLOCATED(side);

  WeakReference w_deinit;
  language_weakInit(&w_deinit, object);
  object->WeakRef = &w_deinit;
  // destroyed during deinit

  // RC increment ok
  // RC decrement ok
  language_retain(object);
  language_retain(object);
  language_retain(object);
  language_release(object);
  language_release(object);
  language_release(object);
  
  // URC load OK
  // URC increment OK
  // URC decrement OK
  language_unownedCheck(object);
  language_unownedRetain(object);   language_unownedCheck(object);
  language_unownedRetain(object);   language_unownedCheck(object);  
  language_unownedRetain(object);   language_unownedCheck(object);
  language_unownedRelease(object);  language_unownedCheck(object);
  language_unownedRelease(object);  language_unownedCheck(object);
  language_unownedRelease(object);  language_unownedCheck(object);

  // WRC load OK
  // WRC increment OK
  // WRC decrement OK

  WeakReference w2;
  language_weakInit(&w2, object);
  HeapObject *weakValue = language_weakTakeStrong(&w2);
  EXPECT_EQ(weakValue, object);
  language_release(weakValue);

  weakValue = language_weakTakeStrong(&w);
  EXPECT_EQ(weakValue, object);
  language_release(weakValue);
  
  // RC == 1
  // URC == 3
  // WRC == 1

  language_release(object);  // DEINITING is in here

  // Object is DEINITED
  // RC == 0
  // URC == 2
  // WRC == 1
  
  EXPECT_EQ(1u, deinited);
  EXPECT_ALLOCATED(object);
  EXPECT_ALLOCATED(side);

  // RC can't happen

  // WRC can't happen

  // URC load crashes
  // URC increment can't happen
  // URC decrement OK
  ASSERT_DEATH(language_unownedCheck(object),
               "Attempted to read an unowned reference");
  language_unownedRelease(object);
  EXPECT_ALLOCATED(object);
  EXPECT_ALLOCATED(side);
  
  // RC == 0
  // URC == 1
  // WRC == 1

  language_unownedRelease(object);

  // Object is DEAD
  // RC == 0
  // URC == 0
  // WRC == 0

  EXPECT_UNALLOCATED(object);
  EXPECT_UNALLOCATED(side);
}


// LIVE -> DEINITING -> FREED -> DEAD, with side table
TEST(LongRefcountingTest, lifecycle_live_deiniting_freed_with_side_DeathTest) {
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  size_t deinited = 0;
  auto object = allocTestObject(&deinited, 1);
  object->CheckLifecycle = true;

  // Object is LIVE
  
  EXPECT_ALLOCATED(object);
  // RC tested elsewhere
  
  // URC load OK
  // URC increment OK
  // URC decrement OK
  language_unownedCheck(object);
  language_unownedRetain(object);   language_unownedCheck(object);
  language_unownedRetain(object);   language_unownedCheck(object);  
  language_unownedRetain(object);   language_unownedCheck(object);
  language_unownedRelease(object);  language_unownedCheck(object);
  language_unownedRelease(object);  language_unownedCheck(object);
  language_unownedRelease(object);  language_unownedCheck(object);

  // WRC load can't happen
  // WRC increment adds side table
  // WRC decrement can't happen

  WeakReference w;
  language_weakInit(&w, object);

  // Object is LIVE with side table
  
  void *side = w.getSideTable();
  EXPECT_ALLOCATED(side);

  WeakReference w_deinit;
  language_weakInit(&w_deinit, object);
  object->WeakRef = &w_deinit;
  // destroyed during deinit

  // RC increment ok
  // RC decrement ok
  language_retain(object);
  language_retain(object);
  language_retain(object);
  language_release(object);
  language_release(object);
  language_release(object);
  
  // URC load OK
  // URC increment OK
  // URC decrement OK
  language_unownedCheck(object);
  language_unownedRetain(object);   language_unownedCheck(object);
  language_unownedRetain(object);   language_unownedCheck(object);  
  language_unownedRetain(object);   language_unownedCheck(object);
  language_unownedRelease(object);  language_unownedCheck(object);
  language_unownedRelease(object);  language_unownedCheck(object);
  language_unownedRelease(object);  language_unownedCheck(object);

  // WRC load OK
  // WRC increment OK
  // WRC decrement OK

  WeakReference w2;
  language_weakInit(&w2, object);
  HeapObject *weakValue = language_weakLoadStrong(&w2);
  EXPECT_EQ(weakValue, object);
  language_release(weakValue);

  weakValue = language_weakLoadStrong(&w);
  EXPECT_EQ(weakValue, object);
  language_release(weakValue);
  
  // RC == 1
  // URC == 1
  // WRC == 3

  language_release(object);  // DEINITING is in here

  // Object is FREED
  // RC == 0
  // URC == 0
  // WRC == 2
  
  EXPECT_EQ(1u, deinited);
  EXPECT_UNALLOCATED(object);
  EXPECT_ALLOCATED(side);

  // RC can't happen

  // URC can't happen

  // WRC load is nil
  // WRC increment can't happen
  // WRC decrement OK

  weakValue = language_weakTakeStrong(&w2);
  EXPECT_EQ(0, weakValue);
  
  // RC == 0
  // URC == 0
  // WRC == 1

  weakValue = language_weakTakeStrong(&w);

  // Object is DEAD
  // RC == 0
  // URC == 0
  // WRC == 0

  EXPECT_UNALLOCATED(side);
  EXPECT_EQ(0, weakValue);
}


// LIVE -> DEINITING -> DEINITED -> FREED -> DEAD, with side table
TEST(LongRefcountingTest, lifecycle_live_deiniting_deinited_freed_with_side_DeathTest) {
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  size_t deinited = 0;
  auto object = allocTestObject(&deinited, 1);
  object->CheckLifecycle = true;

  // Object is LIVE
  
  EXPECT_ALLOCATED(object);
  // RC tested elsewhere
  
  // URC load OK
  // URC increment OK
  // URC decrement OK
  language_unownedCheck(object);
  language_unownedRetain(object);   language_unownedCheck(object);
  language_unownedRetain(object);   language_unownedCheck(object);  
  language_unownedRetain(object);   language_unownedCheck(object);
  language_unownedRelease(object);  language_unownedCheck(object);
  // Remaining releases are performed during DEINITED.
  
  // WRC load can't happen
  // WRC increment adds side table
  // WRC decrement can't happen

  WeakReference w;
  language_weakInit(&w, object);

  // Object is LIVE with side table

  void *side = w.getSideTable();
  EXPECT_ALLOCATED(side);
  
  WeakReference w_deinit;
  language_weakInit(&w_deinit, object);
  object->WeakRef = &w_deinit;
  // destroyed during deinit  

  // RC increment ok
  // RC decrement ok
  language_retain(object);
  language_retain(object);
  language_retain(object);
  language_release(object);
  language_release(object);
  language_release(object);
  
  // URC load OK
  // URC increment OK
  // URC decrement OK
  language_unownedCheck(object);
  language_unownedRetain(object);   language_unownedCheck(object);
  language_unownedRetain(object);   language_unownedCheck(object);  
  language_unownedRetain(object);   language_unownedCheck(object);
  language_unownedRelease(object);  language_unownedCheck(object);
  language_unownedRelease(object);  language_unownedCheck(object);
  language_unownedRelease(object);  language_unownedCheck(object);

  // WRC load OK
  // WRC increment OK
  // WRC decrement OK

  WeakReference w2;
  language_weakInit(&w2, object);
  HeapObject *weakValue = language_weakLoadStrong(&w2);
  EXPECT_EQ(weakValue, object);
  language_release(weakValue);

  weakValue = language_weakLoadStrong(&w);
  EXPECT_EQ(weakValue, object);
  language_release(weakValue);
  
  // RC == 1
  // URC == 3
  // WRC == 3

  language_release(object);  // DEINITING is in here

  // Object is DEINITED
  // RC == 0
  // URC == 2
  // WRC == 3

  EXPECT_EQ(1u, deinited);
  EXPECT_ALLOCATED(object);
  EXPECT_ALLOCATED(side);

  // RC can't happen

  // WRC load is nil
  // WRC increment can't happen
  // WRC decrement OK

  weakValue = language_weakTakeStrong(&w2);
  EXPECT_EQ(0, weakValue);

  // URC load crashes
  // URC increment can't happen
  // URC decrement OK
  ASSERT_DEATH(language_unownedCheck(object),
               "Attempted to read an unowned reference");
  language_unownedRelease(object);
  EXPECT_ALLOCATED(object);
  EXPECT_ALLOCATED(side);

  // RC == 0
  // URC == 1
  // WRC == 2

  language_unownedRelease(object);

  // Object is FREED
  // RC == 0
  // URC == 0
  // WRC == 1
  
  EXPECT_EQ(1u, deinited);
  EXPECT_UNALLOCATED(object);
  EXPECT_ALLOCATED(side);

  // RC can't happen

  // URC can't happen

  // WRC load is nil
  // WRC increment can't happen
  // WRC decrement OK
  
  // RC == 0
  // URC == 0
  // WRC == 1

  weakValue = language_weakTakeStrong(&w);

  // Object is DEAD
  // RC == 0
  // URC == 0
  // WRC == 0

  EXPECT_UNALLOCATED(side);
  EXPECT_EQ(0, weakValue);
}

TEST(LongRefcountingTest, lifecycle_live_deiniting_urc_overflow_to_side) {
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  uint64_t urcOverflowCount;
  switch(sizeof(void *)) {
  // 32-bit has a 7-bit inline refcount that overflows into the side table.
  case 4: urcOverflowCount = 1 << 7; break;

  // 64-bit can't store any extra count in the side table, so there's nothing to test.
  case 8: return;

  // We should never see any other bitness.
  default: FAIL(); break;
  }

  size_t deinited = 0;
  auto object = allocTestObject(&deinited, 1);
  HeapObjectSideTableEntry *side = nullptr;
  object->DeinitCallback = [&]() {
    for (uint64_t i = 0; i < urcOverflowCount; i++) {
      language_unownedRetain(object);
    }

    side = reinterpret_cast<HeapObjectSideTableEntry *>(object->refCounts.getSideTable());
    EXPECT_ALLOCATED(side);

    for (uint64_t i = 0; i < urcOverflowCount; i++) {
      language_unownedRelease(object);
    }
  };

  language_release(object);
  EXPECT_UNALLOCATED(object);
  EXPECT_UNALLOCATED(side);
}
