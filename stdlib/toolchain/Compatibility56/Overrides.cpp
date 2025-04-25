//===--- Overrides.cpp - Compat override table for Swift 5.6 runtime ------===//
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
//  This file provides compatibility override hooks for Swift 5.6 runtimes.
//
//===----------------------------------------------------------------------===//

#include "Overrides.h"

#include <dlfcn.h>
#include <mach-o/dyld.h>
#include <mach-o/getsect.h>

using namespace language;

__asm__ (".linker_option \"-lc++\"");

#define OVERRIDE(name, ret, attrs, ccAttrs, namespace, typedArgs, namedArgs) \
  Override_ ## name name;

struct RuntimeOverrideSection {
  uintptr_t version;
#include "CompatibilityOverrideRuntime.def"
};

struct ConcurrencyOverrideSection {
  uintptr_t version;
#include "CompatibilityOverrideConcurrency.def"
};

#undef OVERRIDE

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-designated-field-initializers"
__attribute__((visibility("hidden")))
ConcurrencyOverrideSection Swift56ConcurrencyOverrides
__attribute__((used, section("__DATA,__s_async_hook"))) = {
  .version = 0,
#if __POINTER_WIDTH__ == 64
  .task_create_common = swift56override_swift_task_create_common,
#endif
  .task_future_wait = swift56override_swift_task_future_wait,
  .task_future_wait_throwing = swift56override_swift_task_future_wait_throwing,
};

__attribute__((visibility("hidden")))
RuntimeOverrideSection Swift56RuntimeOverrides
__attribute__((used, section("__DATA,__swift56_hooks"))) = {
  .version = 0,
};
#pragma clang diagnostic pop

// Allow this library to get force-loaded by autolinking
__attribute__((weak, visibility("hidden")))
extern "C"
char _swift_FORCE_LOAD_$_swiftCompatibility56 = 0;
