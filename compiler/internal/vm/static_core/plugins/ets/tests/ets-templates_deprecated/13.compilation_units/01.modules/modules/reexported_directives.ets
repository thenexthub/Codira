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

/*---
desc: >-
      Module exports top-level declaration - types, classes, interfaces, enums, variables (including const) and functions
      to reexport them.
tags: [compile-only, not-a-test]
files:
- ./declarations.ets
---*/

// classes, interfaces, enum
export {A as RA} from "./declarations.ets"
export {B as RB} from "./declarations.ets"
export {AC as RAC} from "./declarations.ets"
export {C as RC} from "./declarations.ets"
export {I as RI} from "./declarations.ets"
export {II as RII} from "./declarations.ets"
export {D as RD} from "./declarations.ets"
export {ENUM as RENUM} from "./declarations.ets"

// variables
export {a as ra} from "./declarations.ets"
export {i as ri} from "./declarations.ets"
export {n as rn} from "./declarations.ets"
export {s as rs} from "./declarations.ets"
export {c as rc} from "./declarations.ets"
export {u as ru} from "./declarations.ets"
export {arr as rarr} from "./declarations.ets"

// constants
export { ca as rca } from "./declarations.ets"
export { cs as rcs } from "./declarations.ets"
export { ci as rci } from "./declarations.ets"

// functions
export { NoArg as RNoArg } from "./declarations.ets"
export { OneArg as ROneArg } from "./declarations.ets"
export { TwoArgs as RTwoArgs } from "./declarations.ets"
export { Gen as RGen } from "./declarations.ets"
export { ArgFun as RArgFun } from "./declarations.ets"
export { ArgFun2 as RArgFun2 } from "./declarations.ets"

// types
export { CLASS_A as RCLASS_A } from "./declarations.ets"
export { VAR_i as RVAR_i } from "./declarations.ets"
export { TPL as RTPL } from "./declarations.ets"
