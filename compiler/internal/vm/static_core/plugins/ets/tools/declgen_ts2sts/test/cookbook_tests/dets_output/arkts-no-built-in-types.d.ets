'use static'
/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
export declare let sym1: Any;
export declare const sym2: Any;
export declare const Direction: Any;
export declare const map: Map<Any, string>;
export declare const sym5: Any;
export declare function getSymbol(): Any;
export declare const mySym: Any;
export declare function printSymbol(value: Any): void;
export type SymbolAlias = Any;
export declare const sym6: SymbolAlias;
export declare const obj3: Any;
export declare let a: Any;
export declare let buffers: Any[];
export declare function processData(buffer: Any): Any;
export type BufferType = Any;
export type ComplexType = Any | string;
export declare function A(): Any;
export declare const propDecorator1: Any;
export declare const propDecorator2: Any;
export declare const PropertyDecorators: Any;
export declare const decoratorMap: Map<string, Any>;
export declare function getPropertyDecorator(name: string): Any;
export declare function processProperty(decorator: Any, target: Any, propertyKey: string | Any): void;
export type PropertyDecoratorAlias = Any;
export declare const aliasedDecorator: PropertyDecoratorAlias;
export type ComplexDecoratorType = Any;
export declare const complexDecorator: ComplexDecoratorType;
export declare class DataProcessor {
    constructor(buffer: Any, sharedBuffer: Any);
    sharedBuffer: Any;
}
export declare class DecoratedClass {
    normalProperty: string;
    readOnlyProperty: number;
    customDecoratedProperty: boolean;
}
export declare class DecoratorManager implements DecoratorContainer {
    decorator: Any;
    constructor(decorator: Any);
    applyDecorator(target: Any, propertyKey: string | Any): void;
}
export declare interface BufferHolder {
    buffer: Any;
    getBuffer(): Any;
}
export declare interface DecoratorContainer {
    decorator: Any;
    applyDecorator(target: Any, propertyKey: string | Any): void;
}
