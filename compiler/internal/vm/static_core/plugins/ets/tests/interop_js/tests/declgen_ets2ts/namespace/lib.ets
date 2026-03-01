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

export namespace A {
  export let v1 = 1;
  export namespace B {
    export let v2 = 2;
    export namespace C {
      export let v3 = 3;
    }
  }
}

export declare namespace X1 {
    function foo1(): void;
}

export namespace X {
  export function foo(): void { console.log("Hi") }
}

export namespace myNamespace {
  export enum nsEnum { White, Blue = 2, Red = 2 }
}

export namespace BarNameSpace {
    export type Callback = (arg: string) => void

    export namespace FooNameSpace {
        export type Callback2 = (arg: string) => void
    }
}

export function foo(arg: BarNameSpace.Callback): void {
    arg('hello');
}

export function bar(arg: BarNameSpace.FooNameSpace.Callback2): void {
    arg('goodbye');
}

export namespace MySpace {
    // 1. Variable definition and assignment
    export let count = 123;
    export let message = "Initial message";

    // 2. Function definition
    export function resetMessage() {
        message = "Message has been reset";
    }

    // 3. Interface definition
    export interface IShape {
        area(): number;
    }

    // 4. Class definition implementing the interface
    export class Rectangle implements IShape {
        width: number;
        height: number;

        constructor(width: number, height: number) {
            this.width = width;
            this.height = height;
        }

        area(): number {
            return this.width * this.height;
        }
    }

    // 5. Enum definition
    export enum Color {
        RED,
        GREEN,
        BLUE
    }

    // 6. Annotations definition
    export @interface Anno {
        color: Color
    }

    // Reset function to initialize the variables to their default values
    export function resetState() {
        count = 123;
        message = "Initial message";
    }

}

export let rectangleShape: MySpace.IShape = new MySpace.Rectangle(5, 10);

export namespace N {
    export class NA {}
    export type NB = (arg: string) => void
    export namespace NN {
        export class NNA {}
        export namespace NNN {
            export class NNNA {}
        }
    }
}

export let a: N.NA
export let b: N.NN.NNA
export let c: N.NN.NNN.NNNA
export function foo1(arg: N.NA) {}
export function foo2(arg: N.NN.NNA) {}
export function foo3(arg: N.NN.NNN.NNNA):N.NN.NNN.NNNA {
    return new N.NN.NNN.NNNA;
}