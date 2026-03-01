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

arktest.assertEQ(boolean_foo(true), true);
arktest.assertEQ(byte_foo(127 as byte), 127);
arktest.assertEQ(short_foo(32767 as short), 32767);
arktest.assertEQ(int_foo(50000 as int), 50000);
arktest.assertEQ(long_foo(50000000 as long), 50000000);
arktest.assertEQ(double_foo(12345.54321 as double), 12345.54321);
arktest.assertEQ(float_foo(12345.5 as float), 12345.5 as float);
