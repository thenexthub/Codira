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

#ifndef COMMON_COMPONENTS_BASE_ASAN_INTERFACE_H
#define COMMON_COMPONENTS_BASE_ASAN_INTERFACE_H

#if (defined(__has_feature) && __has_feature(address_sanitizer)) || defined(__SANITIZE_ADDRESS__)
#define HAS_SANITIZER
#endif

#if defined(HAS_SANITIZER) && defined(RUN_WITH_ASAN)
#define ARK_ASAN_ON
#endif

#ifdef ARK_ASAN_ON
extern "C" {
// NOLINTNEXTLINE(readability-identifier-naming)
void __asan_poison_memory_region(void const volatile *addr, size_t size) __attribute__((visibility("default")));
// NOLINTNEXTLINE(readability-identifier-naming)
void __asan_unpoison_memory_region(void const volatile *addr, size_t size) __attribute__((visibility("default")));
}

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#ifndef ASAN_POISON_MEMORY_REGION
#define ASAN_POISON_MEMORY_REGION(addr, size) __asan_poison_memory_region((addr), (size))
#endif
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#ifndef ASAN_UNPOISON_MEMORY_REGION
#define ASAN_UNPOISON_MEMORY_REGION(addr, size) __asan_unpoison_memory_region((addr), (size))
#endif
#else
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#ifdef ASAN_POISON_MEMORY_REGION
#undef ASAN_POISON_MEMORY_REGION
#endif
#define ASAN_POISON_MEMORY_REGION(addr, size) ((void)(addr), (void)(size))
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#ifdef ASAN_UNPOISON_MEMORY_REGION
#undef ASAN_UNPOISON_MEMORY_REGION
#endif
#define ASAN_UNPOISON_MEMORY_REGION(addr, size) ((void)(addr), (void)(size))
#endif

#endif  // COMMON_COMPONENTS_BASE_ASAN_INTERFACE_H

