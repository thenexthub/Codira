//===------ Array.cpp - Array runtime function tests ----------------------===//
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

#include "language/Runtime/Metadata.h"
#include "language/Runtime/HeapObject.h"
#include "language/Runtime/Concurrent.h"
#include "language/Runtime/HeapObject.h"
#include "language/Runtime/Metadata.h"
#include "gtest/gtest.h"

using namespace language;

namespace language {
  void installCommonValueWitnesses(const TypeLayout &layout,
                                   ValueWitnessTable *vwtable);
} // namespace language


static void initialize_pod_witness_table_size_uint32_t_stride_uint64_t(
    ValueWitnessTable &testTable) {
  testTable.size = sizeof(uint32_t);
  testTable.flags = ValueWitnessFlags()
    .withAlignment(alignof(ValueBuffer))
    .withPOD(true)
    .withBitwiseTakable(true)
    .withInlineStorage(true);
  testTable.stride = sizeof(uint64_t);
  installCommonValueWitnesses(*testTable.getTypeLayout(), &testTable);
}

extern "C" void language_arrayInitWithCopy(OpaqueValue *dest,
                                               OpaqueValue *src, size_t count,
                                               const Metadata *self);

#define COPY_POD_TEST(kind)                                                    \
  ValueWitnessTable pod_witnesses;                                             \
  memset(&pod_witnesses, 0, sizeof(pod_witnesses));                            \
  initialize_pod_witness_table_size_uint32_t_stride_uint64_t(pod_witnesses);   \
  uint64_t srcArray[3] = {0, 1, 2};                                            \
  uint64_t destArray[3] = {0x5A5A5A5AU, 0x5A5A5A5AU, 0x5A5A5A5AU};             \
  FullOpaqueMetadata testMetadata = {{&pod_witnesses},                         \
                                     {{MetadataKind::Opaque}}};                \
  Metadata *metadata = &testMetadata.base;                                     \
  language_array##kind((OpaqueValue *)destArray, (OpaqueValue *)srcArray, 3,      \
                    metadata);                                                 \
  EXPECT_EQ(destArray[0], 0u);                                                 \
  EXPECT_EQ(destArray[1], 1u);                                                 \
  EXPECT_EQ(destArray[2], 2u);                                                 \
                                                                               \
  EXPECT_EQ(srcArray[0], 0u);                                                  \
  EXPECT_EQ(srcArray[1], 1u);                                                  \
  EXPECT_EQ(srcArray[2], 2u);

TEST(TestArrayCopy, test_language_arrayInitWithCopy) {
  COPY_POD_TEST(InitWithCopy);
}


namespace {
struct TestObject : HeapObject {
  size_t *Addr;
  size_t Value;
};
}

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

TEST(TestArrayCopy, test_language_arrayInitWithCopyNoAliasNonPOD) {
  size_t object0Val = 0;
  size_t object1Val = 1;
  HeapObject *object0 = allocTestObject(&object0Val, 2);
  HeapObject *object1 = allocTestObject(&object1Val, 3);

  HeapObject *srcArray[2] = {object0, object1};
  HeapObject *destArray[2] = {(HeapObject *)0x5A5A5A5AU,
                              (HeapObject *)0x5A5A5A5AU};
  const Metadata *metadata = &TestClassObjectMetadata;
  language_arrayInitWithCopy((OpaqueValue *)destArray, (OpaqueValue *)srcArray, 2,
                          metadata);
  EXPECT_EQ(destArray[0], object0);
  EXPECT_EQ(destArray[1], object1);

  EXPECT_EQ(srcArray[0], object0);
  EXPECT_EQ(srcArray[1], object1);

  EXPECT_EQ(language_retainCount(object0), 2u);
  EXPECT_EQ(language_retainCount(object1), 2u);

  language_release(object0);
  language_release(object0);
  language_release(object1);
  language_release(object1);
  EXPECT_EQ(object0Val, 2u);
  EXPECT_EQ(object1Val, 3u);
}

extern "C" void language_arrayInitWithTakeNoAlias(OpaqueValue *dest,
                                               OpaqueValue *src, size_t count,
                                               const Metadata *self);
extern "C" void language_arrayInitWithTakeFrontToBack(OpaqueValue *dest,
                                                   OpaqueValue *src,
                                                   size_t count,
                                                   const Metadata *self);
extern "C" void language_arrayInitWithTakeBackToFront(OpaqueValue *dest,
                                                   OpaqueValue *src,
                                                   size_t count,
                                                   const Metadata *self);
TEST(TestArrayCopy, test_language_arrayInitWithTakeNoAliasPOD) {
  COPY_POD_TEST(InitWithTakeNoAlias);
}

TEST(TestArrayCopy, test_language_arrayInitWithTakeFrontToBackPOD) {
  COPY_POD_TEST(InitWithTakeFrontToBack);
}

TEST(TestArrayCopy, test_language_arrayInitWithTakeBackToFrontPOD) {
  COPY_POD_TEST(InitWithTakeBackToFront);
}

#define INITWITHTAKE_NONPOD_TEST(copy)                                         \
  size_t object0Val = 0;                                                       \
  size_t object1Val = 1;                                                       \
  HeapObject *object0 = allocTestObject(&object0Val, 2);                       \
  HeapObject *object1 = allocTestObject(&object1Val, 3);                       \
                                                                               \
  HeapObject *srcArray[2] = {object0, object1};                                \
  HeapObject *destArray[2] = {(HeapObject *)0x5A5A5A5AU,                       \
                              (HeapObject *)0x5A5A5A5AU};                      \
  const Metadata *metadata = &TestClassObjectMetadata;                         \
  language_arrayInitWithTake##copy((OpaqueValue *)destArray,                      \
                                (OpaqueValue *)srcArray, 2, metadata);         \
  EXPECT_EQ(destArray[0], object0);                                            \
  EXPECT_EQ(destArray[1], object1);                                            \
                                                                               \
  EXPECT_EQ(srcArray[0], object0);                                             \
  EXPECT_EQ(srcArray[1], object1);                                             \
                                                                               \
  EXPECT_EQ(language_retainCount(object0), 1u);                                   \
  EXPECT_EQ(language_retainCount(object1), 1u);                                   \
                                                                               \
  language_release(object0);                                                      \
  language_release(object1);                                                      \
  EXPECT_EQ(object0Val, 2u);                                                   \
  EXPECT_EQ(object1Val, 3u);

TEST(TestArrayCopy, test_language_arrayInitWithTakeNoAliasNonPOD) {
  INITWITHTAKE_NONPOD_TEST(NoAlias);
}

TEST(TestArrayCopy, test_language_arrayInitWithTakeFrontToBackNonPOD) {
  INITWITHTAKE_NONPOD_TEST(FrontToBack);
}

TEST(TestArrayCopy, test_language_arrayInitWithTakeBackToFrontNonPOD) {
  INITWITHTAKE_NONPOD_TEST(BackToFront);
}

extern "C" void language_arrayAssignWithCopyNoAlias(OpaqueValue *dest, OpaqueValue *src,
                                      size_t count, const Metadata *self);
extern "C" void language_arrayAssignWithCopyFrontToBack(OpaqueValue *dest, OpaqueValue *src,
                                          size_t count, const Metadata *self);
extern "C" void language_arrayAssignWithCopyBackToFront(OpaqueValue *dest, OpaqueValue *src,
                                          size_t count, const Metadata *self);

TEST(TestArrayCopy, test_language_arrayAssignWithCopyNoAliasPOD) {
  COPY_POD_TEST(AssignWithCopyNoAlias);
}

TEST(TestArrayCopy, test_language_arrayAssignWithCopyFrontToBackPOD) {
  COPY_POD_TEST(AssignWithCopyFrontToBack);
}

TEST(TestArrayCopy, test_language_arrayAssignWithCopyBackToFrontPOD) {
  COPY_POD_TEST(AssignWithCopyBackToFront);
}

#define ASSIGNWITHCOPY_NONPOD_TEST(copy)                                       \
  size_t object0Val = 0;                                                       \
  size_t object1Val = 1;                                                       \
  HeapObject *object0 = allocTestObject(&object0Val, 2);                       \
  HeapObject *object1 = allocTestObject(&object1Val, 3);                       \
  size_t object2Val = 4;                                                       \
  size_t object3Val = 5;                                                       \
  HeapObject *object2 = allocTestObject(&object2Val, 6);                       \
  HeapObject *object3 = allocTestObject(&object3Val, 7);                       \
                                                                               \
  HeapObject *srcArray[2] = {object0, object1};                                \
  HeapObject *destArray[2] = {object2, object3};                               \
  const Metadata *metadata = &TestClassObjectMetadata;                         \
  language_arrayAssignWithCopy##copy((OpaqueValue *)destArray,                    \
                                  (OpaqueValue *)srcArray, 2, metadata);       \
  EXPECT_EQ(destArray[0], object0);                                            \
  EXPECT_EQ(destArray[1], object1);                                            \
                                                                               \
  EXPECT_EQ(srcArray[0], object0);                                             \
  EXPECT_EQ(srcArray[1], object1);                                             \
                                                                               \
  EXPECT_EQ(language_retainCount(object0), 2u);                                   \
  EXPECT_EQ(language_retainCount(object1), 2u);                                   \
                                                                               \
  EXPECT_EQ(object2Val, 6u);                                                   \
  EXPECT_EQ(object3Val, 7u);                                                   \
                                                                               \
  language_release(object0);                                                      \
  language_release(object0);                                                      \
  language_release(object1);                                                      \
  language_release(object1);                                                      \
  EXPECT_EQ(object0Val, 2u);                                                   \
  EXPECT_EQ(object1Val, 3u);

TEST(TestArrayCopy, test_language_arrayAssignWithCopyNoAliasNonPOD) {
  ASSIGNWITHCOPY_NONPOD_TEST(NoAlias);
}

TEST(TestArrayCopy, test_language_arrayAssignWithCopyFrontToBackNonPOD) {
  ASSIGNWITHCOPY_NONPOD_TEST(FrontToBack);
}

TEST(TestArrayCopy, test_language_arrayAssignWithCopyBackToFrontNonPOD) {
  ASSIGNWITHCOPY_NONPOD_TEST(BackToFront);
}

extern "C" void language_arrayAssignWithTake(OpaqueValue *dest, OpaqueValue *src,
                                          size_t count, const Metadata *self);

TEST(TestArrayCopy, test_language_arrayAssignWithTakePOD) {
  COPY_POD_TEST(AssignWithTake);
}

TEST(TestArrayCopy, test_language_arrayAssignWithTakeNonPOD) {
  size_t object0Val = 0;
  size_t object1Val = 1;
  HeapObject *object0 = allocTestObject(&object0Val, 2);
  HeapObject *object1 = allocTestObject(&object1Val, 3);
  size_t object2Val = 4;
  size_t object3Val = 5;
  HeapObject *object2 = allocTestObject(&object2Val, 6);
  HeapObject *object3 = allocTestObject(&object3Val, 7);

  HeapObject *srcArray[2] = {object0, object1};
  HeapObject *destArray[2] = {object2, object3};
  const Metadata *metadata = &TestClassObjectMetadata;
  language_arrayAssignWithTake((OpaqueValue *)destArray, (OpaqueValue *)srcArray,
                            2, metadata);
  EXPECT_EQ(destArray[0], object0);
  EXPECT_EQ(destArray[1], object1);

  EXPECT_EQ(srcArray[0], object0);
  EXPECT_EQ(srcArray[1], object1);

  EXPECT_EQ(language_retainCount(object0), 1u);
  EXPECT_EQ(language_retainCount(object1), 1u);

  EXPECT_EQ(object2Val, 6u);
  EXPECT_EQ(object3Val, 7u);

  language_release(object0);
  language_release(object1);
  EXPECT_EQ(object0Val, 2u);
  EXPECT_EQ(object1Val, 3u);
}

extern "C" void language_arrayDestroy(OpaqueValue *begin, size_t count,
                                   const Metadata *self);

TEST(TestArrayCopy, test_language_arrayDestroyPOD) {
  ValueWitnessTable pod_witnesses;
  memset(&pod_witnesses, 0, sizeof(pod_witnesses));
  initialize_pod_witness_table_size_uint32_t_stride_uint64_t(pod_witnesses);
  uint64_t array[3] = {0, 1, 2};
  FullOpaqueMetadata testMetadata = {{&pod_witnesses},
                                     {{MetadataKind::Opaque}}};
  Metadata *metadata = &testMetadata.base;
  // Let's make sure this does not crash.
  language_arrayDestroy((OpaqueValue *)array, 3, metadata);
}

TEST(TestArrayCopy, test_language_arrayDestroyNonPOD) {
  size_t object0Val = 0;
  size_t object1Val = 1;
  HeapObject *object0 = allocTestObject(&object0Val, 2);
  HeapObject *object1 = allocTestObject(&object1Val, 3);
  HeapObject *array[2] = {object0, object1};
  const Metadata *metadata = &TestClassObjectMetadata;
  language_arrayDestroy((OpaqueValue *)array, 2, metadata);
  EXPECT_EQ(object0Val, 2u);
  EXPECT_EQ(object1Val, 3u);
}
