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
 desc: Issue 23623. Failed to extend TSQualifiedName
 tags: []

 ---*/

namespace shared {
    export class URIProvider {}
    export interface IURI {}
 }

namespace work {
    export class FileURI extends shared.URIProvider {}
    export class IURIImplementation implements shared.IURI {}
    export class TogetherURIImpl extends shared.URIProvider implements shared.IURI {}
}

function main() {
    const c1 = new work.FileURI
    const c2 = new work.IURIImplementation
    const c3 = new work.TogetherURIImpl
}
