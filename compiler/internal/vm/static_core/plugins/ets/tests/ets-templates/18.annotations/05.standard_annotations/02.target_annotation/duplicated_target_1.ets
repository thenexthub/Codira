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
desc: Target is a standard meta-annotation that is used to annotate a declaration of another annotation. A compile-time
       error occurs if @Target is used elsewhere.
tags: [compile-only, negative]
---*/

@Target({targets: [AnnotationTargets.FUNCTION, AnnotationTargets.FUNCTION]})
@interface MyAnnotationAnnotatedInterface {}


