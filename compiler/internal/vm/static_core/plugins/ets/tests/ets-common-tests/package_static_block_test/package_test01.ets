/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*---
  files: [./package_with_static_block/package01.ets]
---*/

import {nn, NS, ME, bb, cc, ss} from "./package_with_static_block"
import {in_bb, in_cc, in_ss, in_nn} from "./package_with_static_block"
import {c_nn, c_bb, c_cc, c_ss} from "./package_with_static_block"
import {c_in_bb, c_in_cc, c_in_ss, c_in_nn} from "./package_with_static_block"

arktest.assertEQ(nn, 6)
arktest.assertEQ(NS.n, 66)
arktest.assertEQ(NS.NS1.n_sub, 666)
arktest.assertEQ(bb, true)
arktest.assertEQ(cc, c'c')
arktest.assertEQ(ss, "hello")

arktest.assertEQ(in_bb, true)
arktest.assertEQ(in_cc, c'c')
arktest.assertEQ(in_ss, "world")
arktest.assertEQ(in_nn, 666)

arktest.assertEQ(c_nn, 1)
arktest.assertEQ(c_bb, true)
arktest.assertEQ(c_cc, c'c')
arktest.assertEQ(c_ss, "hello")

arktest.assertEQ(c_in_bb, true)
arktest.assertEQ(c_in_cc, c'c')
arktest.assertEQ(c_in_ss, "world")
arktest.assertEQ(c_in_nn, 666)
