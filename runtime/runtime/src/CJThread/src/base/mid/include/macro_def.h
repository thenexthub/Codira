/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */


#ifndef MRT_MARCO_DEF_H
#define MRT_MARCO_DEF_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* x86 */
#ifndef MRT_X86
#define MRT_X86 0
#endif

/* arm */
#ifndef MRT_ARM
#define MRT_ARM 1
#endif

/* windows x86 */
#ifndef MRT_WINDOWS_X86
#define MRT_WINDOWS_X86 2
#endif

/* INLINE is used for include file
 * STATIC_INLINE is used for cpp file
 */
#if defined(MRT_DEBUG) && (MRT_DEBUG == 1)
#define MRT_INLINE inline
#define MRT_STATIC_INLINE
#else
#define MRT_INLINE inline __attribute__((always_inline))
#define MRT_STATIC_INLINE static inline  __attribute__((always_inline))
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* MRT_MARCO_DEF_H */
