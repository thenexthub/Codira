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

@interface ClassAuthor {
    authorName: string
}

@interface deprecated{
    fromVersion: string
}

@interface ClassPreamble {
    authorName: string
    revision: number = 1
    reviewers: string[]
}

@ClassPreamble(
 {authorName: 'Alice',
 reviewers: ['Bob', 'Clara']}
 )
 class C3 {
    @deprecated({fromVersion: '5.18'})
    goo() {}

    @deprecated('5.18')
    foo() {}

    @ClassAuthor({authorName: 'John Doe'})
    unusedFunction(): void {
        // CC-OFFNXT(G.AOD.03) for test purposes
        console.log('This function is never called');
    }
 }

@ClassAuthor({authorName: 'John Doe'})
export function main(){
    // CC-OFFNXT(G.AOD.03) for test purposes
    console.log('Starting the application');
}