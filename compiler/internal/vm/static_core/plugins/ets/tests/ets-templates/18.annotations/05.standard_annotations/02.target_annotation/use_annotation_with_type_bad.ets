/*
* Copyright (c) 2025 Huawei Device Co., Ltd.
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
desc: If an annotation is used in front of type in parentheses, then the parentheses
      become a mandatory part of the annotation to prevent ambiguity.
tags: [compile-only, negative]
---*/

@Target([AnnotationTargets.TYPE])
@interface MyTypeAnnotation() {}

function main() {
    let x: @MyTypeAnnotation (string | null)
}
