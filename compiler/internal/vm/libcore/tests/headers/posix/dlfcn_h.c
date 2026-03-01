// Copyright (C) 2017 The Android Open Source Project
// SPDX-License-Identifier: BSD-2-Clause

#include <dlfcn.h>

#include "header_checks.h"

static void dlfcn_h() {
  MACRO(RTLD_LAZY);
  MACRO(RTLD_NOW);
  MACRO(RTLD_GLOBAL);
  MACRO(RTLD_LOCAL);

  // POSIX accidentally standardized `Dl_info_t`,
  // rather than the `Dl_info` that everyone was using.
  // See https://www.austingroupbugs.net/view.php?id=1847.
#if !defined(__GLIBC__)
  // Our glibc does have the old name,
  // but only hidden behind _GNU_SOURCE as an extension.
  TYPE(Dl_info);
  STRUCT_MEMBER(Dl_info, const char*, dli_fname);
  STRUCT_MEMBER(Dl_info, void*, dli_fbase);
  STRUCT_MEMBER(Dl_info, const char*, dli_sname);
  STRUCT_MEMBER(Dl_info, void*, dli_saddr);
#endif
#if !defined(__GLIBC__)
  // Our glibc is too old for the new name.
  TYPE(Dl_info_t);
  STRUCT_MEMBER(Dl_info_t, const char*, dli_fname);
  STRUCT_MEMBER(Dl_info_t, void*, dli_fbase);
  STRUCT_MEMBER(Dl_info_t, const char*, dli_sname);
  STRUCT_MEMBER(Dl_info_t, void*, dli_saddr);
#endif

#if !defined(__GLIBC__)  // Our glibc is too old.
  FUNCTION(dladdr, int (*f)(const void*, Dl_info*));
#endif
  FUNCTION(dlclose, int (*f)(void*));
  FUNCTION(dlerror, char* (*f)(void));
  FUNCTION(dlopen, void* (*f)(const char*, int));
  FUNCTION(dlsym, void* (*f)(void*, const char*));
}
