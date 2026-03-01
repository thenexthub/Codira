/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
 desc: 13.03 Import Directives
 name: 13.compilation_units/13.03.import_directives/13.04.06.type_binding/type_binding_03
 files: ["./to_export.ets"]
 tags:
 ---*/

import {StandardExportedClass as GIT1, ClassForExport as GIT2} from "./to_export.ets"

function main () {
  let c3 = new GIT1 // OK, `Module initialization code` be printed
}
