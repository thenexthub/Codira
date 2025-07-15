//===--- weak.mm - Weak-pointer tests -------------------------------------===//
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

#include <Foundation/NSObject.h>
#include <objc/runtime.h>
#include "language/Runtime/HeapObject.h"
#include "language/Runtime/Metadata.h"
#include "gtest/gtest.h"

using namespace language;

// A fake definition of Codira runtime's WeakReference.
// This has the proper size and alignment which is all we need.
namespace language {
class WeakReference { void *value __attribute__((unused)); };
}

// Declare some Objective-C stuff.
extern "C" void objc_release(id);

static unsigned DestroyedObjCCount = 0;

/// A trivial class that increments DestroyedObjCCount when deallocated.
@interface ObjCClass : NSObject @end
@implementation ObjCClass
- (void) dealloc {
  DestroyedObjCCount++;
  [super dealloc];
}
@end
static HeapObject *make_objc_object() {
  return (HeapObject*) [ObjCClass new];
}

// Make a Native Codira object by calling a Codira function.
// _language_StdlibUnittest_make_language_object is implemented in StdlibUnittest.
LANGUAGE_CC(language) extern "C"
HeapObject *_language_StdlibUnittest_make_language_object();

static HeapObject *make_language_object() {
  return _language_StdlibUnittest_make_language_object();
}

static unsigned getUnownedRetainCount(HeapObject *object) {
  return language_unownedRetainCount(object) - 1;
}

static void unknown_release(void *value) {
  objc_release((id) value);
}

TEST(WeakTest, preconditions) {
  language_release(make_language_object());
  unknown_release(make_objc_object());
}

TEST(WeakTest, simple_language) {
  HeapObject *o1 = make_language_object();
  HeapObject *o2 = make_language_object();
  ASSERT_NE(o1, o2);
  ASSERT_NE(o1, nullptr);
  ASSERT_NE(o2, nullptr);

  WeakReference ref1;
  auto res = language_weakInit(&ref1, o1);
  ASSERT_EQ(res, &ref1);

  HeapObject *tmp = language_weakLoadStrong(&ref1);
  ASSERT_EQ(tmp, o1);
  language_release(tmp);

  tmp = language_weakLoadStrong(&ref1);
  ASSERT_EQ(o1, tmp);
  language_release(tmp);

  res = language_weakAssign(&ref1, o2);
  ASSERT_EQ(res, &ref1);
  tmp = language_weakLoadStrong(&ref1);
  ASSERT_EQ(o2, tmp);
  language_release(tmp);

  tmp = language_weakLoadStrong(&ref1);
  ASSERT_EQ(o2, tmp);
  language_release(tmp);

  language_release(o1);

  tmp = language_weakLoadStrong(&ref1);
  ASSERT_EQ(o2, tmp);
  language_release(tmp);

  language_release(o2);

  tmp = language_weakLoadStrong(&ref1);
  ASSERT_EQ(nullptr, tmp);
  language_release(tmp);

  WeakReference ref2;
  res = language_weakCopyInit(&ref2, &ref1);
  ASSERT_EQ(res, &ref2);

  WeakReference ref3;
  res = language_weakTakeInit(&ref3, &ref2);
  ASSERT_EQ(res, &ref3);

  HeapObject *o3 = make_language_object();
  WeakReference ref4; // ref4 = init
  res = language_weakInit(&ref4, o3);
  ASSERT_EQ(res, &ref4);

  res = language_weakCopyAssign(&ref4, &ref3);
  ASSERT_EQ(res, &ref4);

  res = language_weakTakeAssign(&ref4, &ref3);
  ASSERT_EQ(res, &ref4);

  language_weakDestroy(&ref4);
  language_weakDestroy(&ref1);
  language_release(o3);
}

TEST(WeakTest, simple_objc) {
  void *o1 = make_objc_object();
  void *o2 = make_objc_object();
  ASSERT_NE(o1, o2);
  ASSERT_NE(o1, nullptr);
  ASSERT_NE(o2, nullptr);

  DestroyedObjCCount = 0;

  WeakReference ref1;
  language_unknownObjectWeakInit(&ref1, o1);

  void *tmp = language_unknownObjectWeakLoadStrong(&ref1);
  ASSERT_EQ(tmp, o1);
  unknown_release(tmp);

  tmp = language_unknownObjectWeakLoadStrong(&ref1);
  ASSERT_EQ(o1, tmp);
  unknown_release(tmp);
  ASSERT_EQ(0U, DestroyedObjCCount);

  language_unknownObjectWeakAssign(&ref1, o2);
  tmp = language_unknownObjectWeakLoadStrong(&ref1);
  ASSERT_EQ(o2, tmp);
  unknown_release(tmp);
  ASSERT_EQ(0U, DestroyedObjCCount);

  tmp = language_unknownObjectWeakLoadStrong(&ref1);
  ASSERT_EQ(o2, tmp);
  unknown_release(tmp);
  ASSERT_EQ(0U, DestroyedObjCCount);

  unknown_release(o1);
  ASSERT_EQ(1U, DestroyedObjCCount);

  tmp = language_unknownObjectWeakLoadStrong(&ref1);
  ASSERT_EQ(o2, tmp);
  unknown_release(tmp);
  ASSERT_EQ(1U, DestroyedObjCCount);

  unknown_release(o2);
  ASSERT_EQ(2U, DestroyedObjCCount);

  tmp = language_unknownObjectWeakLoadStrong(&ref1);
  ASSERT_EQ(nullptr, tmp);
  unknown_release(tmp);
  ASSERT_EQ(2U, DestroyedObjCCount);

  language_unknownObjectWeakDestroy(&ref1);
}

TEST(WeakTest, simple_language_as_unknown) {
  void *o1 = make_language_object();
  void *o2 = make_language_object();
  ASSERT_NE(o1, o2);
  ASSERT_NE(o1, nullptr);
  ASSERT_NE(o2, nullptr);

  WeakReference ref1;
  language_unknownObjectWeakInit(&ref1, o1);

  void *tmp = language_unknownObjectWeakLoadStrong(&ref1);
  ASSERT_EQ(tmp, o1);
  unknown_release(tmp);

  tmp = language_unknownObjectWeakLoadStrong(&ref1);
  ASSERT_EQ(o1, tmp);
  unknown_release(tmp);

  language_unknownObjectWeakAssign(&ref1, o2);
  tmp = language_unknownObjectWeakLoadStrong(&ref1);
  ASSERT_EQ(o2, tmp);
  unknown_release(tmp);

  tmp = language_unknownObjectWeakLoadStrong(&ref1);
  ASSERT_EQ(o2, tmp);
  unknown_release(tmp);

  unknown_release(o1);

  tmp = language_unknownObjectWeakLoadStrong(&ref1);
  ASSERT_EQ(o2, tmp);
  unknown_release(tmp);

  unknown_release(o2);

  tmp = language_unknownObjectWeakLoadStrong(&ref1);
  ASSERT_EQ(nullptr, tmp);
  unknown_release(tmp);

  language_unknownObjectWeakDestroy(&ref1);
}

TEST(WeakTest, simple_language_and_objc) {
  void *o1 = make_language_object();
  void *o2 = make_objc_object();
  ASSERT_NE(o1, o2);
  ASSERT_NE(o1, nullptr);
  ASSERT_NE(o2, nullptr);

  DestroyedObjCCount = 0;

  WeakReference ref1;
  language_unknownObjectWeakInit(&ref1, o1);

  void *tmp = language_unknownObjectWeakLoadStrong(&ref1);
  ASSERT_EQ(tmp, o1);
  unknown_release(tmp);

  tmp = language_unknownObjectWeakLoadStrong(&ref1);
  ASSERT_EQ(o1, tmp);
  unknown_release(tmp);
  ASSERT_EQ(0U, DestroyedObjCCount);

  language_unknownObjectWeakAssign(&ref1, o2);
  tmp = language_unknownObjectWeakLoadStrong(&ref1);
  ASSERT_EQ(o2, tmp);
  unknown_release(tmp);
  ASSERT_EQ(0U, DestroyedObjCCount);

  tmp = language_unknownObjectWeakLoadStrong(&ref1);
  ASSERT_EQ(o2, tmp);
  unknown_release(tmp);
  ASSERT_EQ(0U, DestroyedObjCCount);

  unknown_release(o1);
  ASSERT_EQ(0U, DestroyedObjCCount);

  tmp = language_unknownObjectWeakLoadStrong(&ref1);
  ASSERT_EQ(o2, tmp);
  unknown_release(tmp);
  ASSERT_EQ(0U, DestroyedObjCCount);

  unknown_release(o2);
  ASSERT_EQ(1U, DestroyedObjCCount);

  tmp = language_unknownObjectWeakLoadStrong(&ref1);
  ASSERT_EQ(nullptr, tmp);
  unknown_release(tmp);
  ASSERT_EQ(1U, DestroyedObjCCount);

  language_unknownObjectWeakDestroy(&ref1);
}

TEST(WeakTest, simple_objc_and_language) {
  void *o1 = make_objc_object();
  void *o2 = make_language_object();
  ASSERT_NE(o1, o2);
  ASSERT_NE(o1, nullptr);
  ASSERT_NE(o2, nullptr);

  DestroyedObjCCount = 0;

  WeakReference ref1;
  auto res = language_unknownObjectWeakInit(&ref1, o1);
  ASSERT_EQ(&ref1, res);

  void *tmp = language_unknownObjectWeakLoadStrong(&ref1);
  ASSERT_EQ(tmp, o1);
  unknown_release(tmp);

  tmp = language_unknownObjectWeakLoadStrong(&ref1);
  ASSERT_EQ(o1, tmp);
  unknown_release(tmp);
  ASSERT_EQ(0U, DestroyedObjCCount);

  language_unknownObjectWeakAssign(&ref1, o2);
  tmp = language_unknownObjectWeakLoadStrong(&ref1);
  ASSERT_EQ(o2, tmp);
  unknown_release(tmp);
  ASSERT_EQ(0U, DestroyedObjCCount);

  tmp = language_unknownObjectWeakLoadStrong(&ref1);
  ASSERT_EQ(o2, tmp);
  unknown_release(tmp);
  ASSERT_EQ(0U, DestroyedObjCCount);

  unknown_release(o1);
  ASSERT_EQ(1U, DestroyedObjCCount);

  tmp = language_unknownObjectWeakLoadStrong(&ref1);
  ASSERT_EQ(o2, tmp);
  unknown_release(tmp);
  ASSERT_EQ(1U, DestroyedObjCCount);

  unknown_release(o2);
  ASSERT_EQ(1U, DestroyedObjCCount);

  tmp = language_unknownObjectWeakLoadStrong(&ref1);
  ASSERT_EQ(nullptr, tmp);
  unknown_release(tmp);
  ASSERT_EQ(1U, DestroyedObjCCount);

  language_unknownObjectWeakDestroy(&ref1);
}

TEST(WeakTest, objc_unowned_basic) {
  UnownedReference ref;
  void *objc1 = make_objc_object();
  void *objc2 = make_objc_object();
  HeapObject *language1 = make_language_object();
  HeapObject *language2 = make_language_object();

  ASSERT_NE(objc1, objc2);
  ASSERT_NE(language1, language2);
  ASSERT_EQ(0U, getUnownedRetainCount(language1));
  ASSERT_EQ(0U, getUnownedRetainCount(language2));

  void *result;

  // ref = language1
  language_unknownObjectUnownedInit(&ref, language1);
  ASSERT_EQ(1U, getUnownedRetainCount(language1));
  result = language_unknownObjectUnownedLoadStrong(&ref);
  ASSERT_EQ(language1, result);
  ASSERT_EQ(1U, getUnownedRetainCount(language1));
  language_unknownObjectRelease(result);
  language_unknownObjectUnownedDestroy(&ref);
  ASSERT_EQ(0U, getUnownedRetainCount(language1));

  // ref = objc1
  language_unknownObjectUnownedInit(&ref, objc1);
  result = language_unknownObjectUnownedLoadStrong(&ref);
  ASSERT_EQ(objc1, result);
  language_unknownObjectRelease(result);

  // ref = objc1 (objc self transition)
  language_unknownObjectUnownedAssign(&ref, objc1);
  result = language_unknownObjectUnownedLoadStrong(&ref);
  ASSERT_EQ(objc1, result);
  language_unknownObjectRelease(result);

  // ref = objc2 (objc -> objc transition)
  language_unknownObjectUnownedAssign(&ref, objc2);
  result = language_unknownObjectUnownedLoadStrong(&ref);
  ASSERT_EQ(objc2, result);
  language_unknownObjectRelease(result);

  // ref = language1 (objc -> language transition)
  language_unknownObjectUnownedAssign(&ref, language1);
  ASSERT_EQ(1U, getUnownedRetainCount(language1));
  result = language_unknownObjectUnownedLoadStrong(&ref);
  ASSERT_EQ(language1, result);
  ASSERT_EQ(1U, getUnownedRetainCount(language1));
  language_unknownObjectRelease(result);

  // ref = language1 (language self transition)
  language_unknownObjectUnownedAssign(&ref, language1);
  result = language_unknownObjectUnownedLoadStrong(&ref);
  ASSERT_EQ(language1, result);
  ASSERT_EQ(1U, getUnownedRetainCount(language1));
  language_unknownObjectRelease(result);

  // ref = language2 (language -> language transition)
  language_unknownObjectUnownedAssign(&ref, language2);
  result = language_unknownObjectUnownedLoadStrong(&ref);
  ASSERT_EQ(language2, result);
  ASSERT_EQ(0U, getUnownedRetainCount(language1));
  ASSERT_EQ(1U, getUnownedRetainCount(language2));
  language_unknownObjectRelease(result);

  // ref = objc1 (language -> objc transition)
  language_unknownObjectUnownedAssign(&ref, objc1);
  result = language_unknownObjectUnownedLoadStrong(&ref);
  ASSERT_EQ(objc1, result);
  ASSERT_EQ(0U, getUnownedRetainCount(language2));
  language_unknownObjectRelease(result);

  language_unknownObjectUnownedDestroy(&ref);

  language_unknownObjectRelease(objc1);
  language_unknownObjectRelease(objc2);
  language_unknownObjectRelease(language1);
  language_unknownObjectRelease(language2);
}

TEST(WeakTest, objc_unowned_takeStrong) {
  UnownedReference ref;
  void *objc1 = make_objc_object();
  HeapObject *language1 = make_language_object();

  void *result;

  // ref = objc1
  language_unknownObjectUnownedInit(&ref, objc1);
  result = language_unknownObjectUnownedTakeStrong(&ref);
  ASSERT_EQ(objc1, result);
  language_unknownObjectRelease(result);

  // ref = language1
  language_unknownObjectUnownedInit(&ref, language1);
  ASSERT_EQ(1U, getUnownedRetainCount(language1));
  result = language_unknownObjectUnownedTakeStrong(&ref);
  ASSERT_EQ(language1, result);
  ASSERT_EQ(0U, getUnownedRetainCount(language1));
  language_unknownObjectRelease(result);

  language_unknownObjectRelease(objc1);
  language_unknownObjectRelease(language1);
}

TEST(WeakTest, objc_unowned_copyInit_nil) {
  UnownedReference ref1;
  UnownedReference ref2;

  void *result;

  // ref1 = nil
  language_unknownObjectUnownedInit(&ref1, nullptr);
  result = language_unknownObjectUnownedLoadStrong(&ref1);
  ASSERT_EQ(nullptr, result);

  // ref2 = ref1 (nil -> nil)
  auto res = language_unknownObjectUnownedCopyInit(&ref2, &ref1);
  ASSERT_EQ(&ref2, res);
  result = language_unknownObjectUnownedLoadStrong(&ref1);
  ASSERT_EQ(nullptr, result);
  result = language_unknownObjectUnownedLoadStrong(&ref2);
  ASSERT_EQ(nullptr, result);
  language_unknownObjectUnownedDestroy(&ref2);
}

TEST(WeakTest, objc_unowned_copyInit_objc) {
  UnownedReference ref1;
  UnownedReference ref2;

  void *result;
  void *objc1 = make_objc_object();

  // ref1 = objc1
  language_unknownObjectUnownedInit(&ref1, objc1);
  result = language_unknownObjectUnownedLoadStrong(&ref1);
  ASSERT_EQ(objc1, result);
  language_unknownObjectRelease(result);

  // ref2 = ref1 (objc -> objc)
  language_unknownObjectUnownedCopyInit(&ref2, &ref1);
  result = language_unknownObjectUnownedLoadStrong(&ref1);
  ASSERT_EQ(objc1, result);
  language_unknownObjectRelease(result);
  result = language_unknownObjectUnownedLoadStrong(&ref2);
  ASSERT_EQ(objc1, result);
  language_unknownObjectRelease(result);
  language_unknownObjectUnownedDestroy(&ref2);

  language_unknownObjectUnownedDestroy(&ref1);

  language_unknownObjectRelease(objc1);
}

TEST(WeakTest, objc_unowned_copyInit_language) {
  UnownedReference ref1;
  UnownedReference ref2;

  void *result;

  HeapObject *language1 = make_language_object();
  ASSERT_EQ(0U, getUnownedRetainCount(language1));

  // ref1 = language1
  language_unknownObjectUnownedInit(&ref1, language1);
  ASSERT_EQ(1U, getUnownedRetainCount(language1));
  result = language_unknownObjectUnownedLoadStrong(&ref1);
  ASSERT_EQ(language1, result);
  language_unknownObjectRelease(result);
  ASSERT_EQ(1U, getUnownedRetainCount(language1));

  // ref2 = ref1 (language -> language)
  language_unknownObjectUnownedCopyInit(&ref2, &ref1);
  ASSERT_EQ(2U, getUnownedRetainCount(language1));
  result = language_unknownObjectUnownedLoadStrong(&ref1);
  ASSERT_EQ(language1, result);
  language_unknownObjectRelease(result);
  result = language_unknownObjectUnownedLoadStrong(&ref2);
  ASSERT_EQ(language1, result);
  language_unknownObjectRelease(result);
  ASSERT_EQ(2U, getUnownedRetainCount(language1));
  language_unknownObjectUnownedDestroy(&ref2);
  ASSERT_EQ(1U, getUnownedRetainCount(language1));

  // ref2 = ref1
  // ref2 = nil
  language_unknownObjectUnownedCopyInit(&ref2, &ref1);
  ASSERT_EQ(2U, getUnownedRetainCount(language1));
  language_unknownObjectUnownedAssign(&ref2, nullptr);
  ASSERT_EQ(1U, getUnownedRetainCount(language1));
  result = language_unknownObjectUnownedLoadStrong(&ref1);
  ASSERT_EQ(language1, result);
  language_unknownObjectRelease(result);
  result = language_unknownObjectUnownedLoadStrong(&ref2);
  ASSERT_EQ(nullptr, result);
  language_unknownObjectRelease(result);
  ASSERT_EQ(1U, getUnownedRetainCount(language1));
  language_unknownObjectUnownedDestroy(&ref2);
  ASSERT_EQ(1U, getUnownedRetainCount(language1));

  language_unknownObjectUnownedDestroy(&ref1);
  ASSERT_EQ(0U, getUnownedRetainCount(language1));

  language_unknownObjectRelease(language1);
}

TEST(WeakTest, objc_unowned_takeInit_nil) {
  UnownedReference ref1;
  UnownedReference ref2;

  void *result;

  // ref1 = nil
  language_unknownObjectUnownedInit(&ref1, nullptr);
  result = language_unknownObjectUnownedLoadStrong(&ref1);
  ASSERT_EQ(nullptr, result);

  // ref2 = ref1 (nil -> nil)
  auto res = language_unknownObjectUnownedTakeInit(&ref2, &ref1);
  ASSERT_EQ(&ref2, res);
  result = language_unknownObjectUnownedLoadStrong(&ref2);
  ASSERT_EQ(nullptr, result);
  language_unknownObjectUnownedDestroy(&ref2);
}

TEST(WeakTest, objc_unowned_takeInit_objc) {
  UnownedReference ref1;
  UnownedReference ref2;

  void *result;
  void *objc1 = make_objc_object();

  // ref1 = objc1
  language_unknownObjectUnownedInit(&ref1, objc1);
  result = language_unknownObjectUnownedLoadStrong(&ref1);
  ASSERT_EQ(objc1, result);
  language_unknownObjectRelease(result);

  // ref2 = ref1 (objc -> objc)
  language_unknownObjectUnownedTakeInit(&ref2, &ref1);
  result = language_unknownObjectUnownedLoadStrong(&ref2);
  ASSERT_EQ(objc1, result);
  language_unknownObjectRelease(result);
  language_unknownObjectUnownedDestroy(&ref2);

  language_unknownObjectRelease(objc1);
}

TEST(WeakTest, objc_unowned_takeInit_language) {
  UnownedReference ref1;
  UnownedReference ref2;

  void *result;

  HeapObject *language1 = make_language_object();
  ASSERT_EQ(0U, getUnownedRetainCount(language1));

  // ref1 = language1
  language_unknownObjectUnownedInit(&ref1, language1);
  ASSERT_EQ(1U, getUnownedRetainCount(language1));
  result = language_unknownObjectUnownedLoadStrong(&ref1);
  ASSERT_EQ(language1, result);
  language_unknownObjectRelease(result);
  ASSERT_EQ(1U, getUnownedRetainCount(language1));

  // ref2 = ref1 (language -> language)
  language_unknownObjectUnownedTakeInit(&ref2, &ref1);
  ASSERT_EQ(1U, getUnownedRetainCount(language1));
  result = language_unknownObjectUnownedLoadStrong(&ref2);
  ASSERT_EQ(language1, result);
  language_unknownObjectRelease(result);
  ASSERT_EQ(1U, getUnownedRetainCount(language1));
  language_unknownObjectUnownedDestroy(&ref2);
  ASSERT_EQ(0U, getUnownedRetainCount(language1));

  // ref1 = language1
  language_unknownObjectUnownedInit(&ref1, language1);
  ASSERT_EQ(1U, getUnownedRetainCount(language1));

  // ref2 = ref1
  // ref2 = nil
  language_unknownObjectUnownedTakeInit(&ref2, &ref1);
  ASSERT_EQ(1U, getUnownedRetainCount(language1));
  language_unknownObjectUnownedAssign(&ref2, nullptr);
  ASSERT_EQ(0U, getUnownedRetainCount(language1));
  result = language_unknownObjectUnownedLoadStrong(&ref2);
  ASSERT_EQ(nullptr, result);

  language_unknownObjectUnownedDestroy(&ref2);
  ASSERT_EQ(0U, getUnownedRetainCount(language1));

  language_unknownObjectRelease(language1);
}

TEST(WeakTest, objc_unowned_copyAssign) {
  UnownedReference ref1;
  UnownedReference ref2;
  void *objc1 = make_objc_object();
  void *objc2 = make_objc_object();
  HeapObject *language1 = make_language_object();
  HeapObject *language2 = make_language_object();

  ASSERT_NE(objc1, objc2);
  ASSERT_NE(language1, language2);
  ASSERT_EQ(0U, getUnownedRetainCount(language1));
  ASSERT_EQ(0U, getUnownedRetainCount(language2));

  void *result;

  // ref1 = objc1
  language_unknownObjectUnownedInit(&ref1, objc1);
  result = language_unknownObjectUnownedLoadStrong(&ref1);
  ASSERT_EQ(objc1, result);
  language_unknownObjectRelease(result);

  // ref2 = objc1
  language_unknownObjectUnownedInit(&ref2, objc1);
  result = language_unknownObjectUnownedLoadStrong(&ref2);
  ASSERT_EQ(objc1, result);
  language_unknownObjectRelease(result);

  // ref1 = ref2 (objc self transition)
  auto res = language_unknownObjectUnownedCopyAssign(&ref1, &ref2);
  ASSERT_EQ(&ref1, res);
  result = language_unknownObjectUnownedLoadStrong(&ref1);
  ASSERT_EQ(objc1, result);
  language_unknownObjectRelease(result);
  result = language_unknownObjectUnownedLoadStrong(&ref2);
  ASSERT_EQ(objc1, result);
  language_unknownObjectRelease(result);

  // ref2 = objc2
  language_unknownObjectUnownedAssign(&ref2, objc2);
  result = language_unknownObjectUnownedLoadStrong(&ref2);
  ASSERT_EQ(objc2, result);
  language_unknownObjectRelease(result);

  // ref1 = ref2 (objc -> objc transition)
  language_unknownObjectUnownedCopyAssign(&ref1, &ref2);
  result = language_unknownObjectUnownedLoadStrong(&ref1);
  ASSERT_EQ(objc2, result);
  language_unknownObjectRelease(result);
  result = language_unknownObjectUnownedLoadStrong(&ref2);
  ASSERT_EQ(objc2, result);
  language_unknownObjectRelease(result);

  // ref2 = language1
  language_unknownObjectUnownedAssign(&ref2, language1);
  ASSERT_EQ(1U, getUnownedRetainCount(language1));
  result = language_unknownObjectUnownedLoadStrong(&ref2);
  ASSERT_EQ(language1, result);
  ASSERT_EQ(1U, getUnownedRetainCount(language1));
  language_unknownObjectRelease(result);

  // ref1 = ref2 (objc -> language transition)
  language_unknownObjectUnownedCopyAssign(&ref1, &ref2);
  ASSERT_EQ(2U, getUnownedRetainCount(language1));
  result = language_unknownObjectUnownedLoadStrong(&ref1);
  ASSERT_EQ(language1, result);
  language_unknownObjectRelease(result);
  result = language_unknownObjectUnownedLoadStrong(&ref2);
  ASSERT_EQ(language1, result);
  language_unknownObjectRelease(result);

  // ref2 = language1
  language_unknownObjectUnownedAssign(&ref2, language1);
  ASSERT_EQ(2U, getUnownedRetainCount(language1));
  result = language_unknownObjectUnownedLoadStrong(&ref1);
  ASSERT_EQ(language1, result);
  language_unknownObjectRelease(result);
  result = language_unknownObjectUnownedLoadStrong(&ref2);
  ASSERT_EQ(language1, result);
  language_unknownObjectRelease(result);
  ASSERT_EQ(2U, getUnownedRetainCount(language1));

  // ref1 = ref2 (language self transition)
  language_unknownObjectUnownedCopyAssign(&ref1, &ref2);
  ASSERT_EQ(2U, getUnownedRetainCount(language1));
  result = language_unknownObjectUnownedLoadStrong(&ref1);
  ASSERT_EQ(language1, result);
  ASSERT_EQ(2U, getUnownedRetainCount(language1));
  language_unknownObjectRelease(result);
  result = language_unknownObjectUnownedLoadStrong(&ref2);
  ASSERT_EQ(language1, result);
  ASSERT_EQ(2U, getUnownedRetainCount(language1));
  language_unknownObjectRelease(result);

  // ref2 = language2
  language_unknownObjectUnownedAssign(&ref2, language2);
  ASSERT_EQ(1U, getUnownedRetainCount(language1));
  ASSERT_EQ(1U, getUnownedRetainCount(language2));
  result = language_unknownObjectUnownedLoadStrong(&ref2);
  ASSERT_EQ(language2, result);
  language_unknownObjectRelease(result);

  // ref1 = ref2 (language -> language transition)
  language_unknownObjectUnownedCopyAssign(&ref1, &ref2);
  ASSERT_EQ(0U, getUnownedRetainCount(language1));
  ASSERT_EQ(2U, getUnownedRetainCount(language2));
  result = language_unknownObjectUnownedLoadStrong(&ref1);
  ASSERT_EQ(language2, result);
  ASSERT_EQ(2U, getUnownedRetainCount(language2));
  language_unknownObjectRelease(result);
  result = language_unknownObjectUnownedLoadStrong(&ref2);
  ASSERT_EQ(language2, result);
  ASSERT_EQ(2U, getUnownedRetainCount(language2));
  language_unknownObjectRelease(result);

  // ref2 = objc1
  language_unknownObjectUnownedAssign(&ref2, objc1);
  result = language_unknownObjectUnownedLoadStrong(&ref2);
  ASSERT_EQ(objc1, result);
  ASSERT_EQ(1U, getUnownedRetainCount(language2));
  language_unknownObjectRelease(result);

  // ref1 = ref2 (language -> objc transition)
  language_unknownObjectUnownedCopyAssign(&ref1, &ref2);
  ASSERT_EQ(0U, getUnownedRetainCount(language1));
  ASSERT_EQ(0U, getUnownedRetainCount(language2));
  result = language_unknownObjectUnownedLoadStrong(&ref1);
  ASSERT_EQ(objc1, result);
  language_unknownObjectRelease(result);
  result = language_unknownObjectUnownedLoadStrong(&ref2);
  ASSERT_EQ(objc1, result);
  language_unknownObjectRelease(result);

  language_unknownObjectUnownedDestroy(&ref1);
  language_unknownObjectUnownedDestroy(&ref2);

  language_unknownObjectRelease(objc1);
  language_unknownObjectRelease(objc2);
  language_unknownObjectRelease(language1);
  language_unknownObjectRelease(language2);
}

TEST(WeakTest, objc_unowned_takeAssign) {
  UnownedReference ref1;
  UnownedReference ref2;
  void *objc1 = make_objc_object();
  void *objc2 = make_objc_object();
  HeapObject *language1 = make_language_object();
  HeapObject *language2 = make_language_object();

  ASSERT_NE(objc1, objc2);
  ASSERT_NE(language1, language2);
  ASSERT_EQ(0U, getUnownedRetainCount(language1));
  ASSERT_EQ(0U, getUnownedRetainCount(language2));

  void *result;

  // ref1 = objc1
  auto res = language_unknownObjectUnownedInit(&ref1, objc1);
  ASSERT_EQ(&ref1, res);
  result = language_unknownObjectUnownedLoadStrong(&ref1);
  ASSERT_EQ(objc1, result);
  language_unknownObjectRelease(result);

  // ref2 = objc1
  language_unknownObjectUnownedInit(&ref2, objc1);
  result = language_unknownObjectUnownedLoadStrong(&ref2);
  ASSERT_EQ(objc1, result);
  language_unknownObjectRelease(result);

  // ref1 = ref2 (objc self transition)
  res = language_unknownObjectUnownedTakeAssign(&ref1, &ref2);
  ASSERT_EQ(&ref1, res);
  result = language_unknownObjectUnownedLoadStrong(&ref1);
  ASSERT_EQ(objc1, result);
  language_unknownObjectRelease(result);

  // ref2 = objc2
  language_unknownObjectUnownedInit(&ref2, objc2);
  result = language_unknownObjectUnownedLoadStrong(&ref2);
  ASSERT_EQ(objc2, result);
  language_unknownObjectRelease(result);

  // ref1 = ref2 (objc -> objc transition)
  language_unknownObjectUnownedTakeAssign(&ref1, &ref2);
  result = language_unknownObjectUnownedLoadStrong(&ref1);
  ASSERT_EQ(objc2, result);
  language_unknownObjectRelease(result);

  // ref2 = language1
  language_unknownObjectUnownedInit(&ref2, language1);
  ASSERT_EQ(1U, getUnownedRetainCount(language1));
  result = language_unknownObjectUnownedLoadStrong(&ref2);
  ASSERT_EQ(language1, result);
  ASSERT_EQ(1U, getUnownedRetainCount(language1));
  language_unknownObjectRelease(result);

  // ref1 = ref2 (objc -> language transition)
  language_unknownObjectUnownedTakeAssign(&ref1, &ref2);
  ASSERT_EQ(1U, getUnownedRetainCount(language1));
  result = language_unknownObjectUnownedLoadStrong(&ref1);
  ASSERT_EQ(language1, result);
  language_unknownObjectRelease(result);

  // ref2 = language1
  language_unknownObjectUnownedInit(&ref2, language1);
  ASSERT_EQ(2U, getUnownedRetainCount(language1));
  result = language_unknownObjectUnownedLoadStrong(&ref1);
  ASSERT_EQ(language1, result);
  language_unknownObjectRelease(result);
  result = language_unknownObjectUnownedLoadStrong(&ref2);
  ASSERT_EQ(language1, result);
  language_unknownObjectRelease(result);
  ASSERT_EQ(2U, getUnownedRetainCount(language1));

  // ref1 = ref2 (language self transition)
  language_unknownObjectUnownedTakeAssign(&ref1, &ref2);
  ASSERT_EQ(1U, getUnownedRetainCount(language1));
  result = language_unknownObjectUnownedLoadStrong(&ref1);
  ASSERT_EQ(language1, result);
  ASSERT_EQ(1U, getUnownedRetainCount(language1));
  language_unknownObjectRelease(result);

  // ref2 = language2
  language_unknownObjectUnownedInit(&ref2, language2);
  ASSERT_EQ(1U, getUnownedRetainCount(language1));
  ASSERT_EQ(1U, getUnownedRetainCount(language2));
  result = language_unknownObjectUnownedLoadStrong(&ref2);
  ASSERT_EQ(language2, result);
  language_unknownObjectRelease(result);

  // ref1 = ref2 (language -> language transition)
  language_unknownObjectUnownedTakeAssign(&ref1, &ref2);
  ASSERT_EQ(0U, getUnownedRetainCount(language1));
  ASSERT_EQ(1U, getUnownedRetainCount(language2));
  result = language_unknownObjectUnownedLoadStrong(&ref1);
  ASSERT_EQ(language2, result);
  ASSERT_EQ(1U, getUnownedRetainCount(language2));
  language_unknownObjectRelease(result);

  // ref2 = objc1
  language_unknownObjectUnownedInit(&ref2, objc1);
  result = language_unknownObjectUnownedLoadStrong(&ref2);
  ASSERT_EQ(objc1, result);
  ASSERT_EQ(1U, getUnownedRetainCount(language2));
  language_unknownObjectRelease(result);

  // ref1 = ref2 (language -> objc transition)
  language_unknownObjectUnownedTakeAssign(&ref1, &ref2);
  ASSERT_EQ(0U, getUnownedRetainCount(language1));
  ASSERT_EQ(0U, getUnownedRetainCount(language2));
  result = language_unknownObjectUnownedLoadStrong(&ref1);
  ASSERT_EQ(objc1, result);
  language_unknownObjectRelease(result);

  language_unknownObjectUnownedDestroy(&ref1);

  language_unknownObjectRelease(objc1);
  language_unknownObjectRelease(objc2);
  language_unknownObjectRelease(language1);
  language_unknownObjectRelease(language2);
}

TEST(WeakTest, objc_unowned_isEqual_DeathTest) {
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";

  DestroyedObjCCount = 0;

  UnownedReference ref1;
  UnownedReference ref2;
  void *objc1 = make_objc_object();
  void *objc2 = make_objc_object();
  HeapObject *language1 = make_language_object();
  HeapObject *language2 = make_language_object();

  // ref1 = language1
  language_unownedInit(&ref1, language1);
  ASSERT_EQ(true,  language_unownedIsEqual(&ref1, language1));
  ASSERT_EQ(false, language_unownedIsEqual(&ref1, language2));
  ASSERT_EQ(true,  language_unknownObjectUnownedIsEqual(&ref1, language1));
  ASSERT_EQ(false, language_unknownObjectUnownedIsEqual(&ref1, language2));
  ASSERT_EQ(false, language_unknownObjectUnownedIsEqual(&ref1, objc1));
  ASSERT_EQ(false, language_unknownObjectUnownedIsEqual(&ref1, objc2));

  // ref2 = objc1
  language_unknownObjectUnownedInit(&ref2, objc1);
  ASSERT_EQ(false, language_unknownObjectUnownedIsEqual(&ref2, language1));
  ASSERT_EQ(false, language_unknownObjectUnownedIsEqual(&ref2, language2));
  ASSERT_EQ(true,  language_unknownObjectUnownedIsEqual(&ref2, objc1));
  ASSERT_EQ(false, language_unknownObjectUnownedIsEqual(&ref2, objc2));

  // Deinit the assigned objects, invalidating ref1 and ref2
  language_release(language1);
  ASSERT_DEATH(language_unownedCheck(language1),
               "Attempted to read an unowned reference");
  ASSERT_EQ(0U, DestroyedObjCCount);
  language_unknownObjectRelease(objc1);
  ASSERT_EQ(1U, DestroyedObjCCount);

  // Unequal does not abort, even after invalidation
  // Equal but invalidated does abort (Codira)
  // Formerly equal but now invalidated returns unequal (ObjC)
  ASSERT_DEATH(language_unownedIsEqual(&ref1, language1),
               "Attempted to read an unowned reference");
  ASSERT_EQ(false, language_unownedIsEqual(&ref1, language2));
  ASSERT_DEATH(language_unknownObjectUnownedIsEqual(&ref1, language1),
               "Attempted to read an unowned reference");
  ASSERT_EQ(false, language_unknownObjectUnownedIsEqual(&ref1, language2));
  ASSERT_EQ(false, language_unknownObjectUnownedIsEqual(&ref1, objc1));
  ASSERT_EQ(false, language_unknownObjectUnownedIsEqual(&ref1, objc2));

  ASSERT_EQ(false, language_unknownObjectUnownedIsEqual(&ref2, language1));
  ASSERT_EQ(false, language_unknownObjectUnownedIsEqual(&ref2, language2));
  ASSERT_EQ(false, language_unknownObjectUnownedIsEqual(&ref2, objc1));
  ASSERT_EQ(false, language_unknownObjectUnownedIsEqual(&ref2, objc2));

  language_release(language2);
  language_unknownObjectRelease(objc2);

  language_unownedDestroy(&ref1);
  language_unknownObjectUnownedDestroy(&ref2);
}

TEST(WeakTest, unknownWeak) {
  void *objc1 = make_objc_object();
  HeapObject *language1 = make_language_object();

  WeakReference ref1;
  auto res = language_unknownObjectWeakInit(&ref1, objc1);
  ASSERT_EQ(&ref1, res);

  WeakReference ref2;
  res = language_unknownObjectWeakCopyInit(&ref2, &ref1);
  ASSERT_EQ(&ref2, res);

  WeakReference ref3; // ref2 dead.
  res = language_unknownObjectWeakTakeInit(&ref3, &ref2);
  ASSERT_EQ(&ref3, res);

  res = language_unknownObjectWeakAssign(&ref3, language1);
  ASSERT_EQ(&ref3, res);

  res = language_unknownObjectWeakCopyAssign(&ref3, &ref1);
  ASSERT_EQ(&ref3, res);

  res = language_unknownObjectWeakTakeAssign(&ref3, &ref1);
  ASSERT_EQ(&ref3, res);

  language_unknownObjectWeakDestroy(&ref3);

  language_release(language1);
  language_unknownObjectRelease(objc1);
}
