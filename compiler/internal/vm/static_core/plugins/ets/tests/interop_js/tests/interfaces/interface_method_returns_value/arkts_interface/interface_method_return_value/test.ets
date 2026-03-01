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
package interface_method_return_value

/**
 * Interfaces for particular parameters
 */

interface IReturnLiteral{
    getLiteral(): number
}

interface IReturnBoolean{
    getBoolean(): boolean
}

interface IReturnArray{
    getArray(): FixedArray<number>
}

interface IReturnUnion {
    getUnion(): int | boolean | string
}

interface ILiteralProperty{
    someProp: number
}

interface IBooleanProperty{
    booleanProperty: boolean
}

interface IMappedProperty{
    mappedProperty: Record<number, string>
}

interface IReturnRecord {
    getRecord(): Record<number, Record<number, string>>
}

/**
 * Merged interface
 */

interface ISuperInterface extends IReturnLiteral,
                                    IReturnArray,
                                    IReturnBoolean,
                                    ILiteralProperty,
                                    IBooleanProperty,
                                    IReturnRecord { };


function helperMakeNestedRecord(): Record<number, Record<number, string>> {
    const childRef: Record<number, string> = { 0: "child_0", 1: "child_1", 100: "child_100"}
    const parentRef: Record<number, Record<number, string>> = { 0: childRef }
    return parentRef;
}

/**
 * Interface implementations as separate classes to extend from
 */

class TestClass implements ISuperInterface {
    literalProperty: number = 1234567980
    someProp: number = 1234567980
    booleanProperty: boolean = true
    mappedProperty: Record<number, string> = { 1: 'one', 2: 'two', 3: 'three'}
    nestedRecordProperty = helperMakeNestedRecord();

    getLiteral(): number {
        return 1234567890
    }
    getBoolean(): boolean {
        return false;
    }
    getArray(): FixedArray<number> {
        const returnedArray: FixedArray<number> = [1,2,3,4,5,6,8,9,0]
        return returnedArray
    }

    loopback(): TestClass {
        return this;
    }

    getRecord(): Record<number, Record<number, string>> {
        const childRef: Record<number, string> = { 1: "child_1", 2: "child_2", 100: "child_100"}
        const parentRef: Record<number, Record<number, string>> = { 0: childRef }
        return parentRef;
    }

    methodNotDeclaredInInterface(){
        return 'noninterface'
    }
}

class ReturnsLiteral implements IReturnLiteral {
    getLiteral(): number {
        return 1234567890
    }
}

class ReturnsBoolean implements IReturnBoolean {
    getBoolean(): boolean {
        return false;
    }
}

class ReturnsUnion implements IReturnUnion {
    getUnion(): boolean | int | string {
        const seed = Math.random();
        if (seed > 2/3) return 'stringValue';
        return seed > 1/3 ? 1234 : true;
    }
}

// class ReturnsRef implements IReturnRef {
//     getNumber(): number {
//         return 1234567890
//     }
//     getRef() {
//         return this.getNumber
//     }
// }

class ReturnsArray implements IReturnArray {
    getArray(): FixedArray<number> {
        const returnedArray: FixedArray<number> = [1,2,3,4,5,6,8,9,0]
        return returnedArray
    }
}


class HasLiteralProperty implements ILiteralProperty{
    public someProp: number = 1234567980
}

class HasBooleanProperty implements IBooleanProperty{
    booleanProperty: boolean = true
}

// class HasrefProperty implements IRefProperty{
//     refProperty: Function<string>
// }

class HasMappedProperty implements IMappedProperty{
    public mappedProperty: Record<number, string> = { 1: 'one', 2: 'two', 3: 'three'}
}

/**
 * object getters
 */
class InstanceClass extends ReturnsArray implements IMappedProperty, ILiteralProperty {
    mappedProperty: Record<number, string> = { 1: 'one', 2: 'two', 3: 'three'}
    literalProperty: number = 123;
    someProp: number = 123;
    privProp(){
        return 'prop'
    }
}

function getBoolValue(): IReturnBoolean {
    return new ReturnsBoolean()
}

function getArrayValue(): IReturnArray {
    return new ReturnsArray()
}

function getRefValue(): ISuperInterface {
    return new TestClass();
}
class UnionLocalClass extends ReturnsUnion implements IReturnUnion{

    };
function getUnion(): IReturnUnion {
    
    return (new UnionLocalClass())
}
