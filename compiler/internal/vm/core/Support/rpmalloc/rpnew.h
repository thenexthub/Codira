//===-------------------------- rpnew.h -----------------*- C -*-=============//
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
//
// This library provides a cross-platform lock free thread caching malloc
// implementation in C11.
//
//===----------------------------------------------------------------------===//

#ifdef __cplusplus

#include <new>
#include <rpmalloc.h>

#ifndef __CRTDECL
#define __CRTDECL
#endif

extern void __CRTDECL operator delete(void *p) noexcept { rpfree(p); }

extern void __CRTDECL operator delete[](void *p) noexcept { rpfree(p); }

extern void *__CRTDECL operator new(std::size_t size) noexcept(false) {
  return rpmalloc(size);
}

extern void *__CRTDECL operator new[](std::size_t size) noexcept(false) {
  return rpmalloc(size);
}

extern void *__CRTDECL operator new(std::size_t size,
                                    const std::nothrow_t &tag) noexcept {
  (void)sizeof(tag);
  return rpmalloc(size);
}

extern void *__CRTDECL operator new[](std::size_t size,
                                      const std::nothrow_t &tag) noexcept {
  (void)sizeof(tag);
  return rpmalloc(size);
}

#if (__cplusplus >= 201402L || _MSC_VER >= 1916)

extern void __CRTDECL operator delete(void *p, std::size_t size) noexcept {
  (void)sizeof(size);
  rpfree(p);
}

extern void __CRTDECL operator delete[](void *p, std::size_t size) noexcept {
  (void)sizeof(size);
  rpfree(p);
}

#endif

#if (__cplusplus > 201402L || defined(__cpp_aligned_new))

extern void __CRTDECL operator delete(void *p,
                                      std::align_val_t align) noexcept {
  (void)sizeof(align);
  rpfree(p);
}

extern void __CRTDECL operator delete[](void *p,
                                        std::align_val_t align) noexcept {
  (void)sizeof(align);
  rpfree(p);
}

extern void __CRTDECL operator delete(void *p, std::size_t size,
                                      std::align_val_t align) noexcept {
  (void)sizeof(size);
  (void)sizeof(align);
  rpfree(p);
}

extern void __CRTDECL operator delete[](void *p, std::size_t size,
                                        std::align_val_t align) noexcept {
  (void)sizeof(size);
  (void)sizeof(align);
  rpfree(p);
}

extern void *__CRTDECL operator new(std::size_t size,
                                    std::align_val_t align) noexcept(false) {
  return rpaligned_alloc(static_cast<size_t>(align), size);
}

extern void *__CRTDECL operator new[](std::size_t size,
                                      std::align_val_t align) noexcept(false) {
  return rpaligned_alloc(static_cast<size_t>(align), size);
}

extern void *__CRTDECL operator new(std::size_t size, std::align_val_t align,
                                    const std::nothrow_t &tag) noexcept {
  (void)sizeof(tag);
  return rpaligned_alloc(static_cast<size_t>(align), size);
}

extern void *__CRTDECL operator new[](std::size_t size, std::align_val_t align,
                                      const std::nothrow_t &tag) noexcept {
  (void)sizeof(tag);
  return rpaligned_alloc(static_cast<size_t>(align), size);
}

#endif

#endif
