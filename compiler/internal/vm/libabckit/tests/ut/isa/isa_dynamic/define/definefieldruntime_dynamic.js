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
let map = {
    'key1': 'val1',
    'key2': 'val2'
};
let arr = [1, 2];

// CC-OFFNXT(G.OBJ.04) for test purposes
print(map['key3']); // eslint-disable-line dot-notation
// CC-OFFNXT(G.OBJ.04) for test purposes
// CC-OFFNXT(G.TYP.07) for test purposes
print(arr['key3']); // eslint-disable-line dot-notation
print(map[4]);
print(arr[4]);
