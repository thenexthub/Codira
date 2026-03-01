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
 desc: 13.06 Top-level Declarations
 name: 13.compilation_units/13.06.top-level_declarations/13.06.01.exported_declaration/exported_declaration_04
 tags:
 files: [./to_export.ets]
 ---*/
import PI from "./to_export"

function main(): void {
  console.log(PI)
}
