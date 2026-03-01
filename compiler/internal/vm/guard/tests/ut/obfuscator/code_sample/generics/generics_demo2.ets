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

interface Issue {
    title: string
    description: string
}

function process(issue: Partial<Issue>) {
    // CC-OFFNXT(G.AOD.03) for test purposes
    console.log('Partial ', issue.title);
}
process({}) // description is undefined

class A {
    public f1: string = ''
    public f2: number = 1
    public f3: boolean = true
}
let varX = new A
let constY: Readonly<A> = varX
// CC-OFFNXT(G.AOD.03) for test purposes
console.log('Readonly', constY.f2)


let recordX: Record<string, number> = {
    'key1': 1,
    'key2': 2,
    'key3': 4,
}
// CC-OFFNXT(G.AOD.03) for test purposes
// CC-OFFNXT(G.OBJ.04) for test purposes
console.log('Record', recordX['key2'])
// CC-OFFNXT(G.OBJ.04) for test purposes
recordX['key2'] = 8
// CC-OFFNXT(G.AOD.03) for test purposes
// CC-OFFNXT(G.OBJ.04) for test purposes
console.log('Record', recordX['key2'])
// CC-OFFNXT(G.OBJ.04) for test purposes
recordX['key4'] = 10
// CC-OFFNXT(G.AOD.03) for test purposes
// CC-OFFNXT(G.OBJ.04) for test purposes
console.log('Record', recordX['key4'])

interface Issue2 {
    title?: string
}
const myIssue: Required<Readonly<Issue2>> = {
    title: 'One'
};
// CC-OFFNXT(G.AOD.03) for test purposes
console.log('Required-Readonly', myIssue.title)
