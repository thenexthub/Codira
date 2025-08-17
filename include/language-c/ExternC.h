/*===- clang-c/ExternC.h - Wrapper for 'extern "C"' ---------------*- C -*-===*\
|*                                                                            *|
|* Part of the LLVM Project, under the Apache License v2.0 with LLVM          *|
|* Exceptions.                                                                *|
|* See https://toolchain.org/LICENSE.txt for license information.                  *|
|* SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception                    *|
|*                                                                            *|
|*===----------------------------------------------------------------------===*|
|*                                                                            *|
|* This file defines an 'extern "C"' wrapper.                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/

#ifndef LANGUAGE_CORE_C_EXTERN_C_H
#define LANGUAGE_CORE_C_EXTERN_C_H

#ifdef __clang__
#define LANGUAGE_CORE_C_STRICT_PROTOTYPES_BEGIN                                   \
  _Pragma("clang diagnostic push")                                             \
      _Pragma("clang diagnostic error \"-Wstrict-prototypes\"")
#define LANGUAGE_CORE_C_STRICT_PROTOTYPES_END _Pragma("clang diagnostic pop")
#else
#define LANGUAGE_CORE_C_STRICT_PROTOTYPES_BEGIN
#define LANGUAGE_CORE_C_STRICT_PROTOTYPES_END
#endif

#ifdef __cplusplus
#define LANGUAGE_CORE_C_EXTERN_C_BEGIN                                            \
  extern "C" {                                                                 \
  LANGUAGE_CORE_C_STRICT_PROTOTYPES_BEGIN
#define LANGUAGE_CORE_C_EXTERN_C_END                                              \
  LANGUAGE_CORE_C_STRICT_PROTOTYPES_END                                           \
  }
#else
#define LANGUAGE_CORE_C_EXTERN_C_BEGIN LANGUAGE_CORE_C_STRICT_PROTOTYPES_BEGIN
#define LANGUAGE_CORE_C_EXTERN_C_END LANGUAGE_CORE_C_STRICT_PROTOTYPES_END
#endif

#endif
