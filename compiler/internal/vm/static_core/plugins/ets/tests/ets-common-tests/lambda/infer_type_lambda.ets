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

class Foo {
    public lambdaInferType() {
        let x = () => {
            return 1
        }
        return x
    }
    
    public lambdaInferTypeReturnLambda() {
        let x = () => () => {
           return 1
        }
        return x
    }

    public lambdaInferTypeReturnLambda1() {
        let x = () => {
            let y = () => {
                return 1
            }
            return y
        }
        return x
    }

    public lambdaInferTypeReturnLiteral() {
        let x = () => 1
        return x
    }

    public lambdaNestingReturnStatement() {
        return () => {
            return () => {
                return 1
            }
        }
    }
}

function main() {
    let foo = new Foo()

    let x = foo.lambdaInferType()()
    arktest.assertEQ(x, 1)

    let x1 = foo.lambdaInferTypeReturnLambda()()()
    arktest.assertEQ(x1, 1)

    let x2 = foo.lambdaInferTypeReturnLambda1()()()
    arktest.assertEQ(x2, 1)

    let x3 = foo.lambdaInferTypeReturnLiteral()()
    arktest.assertEQ(x3, 1)
    
    let x4 = foo.lambdaNestingReturnStatement()()()
    arktest.assertEQ(x4, 1)
}
