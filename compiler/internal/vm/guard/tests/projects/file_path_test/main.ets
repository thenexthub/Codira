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

import { foo1 as A } from './A.ets';
import { foo1 as A1 } from './A1.ets';
import { foo1 as fooA } from './foo/A.ets';
import { foo1 as dirBarA } from './dir/bar/A.ets';
import { foo1 as dirBarFooA } from './dir/bar/foo/A.ets';
import { foo1 as dirFooA } from './dir/foo/A.ets';
import { foo1 as dirFooB } from './dir/foo/B.ets';

A(1, 2);
A1(1, 2);
fooA(1, 2);
dirBarA(1, 2);
dirBarFooA(1, 2);
dirFooA(1, 2);
dirFooB(1, 2);
