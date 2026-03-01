/*---
Copyright (c) 2025-2026 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/

/*---
desc: ->
    The current compiler reserves the keyword struct because it is used in legacy ArkUI code. 
    This keyword can be used as a replacement for the keyword class in Class Declarations. 
    Class declarations marked with the keyword struct are processed by the ArkUI plugin and replaced with class declarations that use specific ArkUI types.
expected_out: "Semantic error ESE0364: Structs are only used to define UI components, it should be translated at 'plugin after parser' phase."
tags: [negative, compile-only]
---*/

struct A {}
