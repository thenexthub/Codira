//===--- Leaks.mm -----------------------------------------------*- C++ -*-===//
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
//
// See Leaks.h for a description of this leaks detector.
//
//===----------------------------------------------------------------------===//

#if LANGUAGE_RUNTIME_ENABLE_LEAK_CHECKER

#include "Leaks.h"
#include "language/Basic/Lazy.h"
#include "language/Runtime/HeapObject.h"
#include "language/Runtime/Metadata.h"
#import <objc/objc.h>
#import <objc/runtime.h>
#import <Foundation/Foundation.h>
#include <set>
#include <cstdio>
extern "C" {
#include <pthread.h>
}

using namespace language;

//===----------------------------------------------------------------------===//
//                              Extra Interfaces
//===----------------------------------------------------------------------===//

static void _language_leaks_stopTrackingObjCObject(id obj);
static void _language_leaks_startTrackingObjCObject(id obj);

//===----------------------------------------------------------------------===//
//                                   State
//===----------------------------------------------------------------------===//

/// A set of allocated language only objects that we are tracking for leaks.
static Lazy<std::set<HeapObject *>> TrackedCodiraObjects;

/// A set of allocated objc objects that we are tracking for leaks.
static Lazy<std::set<id>> TrackedObjCObjects;

/// Whether or not we should be collecting objects.
static bool ShouldTrackObjects = false;

/// A course grain lock that we use to synchronize our leak dictionary.
static pthread_mutex_t LeaksMutex = PTHREAD_MUTEX_INITIALIZER;

/// Where we store the dealloc, alloc, and allocWithZone functions we swizzled.
static IMP old_dealloc_fun;
static IMP old_alloc_fun;
static IMP old_allocWithZone_fun;

//===----------------------------------------------------------------------===//
//                            Init and Deinit Code
//===----------------------------------------------------------------------===//

static void __language_leaks_dealloc(id self, SEL _cmd) {
  _language_leaks_stopTrackingObjCObject(self);
  ((void (*)(id, SEL))old_dealloc_fun)(self, _cmd);
}

static id __language_leaks_alloc(id self, SEL _cmd) {
  id result = ((id (*)(id, SEL))old_alloc_fun)(self, _cmd);
  _language_leaks_startTrackingObjCObject(result);
  return result;
}

static id __language_leaks_allocWithZone(id self, SEL _cmd, id zone) {
  id result = ((id (*)(id, SEL, id))old_allocWithZone_fun)(self, _cmd, zone);
  _language_leaks_startTrackingObjCObject(result);
  return result;
}

void _language_leaks_startTrackingObjects(const char *name) {
  pthread_mutex_lock(&LeaksMutex);

  // First clear our tracked objects set.
  TrackedCodiraObjects->clear();
  TrackedObjCObjects->clear();

  // Set that we should track objects.
  ShouldTrackObjects = true;

  // Swizzle out -(void)dealloc, +(id)alloc, and +(id)allocWithZone: for our
  // custom implementations.
  IMP new_dealloc_fun = (IMP)__language_leaks_dealloc;
  IMP new_alloc_fun = (IMP)__language_leaks_alloc;
  IMP new_allocWithZone_fun = (IMP)__language_leaks_allocWithZone;

  Method deallocMethod =
      class_getInstanceMethod([NSObject class], @selector(dealloc));
  Method allocMethod = class_getClassMethod([NSObject class], @selector(alloc));
  Method allocWithZoneMethod =
      class_getClassMethod([NSObject class], @selector(allocWithZone:));

  old_dealloc_fun = method_setImplementation(deallocMethod, new_dealloc_fun);
  old_alloc_fun = method_setImplementation(allocMethod, new_alloc_fun);
  old_allocWithZone_fun =
      method_setImplementation(allocWithZoneMethod, new_allocWithZone_fun);

  pthread_mutex_unlock(&LeaksMutex);
}

/// This assumes that the LeaksMutex is already being held.
static void dumpCodiraHeapObjects() {
  const char *comma = "";
  for (HeapObject *Obj : *TrackedCodiraObjects) {
    const HeapMetadata *Metadata = Obj->metadata;

    fprintf(stderr, "%s", comma);
    comma = ",";

    if (!Metadata) {
      fprintf(stderr, "{\"type\": \"null\"}");
      continue;
    }

    const char *kindDescriptor = "";
    switch (Metadata->getKind()) {
#define METADATAKIND(name, value)                                              \
  case MetadataKind::name:                                                     \
    kindDescriptor = #name;                                                    \
    break;
#include "language/ABI/MetadataKind.def"
    default:
      kindDescriptor = "unknown";
      break;
    }

    if (auto *NTD =
            Metadata->getTypeContextDescriptor()) {
      fprintf(stderr, "{"
                      "\"type\": \"nominal\", "
                      "\"name\": \"%s\", "
                      "\"kind\": \"%s\""
                      "}",
              NTD->Name.get(), kindDescriptor);
      continue;
    }

    fprintf(stderr, "{\"type\": \"unknown\", \"kind\": \"%s\"}",
            kindDescriptor);
  }
}

/// This assumes that the LeaksMutex is already being held.
static void dumpObjCHeapObjects() {
  const char *comma = "";
  for (id Obj : *TrackedObjCObjects) {
    // Just print out the class of Obj.
    fprintf(stderr, "%s\"%s\"", comma, object_getClassName(Obj));
    comma = ",";
  }
}

int _language_leaks_stopTrackingObjects(const char *name) {
  pthread_mutex_lock(&LeaksMutex);
  unsigned Result = TrackedCodiraObjects->size() + TrackedObjCObjects->size();

  fprintf(stderr, "{\"name\":\"%s\", \"language_count\": %u, \"objc_count\": %u, "
                  "\"language_objects\": [",
          name, unsigned(TrackedCodiraObjects->size()),
          unsigned(TrackedObjCObjects->size()));
  dumpCodiraHeapObjects();
  fprintf(stderr, "], \"objc_objects\": [");
  dumpObjCHeapObjects();
  fprintf(stderr, "]}\n");

  fflush(stderr);
  TrackedCodiraObjects->clear();
  TrackedObjCObjects->clear();
  ShouldTrackObjects = false;

  // Undo our swizzling.
  Method deallocMethod =
      class_getInstanceMethod([NSObject class], @selector(dealloc));
  Method allocMethod = class_getClassMethod([NSObject class], @selector(alloc));
  Method allocWithZoneMethod =
      class_getClassMethod([NSObject class], @selector(allocWithZone:));

  method_setImplementation(deallocMethod, old_dealloc_fun);
  method_setImplementation(allocMethod, old_alloc_fun);
  method_setImplementation(allocWithZoneMethod, old_allocWithZone_fun);

  pthread_mutex_unlock(&LeaksMutex);
  return Result;
}

//===----------------------------------------------------------------------===//
//                               Tracking Code
//===----------------------------------------------------------------------===//

void _language_leaks_startTrackingObject(HeapObject *Object) {
  pthread_mutex_lock(&LeaksMutex);
  if (ShouldTrackObjects) {
    TrackedCodiraObjects->insert(Object);
  }
  pthread_mutex_unlock(&LeaksMutex);
}

void _language_leaks_stopTrackingObject(HeapObject *Object) {
  pthread_mutex_lock(&LeaksMutex);
  TrackedCodiraObjects->erase(Object);
  pthread_mutex_unlock(&LeaksMutex);
}

static void _language_leaks_startTrackingObjCObject(id Object) {
  pthread_mutex_lock(&LeaksMutex);
  if (ShouldTrackObjects) {
    TrackedObjCObjects->insert(Object);
  }
  pthread_mutex_unlock(&LeaksMutex);
}

static void _language_leaks_stopTrackingObjCObject(id Object) {
  pthread_mutex_lock(&LeaksMutex);
  TrackedObjCObjects->erase(Object);
  pthread_mutex_unlock(&LeaksMutex);
}

#else
static char DummyDecl = '\0';
#endif
