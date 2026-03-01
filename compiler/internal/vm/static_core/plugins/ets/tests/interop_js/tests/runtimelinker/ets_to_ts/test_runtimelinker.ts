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

const etsVm = globalThis.gtest.etsVm;

let A = etsVm.getClass('Lruntimelinker1/A;');
ASSERT_TRUE(new A().a === 1);

let b = etsVm.getClass('Lruntimelinker2/ETSGLOBAL;').b;
ASSERT_TRUE(b === 2);

let arr = etsVm.getClass('Lruntimelinker3/ETSGLOBAL;').arr;
ASSERT_TRUE(arr.length === 4);

let D = etsVm.getClass('Lruntimelinker4/D;');
ASSERT_TRUE(new D().a === 1);
ASSERT_TRUE(new D().d === 4);
