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

export let sym1: Symbol = Symbol("111");

export const sym2: Symbol = Symbol("description1");

export const Direction = {
  Up: Symbol("Up"),
  Down: Symbol("Down"),
  Left: Symbol("Left"),
  Right: Symbol("Right")
};

export const map = new Map<Symbol, string>();

export const sym5: Symbol = Symbol("key");
export function getSymbol(): Symbol {
  return sym5;
}
export const mySym = getSymbol();

export function printSymbol(value: Symbol): void {
  console.log(value);
}

export type SymbolAlias = symbol;
export const sym6: SymbolAlias = Symbol("description");

export const obj3: { [key: symbol]: string } = {};

export let a: SharedArrayBuffer;
export let buffers: SharedArrayBuffer[];

export function processData(buffer: SharedArrayBuffer): SharedArrayBuffer {
   return buffer; 
}

export class DataProcessor {
    constructor(buffer: SharedArrayBuffer, sharedBuffer: SharedArrayBuffer) {
        this.buffer = buffer;
        this.sharedBuffer = sharedBuffer;
    }
    private buffer: SharedArrayBuffer;
    public sharedBuffer: SharedArrayBuffer = new SharedArrayBuffer(1024);
}

export interface BufferHolder {
    buffer: SharedArrayBuffer;
    getBuffer(): SharedArrayBuffer;
}

export type BufferType = SharedArrayBuffer;
export type ComplexType = SharedArrayBuffer | string;

export function A(): PropertyDecorator {
    return (_target: Object, _propertyKey: string | symbol)=> {
        
    }
};

export const propDecorator1: PropertyDecorator = (_target: Object, _propertyKey: string | symbol) => {};

export const propDecorator2: PropertyDecorator = (_target: Object, _propertyKey: string | symbol) => {};

export const PropertyDecorators = {
    Readonly: ((_target: Object, propertyKey: string | symbol) => {}) as PropertyDecorator,
    Loggable: ((_target: Object, propertyKey: string | symbol) => {}) as PropertyDecorator
};

export const decoratorMap = new Map<string, PropertyDecorator>();
decoratorMap.set("timestamp", ((_target, propertyKey) => {}) as PropertyDecorator);
decoratorMap.set("validate", ((_target, propertyKey) => {}) as PropertyDecorator);

export function getPropertyDecorator(name: string): PropertyDecorator {
    return ((_target: Object, propertyKey: string | symbol) => {}) as PropertyDecorator;
}

export function processProperty(decorator: PropertyDecorator, target: any, propertyKey: string | symbol): void {
    decorator(target, propertyKey);
}

export class DecoratedClass {
    @propDecorator1
    public normalProperty: string = "value";
    
    @(PropertyDecorators.Readonly as PropertyDecorator)
    public readOnlyProperty: number = 42;
    
    @getPropertyDecorator("custom")
    public customDecoratedProperty: boolean = true;
}

export type PropertyDecoratorAlias = PropertyDecorator;
export const aliasedDecorator: PropertyDecoratorAlias = ((_target, propertyKey) => {}) as PropertyDecorator;

export type ComplexDecoratorType = PropertyDecorator;
export const complexDecorator: ComplexDecoratorType = ((_target, propertyKey) => {}) as PropertyDecorator;

export interface DecoratorContainer {
    decorator: PropertyDecorator;
    applyDecorator(target: any, propertyKey: string | symbol): void;
}

export class DecoratorManager implements DecoratorContainer {
    public decorator: PropertyDecorator;
    
    constructor(decorator: PropertyDecorator) {
        this.decorator = decorator;
    }
    
    public applyDecorator(target: any, propertyKey: string | symbol): void {
        this.decorator(target, propertyKey);
    }
}