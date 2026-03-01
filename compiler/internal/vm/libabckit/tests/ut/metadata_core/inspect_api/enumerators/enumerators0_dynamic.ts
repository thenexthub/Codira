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
function m0F0(): void {
    ((): void => {
        console.log(1);
    })();
    ((): void => {
        console.log(2);
    })();
    class M0F0C0 {
        M0F0C0F0(): void {}
    }
}

function* m0F1(): Iterable<void> {}
async function m0F2(): Promise<void> {}
class M0C0 {
    M0C0F0(): void {}
    static M0C0F1(): void {}
    async M0C0F2(): Promise<void> {}
    M0C0F3 = (): void => {};
    // async *M0C0F3(){}
}
enum M0E0 {}

namespace M0N0 {
    function m0N0F0(): void {}

    function* m0N0F1(): Iterable<void> {}
    async function m0N0F2(): Promise<void> {}
    class M0N0C0 {
        M0N0C0F0(): void {}
        static M0N0C0F1(): void {}
        async M0N0C0F2(): Promise<void> {}
        // async *M0N0C0F3(){}
    }
    enum M0N0E0 {}

    namespace M0N0N0 {
        function m0N0N0F0(): void {}
    }

    namespace A {
        namespace B {
            class C {
                D(): void {
                    let E = (): void => {
                        let F = (): void => {};
                    };
                    E = (): void => {
                    let G = (): void => {};
                    };
                    ((): void => {(): void => {
                        let H = (): void => {};
                    }});
                }
            }

            function I(): void {}
        }
    }
}
