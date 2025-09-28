/*
 *
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 * Date: Tuesday, June 11, 2024.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 *
 */

#ifndef __CCCL_SEQUENCE_ACCESS_H
#define __CCCL_SEQUENCE_ACCESS_H

#include <uscl/std/__cccl/compiler.h>
#include <uscl/std/__cccl/system_header.h>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

// We need to define hidden friends for {cr,r,}{begin,end} of our containers as we will otherwise encounter ambigouities
#define _CCCL_SYNTHESIZE_SEQUENCE_ACCESS(_ClassName, _ConstIter)                                                       \
  [[nodiscard]] _CCCL_HOST_DEVICE friend iterator begin(_ClassName& __sequence) noexcept(noexcept(__sequence.begin())) \
  {                                                                                                                    \
    return __sequence.begin();                                                                                         \
  }                                                                                                                    \
  [[nodiscard]] _CCCL_HOST_DEVICE friend _ConstIter begin(const _ClassName& __sequence) noexcept(                      \
    noexcept(__sequence.begin()))                                                                                      \
  {                                                                                                                    \
    return __sequence.begin();                                                                                         \
  }                                                                                                                    \
  [[nodiscard]] _CCCL_HOST_DEVICE friend iterator end(_ClassName& __sequence) noexcept(noexcept(__sequence.end()))     \
  {                                                                                                                    \
    return __sequence.end();                                                                                           \
  }                                                                                                                    \
  [[nodiscard]] _CCCL_HOST_DEVICE friend _ConstIter end(const _ClassName& __sequence) noexcept(                        \
    noexcept(__sequence.end()))                                                                                        \
  {                                                                                                                    \
    return __sequence.end();                                                                                           \
  }                                                                                                                    \
  [[nodiscard]] _CCCL_HOST_DEVICE friend _ConstIter cbegin(const _ClassName& __sequence) noexcept(                     \
    noexcept(__sequence.begin()))                                                                                      \
  {                                                                                                                    \
    return __sequence.begin();                                                                                         \
  }                                                                                                                    \
  [[nodiscard]] _CCCL_HOST_DEVICE friend _ConstIter cend(const _ClassName& __sequence) noexcept(                       \
    noexcept(__sequence.end()))                                                                                        \
  {                                                                                                                    \
    return __sequence.end();                                                                                           \
  }
#define _CCCL_SYNTHESIZE_SEQUENCE_REVERSE_ACCESS(_ClassName, _ConstRevIter)                            \
  [[nodiscard]] _CCCL_HOST_DEVICE friend reverse_iterator rbegin(_ClassName& __sequence) noexcept(     \
    noexcept(__sequence.rbegin()))                                                                     \
  {                                                                                                    \
    return __sequence.rbegin();                                                                        \
  }                                                                                                    \
  [[nodiscard]] _CCCL_HOST_DEVICE friend _ConstRevIter rbegin(const _ClassName& __sequence) noexcept(  \
    noexcept(__sequence.rbegin()))                                                                     \
  {                                                                                                    \
    return __sequence.rbegin();                                                                        \
  }                                                                                                    \
  [[nodiscard]] _CCCL_HOST_DEVICE friend reverse_iterator rend(_ClassName& __sequence) noexcept(       \
    noexcept(__sequence.rend()))                                                                       \
  {                                                                                                    \
    return __sequence.rend();                                                                          \
  }                                                                                                    \
  [[nodiscard]] _CCCL_HOST_DEVICE friend _ConstRevIter rend(const _ClassName& __sequence) noexcept(    \
    noexcept(__sequence.rend()))                                                                       \
  {                                                                                                    \
    return __sequence.rend();                                                                          \
  }                                                                                                    \
  [[nodiscard]] _CCCL_HOST_DEVICE friend _ConstRevIter crbegin(const _ClassName& __sequence) noexcept( \
    noexcept(__sequence.rbegin()))                                                                     \
  {                                                                                                    \
    return __sequence.rbegin();                                                                        \
  }                                                                                                    \
  [[nodiscard]] _CCCL_HOST_DEVICE friend _ConstRevIter crend(const _ClassName& __sequence) noexcept(   \
    noexcept(__sequence.rend()))                                                                       \
  {                                                                                                    \
    return __sequence.rend();                                                                          \
  }

#endif // __CCCL_SEQUENCE_ACCESS_H
