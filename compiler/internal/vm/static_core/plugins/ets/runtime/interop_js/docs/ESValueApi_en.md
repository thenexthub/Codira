# ESValue
## Introduction
As a wrapper class, ESValue mainly provides a series of data interfaces that can operate on dynamically typed ArkTS-Dyn in statically typed ArkTS-Sta.
## Detailed Interfaces
### Undefined
`public static get Undefined(): ESValue`  
Get the ESValue object that stores the dynamic undefined value.

Return Value:

|    Type     |            Description            |
| :---------: | :------------------------: |
|   ESValue   | The ESValue object storing the dynamic undefined value |

Example:

```typescript
let undefinedVal = ESValue.Undefined;
let isUndefinedVal = undefinedVal.isUndefined(); // true
```
---
### Null
`public static get Null(): ESValue`  
Get the ESValue object that stores a dynamic null value.

Return Value:

| Type | Description |
| :---------: | :--------------------------------: |
| ESValue | The ESValue object storing the dynamic null value |

Example:

```typescript
let nullVal = ESValue.Null;
let isNullVal = nullVal.isNull(); // true
```
---
### wrapBoolean
`public static wrapBoolean(b: boolean): ESValue`  
Wrap a boolean value into an ESValue object.

Parameters:

| Parameter Name | Type | Required | Description |
| :------------: | :-----: | :------: | :---------------------: |
| b | boolean | Yes | The boolean value to be wrapped |

Return Value:

| Type | Description |
| :---------: | :----------------------------: |
| ESValue | The ESValue object storing the dynamic boolean value |

Example:

```typescript
let trueVal = ESValue.wrapBoolean(true);
let boolVal = trueVal.toBoolean(); // true
```

---
### wrapString
`public static wrapString(s: string): ESValue`  
Wrap a string into an ESValue object.

Parameters:

| Parameter Name | Type | Required | Description |
| :------------: | :----: | :------: | :-----------------: |
| s | string | Yes | The string object to be wrapped |

Return Value:

| Type | Description |
| :---------: | :----------------------------: |
| ESValue | The ESValue object storing the dynamic string value |

Example:

```typescript
let strWrap = ESValue.wrapString('Hello');
let strValue = strWrap.toString(); // 'Hello'
```

---
### wrapNumber
`public static wrapNumber(n: number): ESValue`  
Wrap a number into an ESValue object.

Parameters:

| Parameter Name | Type | Required | Description |
| :------------: | :----: | :------: | :-----------------: |
| n | number | Yes | The number object to be wrapped |

Return Value:

| Type | Description |
| :---------: | :----------------------------: |
| ESValue | The ESValue object storing the dynamic number value |

Example:

```typescript
let numWrap = ESValue.wrapNumber(3.1415);
let numValue = numWrap.toNumber(); // 3.1415
```

---
### wrapBigInt
`public static wrapBigInt(bi: bigint): ESValue`  
Wrap a big integer into an ESValue object.

Parameters:

| Parameter Name | Type | Required | Description |
| :------------: | :----: | :------: | :-----------------: |
| bi | bigint | Yes | The bigint object to be wrapped |

Return Value:

| Type | Description |
| :---------: | :------------------------------: |
| ESValue | The ESValue object storing the dynamic bigint value |

Example:

```typescript
let bigWrap = ESValue.wrapBigInt(9007199254740991n);
let bigValue = bigWrap.toBigInt(); // 9007199254740991n
```

---
### wrapByte
`public static wrapByte(b: byte): ESValue`  
Wrap a byte integer value into an ESValue object.

Parameters:

| Parameter Name | Type | Required | Description |
| :------------: | :----: | :------: | :-----------------------: |
| b | byte | Yes | The 8-bit integer to be wrapped (-128~127) |

Return Value:

| Type | Description |
| :---------: | :------------------------------: |
| ESValue | The ESValue object storing the dynamic byte value |

Example:

```typescript
let byteWrap = ESValue.wrapByte(100);
let byteValue = byteWrap.toByte(); // 100
```

---
### wrapShort
`public static wrapShort(s: short): ESValue`  
Wrap a short integer value into an ESValue object.

Parameters:

| Parameter Name | Type | Required | Description |
| :------------: | :----: | :------: | :-------------------------: |
| s | short | Yes | The 16-bit integer to be wrapped (-32768~32767) |

Return Value:

| Type | Description |
| :---------: | :------------------------------: |
| ESValue | The ESValue object storing the dynamic short value |

Example:

```typescript
let shortWrap = ESValue.wrapShort(20000);
let shortValue = shortWrap.toShort(); // 20000
```

---
### wrapInt
`public static wrapInt(i: int): ESValue`  
Wrap an integer value into an ESValue object.

Parameters:

| Parameter Name | Type | Required | Description |
| :------------: | :----: | :------: | :---------------------: |
| i | int | Yes | The 32-bit integer to be wrapped |

Return Value:

| Type | Description |
| :---------: | :------------------------------: |
| ESValue | The ESValue object storing the dynamic int value |

Example:

```typescript
let intWrap = ESValue.wrapInt(2147483647);
let intValue = intWrap.toInt(); // 2147483647
```

---
### wrapLong
`public static wrapLong(l: long): ESValue`  
Wrap a long integer value into an ESValue object.

Parameters:

| Parameter Name | Type | Required | Description |
| :------------: | :----: | :------: | :-------------------------: |
| l | long | Yes | The 64-bit integer to be wrapped |

Return Value:

| Type | Description |
| :---------: | :------------------------------: |
| ESValue | The ESValue object storing the dynamic long value |

Example:

```typescript
let longWrap = ESValue.wrapLong(9223372036854775807);
let longValue = longWrap.toLong(); // 9223372036854775807
```
---
### wrapLongLossy
`public static wrapLongLossy(l: long): ESValue`  
Wrap a long integer value into an ESValue object with lossy conversion.

Parameters:

| Parameter Name | Type | Required | Description |
| :------------: | :----: | :------: | :-----------------------: |
| l | long | Yes | The 64-bit integer to be wrapped |

Return Value:

| Type | Description |
| :---------: | :-------------------------------: |
| ESValue | The ESValue object storing the dynamic long value |

Example:

```typescript
let lossyWrap = ESValue.wrapLongLossy(Number.MAX_SAFE_INTEGER + 100);
let lossyValue = lossyWrap.isNumber(); // true
```

---
### wrapFloat
`public static wrapFloat(f: float): ESValue`  
Wrap a single-precision floating-point value into an ESValue object.

Parameters:

| Parameter Name | Type | Required | Description |
| :------------: | :----: | :------: | :-----------------------------: |
| f | float | Yes | The 32-bit floating-point value to be wrapped |

Return Value:

| Type | Description |
| :---------: | :------------------------------: |
| ESValue | The ESValue object storing the dynamic float value |

Example:

```typescript
let floatWrap = ESValue.wrapFloat(3.1415927);
floatValue = floatWrap.toFloat(); // 3.1415927
```

---
### wrapDouble
`public static wrapDouble(d: double): ESValue`  
Wrap a double-precision floating-point value into an ESValue object.

Parameters:

| Parameter Name | Type | Required | Description |
| :------------: | :----: | :------: | :---------------------------: |
| d | double | Yes | The 64-bit floating-point value to be wrapped |

Return Value:

| Type | Description |
| :---------: | :------------------------------: |
| ESValue | The ESValue object storing the dynamic double value |

Example:

```typescript
let doubleWrap = ESValue.wrapDouble(3.14159265358);
let doubleValue = doubleWrap.toDouble(); // 3.14159265358
```

---
### unwrap
`public unwrap(): Any`  
Unwrap ESValue into the original value type.

Return Value:

| Type | Description |
| :----: | :-----------------------: |
| Any | The original object saved by ESValue |

Example:

```typescript
let numVal = ESValue.wrapNumber(42);
let raw = numVal.unwrap(); // 42 (number)
```
---
### isBoolean
`public isBoolean(): boolean`  
Determine whether the object stored in the ESValue object is of Boolean type.

Return Value:

| Type | Description |
| :-------: | :------------------------: |
| boolean | true indicates it is a Boolean type |

Example:

```typescript
let val = ESValue.wrapBoolean(true);
let isBool = val.isBoolean(); // true
```

---
### isString
`public isString(): boolean`  
Determine whether the object stored in the ESValue object is of string type.

Return Value:

| Type | Description |
| :-------: | :------------------------: |
| boolean | true indicates it is a string type |

Example:

```typescript
let val = ESValue.wrapString('text');
let isStringValue = val.isString(); // true
```
---
### isNumber
`public isNumber(): boolean`  
Determine whether the object stored in the ESValue object is of number type.

Return Value:

| Type | Description |
| :-------: | :------------------------: |
| boolean | true indicates it is a number type |

Example:

```typescript
let val = ESValue.wrapNumber(3.14);
let isNumValue = val.isNumber(); // true
```
---
### isBigInt
`public isBigInt(): boolean`  
Determine whether the object stored in the ESValue object is of bigint type.

Return Value:

| Type | Description |
| :-------: | :--------------------------: |
| boolean | true indicates it is a BigInt type |

Example:

```typescript
let val = ESValue.wrapBigInt(100n);
let isBigIntValue = val.isBigInt(); // true
```

---
### isUndefined
`public isUndefined(): boolean`  
Determine whether the object stored in the ESValue object is an undefined value.

Return Value:

| Type | Description |
| :-------: | :----------------------------: |
| boolean | true indicates it is an undefined value |

Example:

```typescript
let val = ESValue.Undefined;
let isUndefinedValue = val.isUndefined(); // true
```

---
### isNull
`public isNull(): boolean`  
Determine whether the object stored in the ESValue object is a null value.

Return Value:

| Type | Description |
| :-------: | :----------------------: |
| boolean | true indicates it is a null value |

Example:

```typescript
let val = ESValue.Null;
let isNullValue = val.isNull(); // true
```

---
### isStaticObject
`public isStaticObject(): boolean`  
Determine whether the object stored in the ESValue object is a type created in ArkTS-Sta.

Return Value:

| Type | Description |
| :-------: | :-------------------------------: |
| boolean | true indicates it is a static object |

Example:

```typescript
let obj = ESValue.instantiateEmptyObject();
let isStaticObj = obj.isStaticObject(); // true
```

---
### isECMAObject
`public isECMAObject(): boolean`  
Determine whether the object stored in the ESValue object is of ECMA type.

Return Value:

| Type | Description |
| :-------: | :--------------------------------: |
| boolean | true indicates it is an ECMA standard object type |

Example:

```typescript
// file1.ts
class A {};
export let a = new A();
// file2.ets
let module = ESValue.load('file1');
let obj = module.load('a');
let isECMAObj = obj.isECMAObject(); // true
```

---
### isObject
`public isObject(): boolean`  
Determine whether the object stored in the ESValue object is of Object type.

Return Value:

| Type | Description |
| :-------: | :--------------------------------: |
| boolean | true indicates that it is an object type (ECMA object or static object) |

Example:

```typescript
let staticObj = ESValue.instantiateEmptyObject();
let ecmaObj = createECMAObject();
let isStaObj = staticObj.isObject(); // true
let isECMAObj = ecmaObj.isObject(); // true
```
---
### isFunction
`public isFunction(): boolean`  
Determine whether the object stored in the ESValue object is of Function type.

Return Value:

| Type | Description |
| :-------: | :--------------------------: |
| boolean | true indicates it is a function type |

Example:

```typescript
let isFunc = func.isFunction(); // true
```
---
### toBoolean
`public toBoolean(): boolean`  
Determine whether the object stored in the ESValue object is of Boolean type; if so, return it as a Boolean object, otherwise throw an exception.

Return Value:

| Type | Description |
| :-------: | :----------------------: |
| boolean | The boolean value stored in the ESValue object |

Example:

```typescript
let val = ESValue.wrapBoolean(true);
let boolVal = val.toBoolean(); // true
```

---
### toString
`public toString(): string`  
Determine whether the object stored in the ESValue object is of string type; if so, return it as a string object, otherwise throw an exception.

Return Value:

| Type | Description |
| :-----: | :----------------------: |
| string | The string value stored in the ESValue object |

Example:

```typescript
let val = ESValue.wrapString('hello');
let strValue = val.toString(); // 'hello'
```
---
### toNumber
`public toNumber(): number`  
Determine whether the object stored in the ESValue object is of number type; if so, return it as a number object, otherwise throw an exception.

Return Value:

| Type | Description |
| :-----: | :----------------------: |
| number | The number value stored in the ESValue object |

Example:

```typescript
let val = ESValue.wrapNumber(42);
let numVal = val.toNumber(); // 42
```
---
### toBigInt
`public toBigInt(): bigint`  
Determine whether the object stored in the ESValue object is of bigint type; if so, return it as a bigint object, otherwise throw an exception.

Return Value:

| Type | Description |
| :-----: | :-----------------------: |
| bigint | The bigint value stored in the ESValue object |

Example:

```typescript
let val = ESValue.wrapBigInt(100n);
let bigValue = val.toBigInt(); // 100n
```
---
### toUndefined
`public toUndefined(): undefined`  
Determine whether the object stored in the ESValue object is of undefined type; if so, return it as an undefined object, otherwise throw an exception.

Return Value:

| Type | Description |
| :-----------: | :-------------: |
| undefined | The undefined value stored in the ESValue object |

Example:

```typescript
let val = ESValue.Undefined;
let undefValue = val.toUndefined(); // undefined
```

---
### toNull
`public toNull(): null`  
Determine whether the object stored in the ESValue object is of null type; if so, return it as a null object, otherwise throw an exception.

Return Value:

| Type | Description |
| :----: | :---------: |
| null | The null value stored in the ESValue object |

Example:

```typescript
let val = ESValue.Null;
let nullValue = val.toNull(); // null
```

---
### toStaticObject
`public toStaticObject(): object`  
Determine whether the object stored in the ESValue object is a static object and return the static object, otherwise throw an exception.

Return Value:

| Type | Description |
| :-----: | :--------------------------: |
| object | The static object stored in the ESValue object |

Example:

```typescript
let objVal = ESValue.instantiateEmptyObject();
let obj = objVal.toStaticObject();
obj.property = 'value';
```

---
### areEqual
`public static areEqual(ev1: ESValue, ev2: ESValue): boolean`  
Determine whether the objects stored in two ESValue objects are equal.

Parameters:

| Parameter Name | Type | Required | Description |
| :------------: | :--------: | :------: | :-----------------: |
| ev1 | ESValue | Yes | The first value to compare |
| ev2 | ESValue | Yes | The second value to compare |

Return Value:

| Type | Description |
| :-------: | :----------------------------------: |
| boolean | true indicates the values are equal (abstract equality rule) |

Example:

```typescript
let val1 = ESValue.wrapNumber(42);
let val2 = ESValue.wrapString('42');
let res = ESValue.areEqual(val1, val2);
```

---
### areStrictlyEqual
`public static areStrictlyEqual(ev1: ESValue, ev2: ESValue): boolean`  
Determine whether the objects stored in two ESValue objects are strictly equal.

Parameters:

| Parameter Name | Type | Required | Description |
| :------------: | :--------: | :------: | :-----------------: |
| ev1 | ESValue | Yes | The first value to compare |
| ev2 | ESValue | Yes | The second value to compare |

Return Value:

| Type | Description |
| :-------: | :----------------------------------: |
| boolean | true indicates the values are equal (strict equality rule) |

Example:

```typescript
let val1 = ESValue.wrapNumber(42);
let val2 = ESValue.wrapString('42');
let res = ESValue.areStrictlyEqual(val1, val2); // false
```

---
### isEqualTo
`public isEqualTo(other: ESValue): boolean`  
Determine whether the object stored in one ESValue is strictly equal to the object stored in another ESValue.

Parameters:

| Parameter Name | Type | Required | Description |
| :------------: | :--------: | :------: | :-----------------: |
| other | ESValue | Yes | The other value to compare |

Return Value:

| Type | Description |
| :-------: | :----------------------------------: |
| boolean | true indicates the values are equal (abstract equality rule) |

Example:

```typescript
let val1 = ESValue.wrapNumber(42);
let val2 = ESValue.wrapString('42');
let res = val1.isEqualTo(val2); // true
```

---
### isStrictlyEqualTo
`public isStrictlyEqualTo(other: ESValue): boolean`  
Determine whether the value stored in one ESValue is equal to the object stored in another ESValue.

Parameters:

| Parameter Name | Type | Required | Description |
| :------------: | :--------: | :------: | :-----------------: |
| other | ESValue | Yes | The other value to compare |

Return Value:

| Type | Description |
| :-------: | :----------------------------------: |
| boolean | true indicates the values are equal (strict equality rule) |

Example:

```typescript
let val1 = ESValue.wrapNumber(42);
let val2 = ESValue.wrapNumber(42);
let res = val1.isStrictlyEqualTo(val2); // true
```

---
### instantiate
`public instantiate(...args: FixedArray<ESValue>): ESValue`  
Instantiate a class and return it wrapped as an ESValue object. The parameters are the parameters required by the constructor.

Parameters:

| Parameter Name | Type | Required | Description |
| :--------------------: | :----------------------------: | :------: | :---------------------: |
| ...args (dynamic) | FixedArray<ESValue> | No | The parameter list of the constructor |

Return Value:

| Type | Description |
| :-------: | :---------------------------------: |
| ESValue | The ESValue object storing the instance object |

Example:

```typescript
// file1.ts
export class User {
    name: string;
    id: number;
    constructor(name: string, id: number) {
        this.name = name;
        this.id = id;
    }
}
// file2.ets
let module = ESValue.load('file1');
let dynUser = module.getProperty('User');
let instance = dynUser.instantiate('hello', ESValue.wrapNumber(32));
```

---
### instantiateEmptyObject
`public static instantiateEmptyObject(): ESValue`  
Initialize an ArkTS-Dyn object and return it wrapped as an ESValue object.

Return Value:

| Type | Description |
| :-------: | :----------------------------: |
| ESValue | The empty ArkTS-Dyn object wrapped by ESValue |

Example:

```typescript
let obj = ESValue.instantiateEmptyObject();
obj.setProperty('key', ESValue.wrapString('value'));
```

---
### instantiateEmptyArray
`public static instantiateEmptyArray(): ESValue`  
Initialize an empty ArkTS-Dyn Array object and return it wrapped as an ESValue object.

Return Value:

| Type | Description |
| :-------: | :----------------------------: |
| ESValue | The empty ArkTS-Dyn Array object wrapped by ESValue |

Example:

```typescript
let arr = ESValue.instantiateEmptyArray();
arr.push(ESValue.wrapNumber(1));
arr.push(ESValue.wrapNumber(2));
```

---
### getProperty (string version)
`public getProperty(name: string): ESValue`  
Use the value of `name` as the property name to get the property value of the dynamic object stored in `this`.

Parameters:

| Parameter Name | Type | Required | Description |
| :------------: | :----: | :------: | :-----------------: |
| name | string | Yes | The property name |

Return Value:

| Type | Description |
| :-------: | :-------------------------: |
| ESValue | The obtained property value wrapped by ESValue |

Example:

```typescript
// file1.js
export let A = {
    'property1': 1,
    'property2': 2
};
// file2.ets
let jsObjectA = module.getProperty('A');
```

---
### getProperty (number version)
`public getProperty(index: number): ESValue`  
Use the `index` value as the property name to get the array value or property value from the dynamic object stored in `this`.

Parameters:

| Parameter Name | Type | Required | Description |
| :------------: | :----: | :------: | :---------------: |
| index | number | Yes | The array index |

Return Value:

| Type | Description |
| :-------: | :-------------------------: |
| ESValue | The obtained element value wrapped by ESValue |

Example:

```typescript
// file1.js
export let jsArray = ['foo', 1, true];
// file2.ets
let module = ESValue.load('file1');
let jsArray = module.getProperty('jsArray');
let val = jsArray.getProperty(2);
```

---
### getProperty (ESValue version)
`public getProperty(property: ESValue): ESValue`  
Use the dynamic object stored in `property` as the property name to get the attribute value of the dynamic object stored in `this`.

Parameters:

| Parameter Name | Type | Required | Description |
| :--------------: | :------: | :------: | :-----------------: |
| property | ESValue | Yes | The property identifier object |

Return Value:

| Type | Description |
| :-------: | :-------------------------: |
| ESValue | The obtained property value wrapped by ESValue |

Example:

```typescript
// file1.js
export let jsArray = ['foo', 1, true];
// file2.ets
let module = ESValue.load('file1');
let jsArray = module.getProperty('jsArray');
let val = jsArray.getProperty(ESValue.wrapNumber(2));
```

---
### setProperty (string version)
`public setProperty(name: string, value: ESValue): void`  
Use the `name` value as the property name, and the dynamic object stored in `value` as the property value. Set the property value of the dynamic object stored in `this`.

Parameters:

| Parameter Name | Type | Required | Description |
| :------------: | :-------: | :------: | :-----------------: |
| name | string | Yes | The property name |
| value | ESValue | Yes | The property value |

Example:

```typescript
// file1.ts
export let A = {
    'property1': 1,
};
// file2.ets
let module = ESValue.load('file1');
let value = ESValue.wrapNumber(5);
let property = ESValue.wrapString('property1');
jsObjectA.setProperty(property, value);
```

---
### setProperty (number version)
`public setProperty(index: number, value: ESValue): void`  
Use the `index` value as the property name, and the dynamic object stored in `value` as the property value. Set the property value of the dynamic object stored in `this`.

Parameters:

| Parameter Name | Type | Required | Description |
| :------------: | :-------: | :------: | :-----------------: |
| index | number | Yes | The array index |
| value | ESValue | Yes | The element value |

Example:

```typescript
// file1.ts
export let jsArray1 = ['foo', 1, true];
// file2.ets
let module = ESValue.load('file1');
let jsArray1 = module.getProperty('jsArray1');
let value = ESValue.wrapBoolean(false);
jsArray1.setProperty(2, value);
```

---
### setProperty (ESValue version)
`public setProperty(property: ESValue, value: ESValue): void`  
Use the dynamic object stored in `property` as the property name, and the dynamic object stored in `value` as the property value, to set the property of the dynamic object stored in `this`.

Parameters:

| Parameter Name | Type | Required | Description |
| :--------------: | :-------: | :------: | :-----------------: |
| property | ESValue | Yes | The property identifier object |
| value | ESValue | Yes | The property value |

Example:

```typescript
// file1.ts
export let A = {
    'property1': 1,
};
// file2.ets
let module = ESValue.load('file1');
let value = ESValue.wrapNumber(5);
let property = ESValue.wrapString('property1');
jsObjectA.setProperty(property, value);
```

---
### hasProperty (ESValue version)
`public hasProperty(property: ESValue): boolean`  
Use the dynamic object stored in `property` as the property value, and check whether the dynamic object stored in `this` contains the specified property.

Parameters:

| Parameter Name | Type | Required | Description |
| :--------------: | :------: | :------: | :-----------------: |
| property | ESValue | Yes | The property identifier object |

Return Value:

| Type | Description |
| :-------: | :------------------------------------------------: |
| boolean | true indicates the property exists (including properties on the prototype chain) |

Example:

```typescript
// file1.ts
class dynamicObjectA {
    idx: number = 0
}
export let dyObj = new dynamicObjectA();
export let key = 'idx';
// file2.ets
let module = ESValue.load('file1');
let obj = module.getProperty('dyObj');
let key = module.getProperty('key');
let hasIdx = obj.hasProperty(key);
```

---
### hasProperty (string version)
`public hasProperty(name: string): boolean`  
Using `name` as the property name, check whether the dynamic object stored in `this` contains the specified property.

Parameters:

| Parameter Name | Type | Required | Description |
| :------------: | :----: | :------: | :-----------------: |
| name | string | Yes | The property name |

Return Value:

| Type | Description |
| :-------: | :------------------------------------------------: |
| boolean | true indicates the property exists (including properties on the prototype chain) |

Example:

```typescript
// file1.ts
class dynamicObjectA {
    idx: number = 0
}
export let dyObj = new dynamicObjectA();
// file2.ets
let module = ESValue.load('file1');
let obj = module.getProperty('dyObj');
let hasIdx = obj.hasProperty('idx');
```

---
### hasOwnProperty (ESValue version)
`public hasOwnProperty(property: ESValue): boolean`  
Using a dynamic object stored in `property` as the property value, check whether the object saved in `this` contains the specified property itself (excluding the prototype chain).

Parameters:

| Parameter Name | Type | Required | Description |
| :--------------: | :------: | :------: | :-----------------: |
| property | ESValue | Yes | The property identifier object |

Return Value:

| Type | Description |
| :-------: | :----------------------------------------: |
| boolean | true indicates the object itself contains the property |

Example:

```typescript
// file1.ts
class dynamicObjectA {
    idx: number = 0
}
export let dyObj = new dynamicObjectA();
export let key = 'idx';
// file2.ets
let module = ESValue.load('file1');
let obj = module.getProperty('dyObj');
let key = module.getProperty('key');
let hasIdx = obj.hasOwnProperty(key);
```

---
### hasOwnProperty (string version)
`public hasOwnProperty(name: string): boolean`  
Use `name` as the attribute value to check whether the dynamic object stored in `this` contains the specified property itself (excluding the prototype chain).

Parameters:

| Parameter Name | Type | Required | Description |
| :------------: | :----: | :------: | :-----------------: |
| name | string | Yes | The property name |

Return Value:

| Type | Description |
| :-------: | :----------------------------------------: |
| boolean | true indicates the object itself contains the property |

Example:

```typescript
// file1.ts
class dynamicObjectA {
    idx: number = 0
}
export let dyObj = new dynamicObjectA();
// file2.ets
let module = ESValue.load('file1');
let obj = module.getProperty('dyObj');
let hasIdx = obj.hasOwnProperty('idx');
```

---
### invoke
`public invoke(...args: FixedArray<ESValue>): ESValue`  
Execute the function or method of the dynamic object stored in `this`, with `args` being wrapped ESValue objects.

Parameters:

| Parameter Name | Type | Required | Description |
| :----------------: | :--------------: | :------: | :-----------------: |
| ...args | FixedArray<ESValue> | No | The function parameter list |

Return Value:

| Type | Description |
| :-------: | :-------------------------: |
| ESValue | The method execution result wrapped by ESValue |

Example:

```typescript
// file1.ts
export let jsFunc = function () { return 6; };
// file2.ets
let module = ESValue.load('file1');
let jsFunc = module.getProperty('jsFunc');
let result = jsFunc.invoke();
```
---
### invokeWithRecv
`public invokeWithRecv(recv: ESValue, ...args: FixedArray<ESValue>): ESValue`  
Use `recv` as this and `args` as parameters to execute the method stored in ESValue. `args` is a wrapped ESValue object.

Parameters:

| Parameter Name | Type | Required | Description |
| :------------: | :---------------: | :------: | :-----------------: |
| recv | ESValue | Yes | The this value |
| ...args | FixedArray<ESValue> | No | The function parameter list |

Return Value:

| Type | Description |
| :-------: | :-------------------------: |
| ESValue | The method execution result wrapped by ESValue |

Example:

```typescript
let global = ESValue.getGlobal();
let symbol = global.getProperty("Symbol");
let symbolIterator = symbol.getProperty("iterator");
let symbolIteratorMethod = iterableObj.getProperty(symbolIterator);
let iterator = symbolIteratorMethod.invokeWithRecv(iterableObj);
```
---
### invokeMethod
`public invokeMethod(method: string, ...args: FixedArray<ESValue>): ESValue`  
Use the method name `mothod` and `args` as parameters to get the method saved in `this` dynamic object and execute it. `args` are wrapped ESValue objects.

Parameters:

| Parameter Name | Type | Required | Description |
| :------------: | :---------------: | :------: | :-----------------: |
| method | string | Yes | The method name |
| ...args | FixedArray<ESValue> | No | The method parameter list |

Return Value:

| Type | Description |
| :-------: | :-------------------------: |
| ESValue | The method execution result wrapped by ESValue |

Example:

```typescript
// file1.ts
export let objWithFunc = {
    'func': function () {
        return 'hello';
    }
};
// file2.ets
let module = ESValue.load('file1');
let jsObjWithFunc = module.getProperty('objWithFunc');
let result = jsObjWithFunc.invokeMethod('func');
```
---
### keys
`public keys(): IterableIterator<ESValue>`  
Get an iterator of the own enumerable property names of the dynamic object saved in this, where the values obtained from iteration are dynamic objects wrapped by ESValue.

Return Value:

| Type | Description |
| :-------------------------: | :-------------------------: |
| IterableIterator<ESValue> | The property name iterator wrapped by ESValue |

Example:

```typescript
// file1.js
export let testItetatorObject = {'a': 1, 'b': 2, 'c' : 3};
// file2.ets
let module = ESValue.load('file1');
let jsIterableObject = module.getProperty('testItetatorObject');
let result: String = new String();
for (const key of jsIterableObject.keys()) {
    result += key.toString();
}
```

---
### values
`public values(): IterableIterator<ESValue>`  
Obtain an iterator of the enumerable property values of the dynamic object saved in this, where the iterated values are ESValue-wrapped dynamic objects.

Return Value:

| Type | Description |
| :-------------------------: | :-------------------------: |
| IterableIterator<ESValue> | The property value iterator wrapped by ESValue |

Example:

```typescript
// file1.js
export let testItetatorObject = {'a': 1, 'b': 2, 'c' : 3};
// file2.ets
let module = ESValue.load('file1');
let jsIterableObject = module.getProperty('testItetatorObject');
let result: number = 0;
for (const value of jsIterableObject.values()) {
    result += value.toNumber();
}
```

---
### entries
`public entries(): IterableIterator<[ESValue, ESValue]>` 
Get an iterator over the own enumerable key-value pairs of this stored dynamic object, where the key-value pairs obtained from iteration are dynamic objects wrapped in ESValue.

Return Value:

| Type | Description |
| :-------------------------------------: | :-------------------------: |
| IterableIterator<[ESValue, ESValue]> | The tuple iterator of [property name wrapped by ESValue, property value wrapped by ESValue] |

Example:

```typescript
// file1.js
export let testItetatorObject = {'a': 1, 'b': 2, 'c' : 3};
// file2.ets
let module = ESValue.load('file1');
let jsIterableObject = module.getProperty('testItetatorObject');
let resultkey: String =  = new String();
let resultValue: number = 0;
for (const entry of jsIterableObject.entries()) {
    resultkey += entry[0].toString();
    resultValue += entry[1].toNumber();
}
```

---
### instanceOf
`public instanceOf(type: ESValue): boolean`  
Check whether the dynamic object stored in `this` object is an instance of the dynamic type stored in the `type` object.

Parameters:

| Parameter Name | Type | Required | Description |
| :------------: | :-----------: | :------: | :-----------------: |
| type | ESValue | Yes | Dynamic type |

Return Value:

| Type | Description |
| :-------: | :-------------------------------: |
| boolean | true indicates it is an instance of the target type |

Example:

```typescript
// file1.js
export class User {
	ID = 123;
}
// file2.ets
let module = ESValue.load('file1');
let jsUser = module.getProperty('User');
let user = jsUser.instantiate(num);
let res =  user.instanceOf(jsUser);
```
---
### typeOf
`public typeOf(): String`  
Get the type string of the dynamic object stored in `this`.

Return Value:

| Type | Description |
| :-------: | :-----------------------------: |
| String | The type string of the object stored in ESValue (in lowercase) |

Example:

```typescript
ESValue.wrapNumber(42).typeOf();   // 'number'
ESValue.Null.typeOf();       // 'null'
```
---
### load
`public static load(module: string): ESValue`  
Load the ArkTS-Dyn module according to the specified path name and return the object wrapped by ESValue.

Parameters:

| Parameter Name | Type | Required | Description |
| :------------: | :----: | :------: | :-----------------: |
| module | string | Yes | The module path or identifier |

Return Value:

| Type | Description |
| :-------: | :----------------------------: |
| ESValue | The ArkTS-Dyn module wrapped by ESValue |

Example:

```typescript
// file1.js
let num = 1;
// file2.ets
let module = ESValue.load('file1');
```

---
### getGlobal
public static getGlobal(): ESValue
Get the globalThis object of ArkTS-Dyn and return the object wrapped by ESValue.

Return Value:

| Type | Description |
| :-------: | :-------------------------: |
| ESValue | The reference to the runtime global object |

Example:

```typescript
let global = ESValue.getGlobal();
```

---
### isPromise
`public isPromise(): boolean`  
Determine whether the dynamic object stored in `this` is a Promise object.

Return Value:

| Type | Description |
| :-------: | :---------------------------------: |
| boolean | true indicates it is a Promise object |

Example:

```typescript
// file1.ts
export async function sleepRetNumber(ms: number): Promise<number> {
    await sleep(ms);
    return 0xcafe;
}
// file2.ets
let module = ESValue.load('file1');
let sleepRetNumber = module.getProperty('sleepRetNumber');
let res = sleepRetNumber.invoke(ESValue.wrapNumber(5000)).isPromise();
```

---
### $_iterator
`public $_iterator(): IterableIterator<ESValue>`  
Get an iterator object from the dynamic object stored in `this`, where the iterated values are dynamic objects wrapped in ESValue.

Return Value:

| Type | Description |
| :-------------------------: | :------------------------------------------------: |
| IterableIterator<ESValue> | The iterable iterator interface that implements the internal members of the ESValue object |

Example:

```typescript
// file1.js
export let jsIterable = ['a', 'b', 'c', 'd'];
// file2.ets
let module = ESValue.load('file1');
let jsIterable = module.getProperty('jsIterable');
let result: String = new String();
for (const elem of jsIterable) {
    result += elem.toString();
}
```
