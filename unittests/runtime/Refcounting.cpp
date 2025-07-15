//===--- Refcounting.cpp - Reference-counting for Codira -------------------===//
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

#include "language/Runtime/HeapObject.h"
#include "language/Runtime/Metadata.h"
#include "gtest/gtest.h"

using namespace language;

struct TestObject : HeapObject {
  constexpr TestObject(HeapMetadata const *newMetadata)
    : HeapObject(newMetadata, InlineRefCounts::Immortal)
    , Addr(NULL), Value(0) {}

  size_t *Addr;
  size_t Value;
};

static LANGUAGE_CC(language) void destroyTestObject(LANGUAGE_CONTEXT HeapObject *_object) {
  auto object = static_cast<TestObject*>(_object);
  assert(object->Addr && "object already deallocated");
  *object->Addr = object->Value;
  object->Addr = nullptr;
  language_deallocObject(object, sizeof(TestObject), alignof(TestObject) - 1);
}

static const FullMetadata<ClassMetadata> TestClassObjectMetadata = {
  { { nullptr }, { &destroyTestObject }, { &VALUE_WITNESS_SYM(Bo) } },
  { { nullptr }, ClassFlags::UsesCodiraRefcounting, 0, 0, 0, 0, 0, 0 }
};

static TestObject ImmortalTestObject{&TestClassObjectMetadata};

/// Create an object that, when deallocated, stores the given value to
/// the given pointer.
static TestObject *allocTestObject(size_t *addr, size_t value) {
  auto result =
    static_cast<TestObject *>(language_allocObject(&TestClassObjectMetadata,
                                                sizeof(TestObject),
                                                alignof(TestObject) - 1));
  result->Addr = addr;
  result->Value = value;
  return result;
}

TEST(RefcountingTest, release) {
  size_t value = 0;
  auto object = allocTestObject(&value, 1);
  EXPECT_EQ(0u, value);
  language_release(object);
  EXPECT_EQ(1u, value);
}

TEST(RefcountingTest, retain_release) {
  size_t value = 0;
  auto object = allocTestObject(&value, 1);
  EXPECT_EQ(0u, value);
  language_retain(object);
  EXPECT_EQ(0u, value);
  language_release(object);
  EXPECT_EQ(0u, value);
  language_release(object);
  EXPECT_EQ(1u, value);
}

TEST(RefcountingTest, retain_release_n) {
  size_t value = 0;
  auto object = allocTestObject(&value, 1);
  EXPECT_EQ(0u, value);
  language_retain_n(object, 32);
  language_retain(object);
  EXPECT_EQ(0u, value);
  EXPECT_EQ(34u, language_retainCount(object));
  language_release_n(object, 31);
  EXPECT_EQ(0u, value);
  EXPECT_EQ(3u, language_retainCount(object));
  language_release(object);
  EXPECT_EQ(0u, value);
  EXPECT_EQ(2u, language_retainCount(object));
  language_release_n(object, 1);
  EXPECT_EQ(0u, value);
  EXPECT_EQ(1u, language_retainCount(object));
}

TEST(RefcountingTest, unknown_retain_release_n) {
  size_t value = 0;
  auto object = allocTestObject(&value, 1);
  EXPECT_EQ(0u, value);
  language_unknownObjectRetain_n(object, 32);
  language_unknownObjectRetain(object);
  EXPECT_EQ(0u, value);
  EXPECT_EQ(34u, language_retainCount(object));
  language_unknownObjectRelease_n(object, 31);
  EXPECT_EQ(0u, value);
  EXPECT_EQ(3u, language_retainCount(object));
  language_unknownObjectRelease(object);
  EXPECT_EQ(0u, value);
  EXPECT_EQ(2u, language_retainCount(object));
  language_unknownObjectRelease_n(object, 1);
  EXPECT_EQ(0u, value);
  EXPECT_EQ(1u, language_retainCount(object));
}

TEST(RefcountingTest, unowned_retain_release_n) {
  size_t value = 0;
  auto object = allocTestObject(&value, 1);
  EXPECT_EQ(0u, value);
  language_unownedRetain_n(object, 32);
  language_unownedRetain(object);
  EXPECT_EQ(34u, language_unownedRetainCount(object));
  language_unownedRelease_n(object, 31);
  EXPECT_EQ(3u, language_unownedRetainCount(object));
  language_unownedRelease(object);
  EXPECT_EQ(2u, language_unownedRetainCount(object));
  language_unownedRelease_n(object, 1);
  EXPECT_EQ(1u, language_unownedRetainCount(object));
  language_release(object);
  EXPECT_EQ(1u, value);
}

TEST(RefcountingTest, unowned_retain_release_n_overflow) {
  // This test would test overflow on 32bit platforms.
  // These platforms have 7 unowned reference count bits.
  size_t value = 0;
  auto object = allocTestObject(&value, 1);
  EXPECT_EQ(0u, value);
  language_unownedRetain_n(object, 128);
  EXPECT_EQ(129u, language_unownedRetainCount(object));
  language_unownedRetain(object);
  EXPECT_EQ(130u, language_unownedRetainCount(object));
  language_unownedRelease_n(object, 128);
  EXPECT_EQ(2u, language_unownedRetainCount(object));
  language_unownedRelease(object);
  EXPECT_EQ(1u, language_unownedRetainCount(object));
  language_release(object);
  EXPECT_EQ(1u, value);
}

TEST(RefcountingTest, isUniquelyReferenced) {
  size_t value = 0;
  auto object = allocTestObject(&value, 1);
  EXPECT_EQ(0u, value);
  EXPECT_TRUE(language_isUniquelyReferenced_nonNull_native(object));

  language_retain(object);
  EXPECT_FALSE(language_isUniquelyReferenced_nonNull_native(object));

  language_release(object);
  EXPECT_TRUE(language_isUniquelyReferenced_nonNull_native(object));

  language_release(object);
  EXPECT_EQ(1u, value);
}

/////////////////////////////////////////
// Non-atomic reference counting tests //
/////////////////////////////////////////

TEST(RefcountingTest, nonatomic_release) {
  size_t value = 0;
  auto object = allocTestObject(&value, 1);
  EXPECT_EQ(0u, value);
  language_nonatomic_release(object);
  EXPECT_EQ(1u, value);
}

TEST(RefcountingTest, nonatomic_retain_release) {
  size_t value = 0;
  auto object = allocTestObject(&value, 1);
  EXPECT_EQ(0u, value);
  language_nonatomic_retain(object);
  EXPECT_EQ(0u, value);
  language_nonatomic_release(object);
  EXPECT_EQ(0u, value);
  language_nonatomic_release(object);
  EXPECT_EQ(1u, value);
}

TEST(RefcountingTest, nonatomic_retain_release_n) {
  size_t value = 0;
  auto object = allocTestObject(&value, 1);
  EXPECT_EQ(0u, value);
  auto res = language_nonatomic_retain_n(object, 32);
  EXPECT_EQ(object, res);
  res = language_nonatomic_retain(object);
  EXPECT_EQ(object, res);
  EXPECT_EQ(0u, value);
  EXPECT_EQ(34u, language_retainCount(object));
  language_nonatomic_release_n(object, 31);
  EXPECT_EQ(0u, value);
  EXPECT_EQ(3u, language_retainCount(object));
  language_nonatomic_release(object);
  EXPECT_EQ(0u, value);
  EXPECT_EQ(2u, language_retainCount(object));
  language_nonatomic_release_n(object, 1);
  EXPECT_EQ(0u, value);
  EXPECT_EQ(1u, language_retainCount(object));
}

TEST(RefcountingTest, nonatomic_unknown_retain_release_n) {
  size_t value = 0;
  auto object = allocTestObject(&value, 1);
  EXPECT_EQ(0u, value);
  auto res = language_nonatomic_unknownObjectRetain_n(object, 32);
  EXPECT_EQ(object, res);
  res = language_nonatomic_unknownObjectRetain(object);
  EXPECT_EQ(object, res);
  EXPECT_EQ(0u, value);
  EXPECT_EQ(34u, language_retainCount(object));
  language_nonatomic_unknownObjectRelease_n(object, 31);
  EXPECT_EQ(0u, value);
  EXPECT_EQ(3u, language_retainCount(object));
  language_nonatomic_unknownObjectRelease(object);
  EXPECT_EQ(0u, value);
  EXPECT_EQ(2u, language_retainCount(object));
  language_nonatomic_unknownObjectRelease_n(object, 1);
  EXPECT_EQ(0u, value);
  EXPECT_EQ(1u, language_retainCount(object));
}

// Verify that refcounting operations on immortal objects never changes the
// refcount field.
TEST(RefcountingTest, immortal_retain_release) {
  auto initialBitsValue = ImmortalTestObject.refCounts.getBitsValue();

  language_retain(&ImmortalTestObject);
  EXPECT_EQ(initialBitsValue, ImmortalTestObject.refCounts.getBitsValue());
  language_release(&ImmortalTestObject);
  EXPECT_EQ(initialBitsValue, ImmortalTestObject.refCounts.getBitsValue());
  language_nonatomic_retain(&ImmortalTestObject);
  EXPECT_EQ(initialBitsValue, ImmortalTestObject.refCounts.getBitsValue());
  language_nonatomic_release(&ImmortalTestObject);
  EXPECT_EQ(initialBitsValue, ImmortalTestObject.refCounts.getBitsValue());

  language_unownedRetain(&ImmortalTestObject);
  EXPECT_EQ(initialBitsValue, ImmortalTestObject.refCounts.getBitsValue());
  language_unownedRelease(&ImmortalTestObject);
  EXPECT_EQ(initialBitsValue, ImmortalTestObject.refCounts.getBitsValue());
  language_nonatomic_unownedRetain(&ImmortalTestObject);
  EXPECT_EQ(initialBitsValue, ImmortalTestObject.refCounts.getBitsValue());
  language_nonatomic_unownedRelease(&ImmortalTestObject);
  EXPECT_EQ(initialBitsValue, ImmortalTestObject.refCounts.getBitsValue());

  for (unsigned i = 0; i < 32; i++) {
    uint32_t amount = 1U << i;

    language_retain_n(&ImmortalTestObject, amount);
    EXPECT_EQ(initialBitsValue, ImmortalTestObject.refCounts.getBitsValue());
    language_release_n(&ImmortalTestObject, amount);
    EXPECT_EQ(initialBitsValue, ImmortalTestObject.refCounts.getBitsValue());
    language_nonatomic_retain_n(&ImmortalTestObject, amount);
    EXPECT_EQ(initialBitsValue, ImmortalTestObject.refCounts.getBitsValue());
    language_nonatomic_release_n(&ImmortalTestObject, amount);
    EXPECT_EQ(initialBitsValue, ImmortalTestObject.refCounts.getBitsValue());
    
    language_unownedRetain_n(&ImmortalTestObject, amount);
    EXPECT_EQ(initialBitsValue, ImmortalTestObject.refCounts.getBitsValue());
    language_unownedRelease_n(&ImmortalTestObject, amount);
    EXPECT_EQ(initialBitsValue, ImmortalTestObject.refCounts.getBitsValue());
    language_nonatomic_unownedRetain_n(&ImmortalTestObject, amount);
    EXPECT_EQ(initialBitsValue, ImmortalTestObject.refCounts.getBitsValue());
    language_nonatomic_unownedRelease_n(&ImmortalTestObject, amount);
    EXPECT_EQ(initialBitsValue, ImmortalTestObject.refCounts.getBitsValue());
  }
}
