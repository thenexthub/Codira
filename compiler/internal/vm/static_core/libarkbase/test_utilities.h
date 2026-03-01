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

#ifndef LIBPANDABASE_TEST_UTILITIES_H
#define LIBPANDABASE_TEST_UTILITIES_H

#include <gtest/gtest.h>

// Death test under qemu is not compatible with 'threadsafe' death test style flag
// because in this mode after process clone() test is tried to be started from the very beginning
// via execv() with test binary and without qemu that cause 'exec format error'
// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#ifdef PANDA_QEMU_BUILD
#define DEATH_TEST_F(test_fixture, test_name) TEST_F(test_fixture, DISABLED_##test_name)
#define DEATH_TEST_P(test_fixture, test_name) TEST_P(test_fixture, DISABLED_##test_name)
#define DEATH_TEST(test_fixture, test_name) TEST(test_fixture, DISABLED_##test_name)
#else
#define DEATH_TEST_F(test_fixture, test_name) TEST_F(test_fixture, test_name)
#define DEATH_TEST_P(test_fixture, test_name) TEST_P(test_fixture, test_name)
#define DEATH_TEST(test_fixture, test_name) TEST(test_fixture, test_name)
#endif
// NOLINTEND(cppcoreguidelines-macro-usage)
#endif /* LIBPANDABASE_TEST_UTILITIES_H */