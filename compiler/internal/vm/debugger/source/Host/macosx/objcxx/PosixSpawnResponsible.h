//===-- PosixSpawnResponsible.h ---------------------------------*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//

#ifndef LLDB_HOST_POSIXSPAWNRESPONSIBLE_H
#define LLDB_HOST_POSIXSPAWNRESPONSIBLE_H

#include <spawn.h>

#if __has_include(<responsibility.h>)
#include <dispatch/dispatch.h>
#include <dlfcn.h>
#include <responsibility.h>

// Older SDKs  have responsibility.h but not this particular function. Let's
// include the prototype here.
errno_t responsibility_spawnattrs_setdisclaim(posix_spawnattr_t *attrs,
                                              bool disclaim);

#endif

static inline int setup_posix_spawn_responsible_flag(posix_spawnattr_t *attr) {
  if (@available(macOS 10.14, *)) {
#if __has_include(<responsibility.h>)
    static __typeof__(responsibility_spawnattrs_setdisclaim)
        *responsibility_spawnattrs_setdisclaim_ptr;
    static dispatch_once_t pred;
    dispatch_once(&pred, ^{
      responsibility_spawnattrs_setdisclaim_ptr =
          reinterpret_cast<__typeof__(&responsibility_spawnattrs_setdisclaim)>
          (dlsym(RTLD_DEFAULT, "responsibility_spawnattrs_setdisclaim"));
    });
    if (responsibility_spawnattrs_setdisclaim_ptr)
      return responsibility_spawnattrs_setdisclaim_ptr(attr, true);
#endif
  }
  return 0;
}

#endif // LLDB_HOST_POSIXSPAWNRESPONSIBLE_H
