# ESValue
## 介绍
ESValue作为一个封装类，它主要提供了一系列能够在静态类型ArkTS-Sta中操作动态类型ArkTS-Dyn的数据接口。
## 详细接口
### Undefined
`public static get Undefined(): ESValue`  
获取存储动态undefined值的ESValue对象。

返回值：

|    类型     |            说明            |
| :---------: | :------------------------: |
|   ESValue   | 存储动态undefined值的ESValue对象 |

示例代码：
```typescript
let undefinedVal = ESValue.Undefined;
let isUndefinedVal = undefinedVal.isUndefined(); // true
```
---
### Null
`public static get Null(): ESValue`  
获取存储动态null值的ESValue对象。

返回值：

|    类型     |            说明            |
| :---------: | :------------------------: |
|   ESValue   | 存储动态null值的ESValue对象 |

示例代码：
```typescript
let nullVal = ESValue.Null;
let isNullVal = nullVal.isNull(); // true
```
---
### wrapBoolean
`public static wrapBoolean(b: boolean): ESValue`  
将布尔值包装为ESValue对象。

参数：

| 参数名 |  类型   | 必填 |       说明       |
| :----: | :-----: | :--: | :--------------: |
|    b   | boolean |  是  | 需要包装的boolean值 |

返回值：

|    类型     |          说明          |
| :---------: | :--------------------: |
|   ESValue   | 存储动态布尔值的ESValue对象 |

示例代码：
```typescript
let trueVal = ESValue.wrapBoolean(true);
let boolVal = trueVal.toBoolean(); // true
```
---
### wrapString
`public static wrapString(s: string): ESValue`  
将字符串包装为ESValue对象。

参数：

| 参数名 |  类型  | 必填 |     说明     |
| :----: | :----: | :--: | :----------: |
|    s   | string |  是  | 需要包装的string对象 |

返回值：

|    类型     |          说明          |
| :---------: | :--------------------: |
|   ESValue   | 存储动态字符串值的ESValue对象 |

示例代码：
```typescript
let strWrap = ESValue.wrapString('Hello');
let strValue = strWrap.toString(); // 'Hello'
```
---
### wrapNumber
`public static wrapNumber(n: number): ESValue`  
将数字包装为ESValue对象。

参数：

| 参数名 |  类型  | 必填 |     说明     |
| :----: | :----: | :--: | :----------: |
|    n   | number |  是  | 需要包装的number对象 |

返回值：

|    类型     |          说明          |
| :---------: | :--------------------: |
|   ESValue   | 存储动态number值的ESValue对象 |

示例代码：
```typescript
let numWrap = ESValue.wrapNumber(3.1415);
let numValue = numWrap.toNumber(); // 3.1415
```
---
### wrapBigInt
`public static wrapBigInt(bi: bigint): ESValue`  
将BigInt包装为ESValue对象。

参数：

| 参数名 |  类型  | 必填 |     说明     |
| :----: | :----: | :--: | :----------: |
|   bi   | bigint |  是  | 需要包装的bigint对象 |

返回值：

|    类型     |           说明           |
| :---------: | :----------------------: |
|   ESValue   | 存储动态bigint值的ESValue对象 |

示例代码：
```typescript
let bigWrap = ESValue.wrapBigInt(9007199254740991n);
let bigValue = bigWrap.toBigInt(); // 9007199254740991n
```
---
### wrapByte
`public static wrapByte(b: byte): ESValue`  
将字节整数值包装为ESValue对象。

参数：

| 参数名 |  类型  | 必填 |        说明        |
| :----: | :----: | :--: | :----------------: |
|    b   |  byte  |  是  | 需要包装的8位整数 (-128~127) |

返回值：

|    类型     |           说明           |
| :---------: | :----------------------: |
|   ESValue   | 存储动态byte的ESValue对象 |

示例代码：
```typescript
let byteWrap = ESValue.wrapByte(100);
let byteValue = byteWrap.toByte(); // 100
```
---
### wrapShort
`public static wrapShort(s: short): ESValue`  
将短整数值包装为ESValue对象。

参数：

| 参数名 |  类型  | 必填 |         说明         |
| :----: | :----: | :--: | :------------------: |
|    s   | short  |  是  | 16位整数 (-32768~32767) |

返回值：

|    类型     |           说明           |
| :---------: | :----------------------: |
|   ESValue   | 存储动态short值的ESValue对象 |

示例代码：
```typescript
let shortWrap = ESValue.wrapShort(20000);
let shortValue = shortWrap.toShort(); // 20000
```
---
### wrapInt
`public static wrapInt(i: int): ESValue`  
将整数值包装为ESValue对象。

参数：

| 参数名 |  类型  | 必填 |       说明       |
| :----: | :----: | :--: | :--------------: |
|    i   |  int  |  是  | 需要包装的32位整数 |

返回值：

|    类型     |           说明           |
| :---------: | :----------------------: |
|   ESValue   | 存储动态int的ESValue对象 |

示例代码：
```typescript
let intWrap = ESValue.wrapInt(2147483647);
let intValue = intWrap.toInt(); // 2147483647
```
---
### wrapLong
`public static wrapLong(l: long): ESValue`  
将长整数值包装为ESValue对象。

参数：

| 参数名 |  类型  | 必填 |         说明         |
| :----: | :----: | :--: | :------------------: |
|    l   |  long  |  是  | 需要包装的64位整数 |

返回值：

|    类型     |           说明           |
| :---------: | :----------------------: |
|   ESValue   | 存储动态long值的ESValue对象 |

示例代码：
```typescript
let longWrap = ESValue.wrapLong(9223372036854775807);
let longValue = longWrap.toLong(); // 9223372036854775807
```
---
### wrapLongLossy
`public static wrapLongLossy(l: long): ESValue`  
将有损转换长整数值为ESValue对象。

参数：

| 参数名 |  类型  | 必填 |        说明        |
| :----: | :----: | :--: | :--------------: |
|    l   |  long  |  是  | 需要包装的64位整数 |

返回值：

|    类型     |           说明            |
| :---------: | :-----------------------: |
|   ESValue   | 存储动态long值的ESValue对象 |

示例代码：
```typescript
let lossyWrap = ESValue.wrapLongLossy(Number.MAX_SAFE_INTEGER + 100);
let lossyValue = lossyWrap.isNumber(); // true
```
---
### wrapFloat
`public static wrapFloat(f: float): ESValue`  
将单精度浮点数值包装为ESValue对象。

参数：

| 参数名 |  类型  | 必填 |           说明           |
| :----: | :----: | :--: | :--------------------: |
|    f   | float  |  是  | 需要包装的32位浮点数数值 |

返回值：

|    类型     |           说明           |
| :---------: | :----------------------: |
|   ESValue   | 存储动态float值的ESValue对象 |

示例代码：
```typescript
let floatWrap = ESValue.wrapFloat(3.1415927);
floatValue = floatWrap.toFloat(); // 3.1415927
```
---
### wrapDouble
`public static wrapDouble(d: double): ESValue`  
将双精度浮点数值包装为ESValue对象。

参数：

| 参数名 |  类型  | 必填 |          说明          |
| :----: | :----: | :--: | :------------------: |
|    d   | double |  是  | 需要包装的64位浮点数数值 |

返回值：

|    类型     |           说明           |
| :---------: | :----------------------: |
|   ESValue   | 存储动态double值的ESValue对象 |

示例代码：
```typescript
let doubleWrap = ESValue.wrapDouble(3.14159265358);
let doubleValue = doubleWrap.toDouble(); // 3.14159265358
```
---
### unwrap
`public unwrap(): Any`  
将ESValue解包为原始值类型

返回值：

|  类型  |        说明        |
| :----: | :----------------: |
|   Any  | ESValue保存的原始对象 |

示例代码：
```typescript
let numVal = ESValue.wrapNumber(42);
let raw = numVal.unwrap(); // 42 (number)
```
---
### isBoolean
`public isBoolean(): boolean`  
判断ESValue对象中保存的对象是否是Boolean类型。

返回值：

|   类型    |        说明         |
| :-------: | :-----------------: |
| boolean | true表示是布尔类型 |

示例代码：
```typescript
let val = ESValue.wrapBoolean(true);
let isBool = val.isBoolean(); // true
```
---
### isString
`public isString(): boolean`  
判断ESValue对象中保存的对象是否是string类型。

返回值：

|   类型    |        说明         |
| :-------: | :-----------------: |
| boolean | true表示是字符串类型 |

示例代码：
```typescript
let val = ESValue.wrapString('text');
let isStringValue = val.isString(); // true
```
---
### isNumber
`public isNumber(): boolean`  
判断ESValue对象中保存的对象是否是number类型。

返回值：

|   类型    |        说明         |
| :-------: | :-----------------: |
| boolean | true表示是数字类型 |

示例代码：
```typescript
let val = ESValue.wrapNumber(3.14);
let isNumValue = val.isNumber(); // true
```
---
### isBigInt
`public isBigInt(): boolean`  
判断ESValue对象中保存的对象是否是bigint类型。

返回值：

|   类型    |         说明          |
| :-------: | :-------------------: |
| boolean | true表示是BigInt类型 |

示例代码：
```typescript
let val = ESValue.wrapBigInt(100n);
let isBigIntValue = val.isBigInt(); // true
```
---
### isUndefined
`public isUndefined(): boolean`  
判断ESValue对象中保存的对象是否是undefined值。

返回值：

|   类型    |          说明           |
| :-------: | :---------------------: |
| boolean | true表示是undefined值 |

示例代码：
```typescript
let val = ESValue.Undefined;
let isUndefinedValue = val.isUndefined(); // true
```
---
### isNull
`public isNull(): boolean`  
判断ESValue对象中保存的对象是否是null值。

返回值：

|   类型    |       说明        |
| :-------: | :--------------: |
| boolean | true表示是null值 |

示例代码：
```typescript
let val = ESValue.Null;
let isNullValue = val.isNull(); // true
```
---
### isStaticObject
`public isStaticObject(): boolean`  
判断ESValue对象中保存的对象是否是在ArkTS-Sta中创建的类型。

返回值：

|   类型    |           说明            |
| :-------: | :-----------------------: |
| boolean | true表示是静态对象 |

示例代码：
```typescript
let obj = ESValue.instantiateEmptyObject();
let isStaticObj = obj.isStaticObject(); // true
```
---
### isECMAObject
`public isECMAObject(): boolean`  
判断ESValue对象中保存的对象是否是ECMA类型。

返回值：

|   类型    |            说明            |
| :-------: | :------------------------: |
| boolean | true表示是ECMA标准对象类型 |

示例代码：
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
判断ESValue对象中保存的对象是否是Object类型。

返回值：

|   类型    |            说明            |
| :-------: | :------------------------: |
| boolean | true表示是对象类型（ECMA对象或静态对象） |

示例代码：
```typescript
let staticObj = ESValue.instantiateEmptyObject();
let ecmaObj = createECMAObject();
let isStaObj = staticObj.isObject(); // true
let isECMAObj = ecmaObj.isObject(); // true
```
---
### isFunction
`public isFunction(): boolean`  
判断ESValue对象中保存的对象是否是Function类型。

返回值：

|   类型    |         说明         |
| :-------: | :------------------: |
| boolean | true表示是函数类型 |

示例代码：
```typescript
let isFunc = func.isFunction(); // true
```
---
### toBoolean
`public toBoolean(): boolean`  
判断ESValue对象中保存的对象是否是为Boolean类型，若是则将其以Boolean对象返回，否则抛出异常。

返回值：

|   类型    |       说明       |
| :-------: | :--------------: |
| boolean | ESValue对象中保存的boolean值 |

示例代码：
```typescript
let val = ESValue.wrapBoolean(true);
let boolVal = val.toBoolean(); // true
```
---
### toString
`public toString(): string`  
判断ESValue对象中保存的对象是否是为string类型，若是则将其以string对象返回，否则抛出异常。

返回值：

|  类型   |       说明       |
| :-----: | :--------------: |
| string | ESValue对象中保存的string值 |

示例代码：
```typescript
let val = ESValue.wrapString('hello');
let strValue = val.toString(); // 'hello'
```
---
### toNumber
`public toNumber(): number`  
判断ESValue对象中保存的对象是否是为number类型，若是则将其以number对象返回，否则抛出异常。

返回值：

|  类型   |       说明       |
| :-----: | :--------------: |
| number | ESValue对象中保存的number值 |

示例代码：
```typescript
let val = ESValue.wrapNumber(42);
let numVal = val.toNumber(); // 42
```
---
### toBigInt
`public toBigInt(): bigint`  
判断ESValue对象中保存的对象是否是为bigint类型，若是则将其以bigint对象返回，否则抛出异常。

返回值：

|  类型   |       说明        |
| :-----: | :---------------: |
| bigint | ESValue对象中保存的bigint值 |

示例代码：
```typescript
let val = ESValue.wrapBigInt(100n);
let bigValue = val.toBigInt(); // 100n
```
---
### toUndefined
`public toUndefined(): undefined`  
判断ESValue对象中保存的对象是否是为undefined类型，若是则将其以undefined对象返回，否则抛出异常。

返回值：

|     类型      |   说明   |
| :-----------: | :------: |
| undefined | ESValue对象中保存的undefined值 |

示例代码：
```typescript
let val = ESValue.Undefined;
let undefValue = val.toUndefined(); // undefined
```
---
### toNull
`public toNull(): null`  
判断ESValue对象中保存的对象是否是为nulll类型，若是则将其以null对象返回，否则抛出异常。

返回值：

|  类型  | 说明 |
| :----: | :--: |
|  null  | ESValue对象中保存的null值 |

示例代码：
```typescript
let val = ESValue.Null;
let nullValue = val.toNull(); // null
```
---
### toStaticObject
`public toStaticObject(): object`  
判断ESValue对象中保存的对象是否是为静态对象，并返回静态对象，否则抛出异常。

返回值：

|  类型   |         说明          |
| :-----: | :-------------------: |
| object | ESValue对象中保存的静态对象 |

示例代码：
```typescript
let objVal = ESValue.instantiateEmptyObject();
let obj = objVal.toStaticObject();
obj.property = 'value';
```
---
### areEqual
`public static areEqual(ev1: ESValue, ev2: ESValue): boolean`  
判断两个ESValue对象中保存的对象是否是相等。

参数：

| 参数名 |    类型    | 必填 |     说明     |
| :----: | :--------: | :--: | :----------: |
|   ev1  |   ESValue  |  是  | 比较的第一个值 |
|   ev2  |   ESValue  |  是  | 比较的第二个值 |

返回值：

|   类型    |             说明             |
| :-------: | :--------------------------: |
| boolean | true表示值相等（抽象相等规则） |

示例代码：
```typescript
let val1 = ESValue.wrapNumber(42);
let val2 = ESValue.wrapString('42');
let res = ESValue.areEqual(val1, val2); // true (抽象相等)
```
---
### areStrictlyEqual
`public static areStrictlyEqual(ev1: ESValue, ev2: ESValue): boolean`  
判断两个ESValue对象中保存的对象是否是严格相等。

参数：

| 参数名 |    类型    | 必填 |     说明     |
| :----: | :--------: | :--: | :----------: |
|   ev1  |   ESValue  |  是  | 比较的第一个值 |
|   ev2  |   ESValue  |  是  | 比较的第二个值 |

返回值：

|   类型    |             说明             |
| :-------: | :--------------------------: |
| boolean | true表示值相等（严格相等规则） |

示例代码：
```typescript
let val1 = ESValue.wrapNumber(42);
let val2 = ESValue.wrapString('42');
let res = ESValue.areStrictlyEqual(val1, val2); // false
```
---
### isEqualTo
`public isEqualTo(other: ESValue): boolean`  
判断一个ESValue中保存的对象是否与另外一个ESValue中保存的对象是否是严格相等。

参数：

| 参数名 |    类型    | 必填 |     说明     |
| :----: | :--------: | :--: | :----------: |
|  other |   ESValue  |  是  | 比较的另一个值 |

返回值：

|   类型    |             说明             |
| :-------: | :--------------------------: |
| boolean | true表示值相等（抽象相等规则） |

示例代码：
```typescript
let val1 = ESValue.wrapNumber(42);
let val2 = ESValue.wrapString('42');
let res = val1.isEqualTo(val2); // true
```
---
### isStrictlyEqualTo
`public isStrictlyEqualTo(other: ESValue): boolean`  
判断一个ESValue中保存的值是否与另外一个ESValue中保存的对象是否是相等。

参数：

| 参数名 |    类型    | 必填 |     说明     |
| :----: | :--------: | :--: | :----------: |
|  other |   ESValue  |  是  | 比较的另一个值 |

返回值：

|   类型    |             说明             |
| :-------: | :--------------------------: |
| boolean | true表示值相等（严格相等规则） |

示例代码：
```typescript
let val1 = ESValue.wrapNumber(42);
let val2 = ESValue.wrapNumber(42);
let res = val1.isStrictlyEqualTo(val2); // true
```
---
### instantiate
`public instantiate(...args: FixedArray<ESValue>): ESValue`  
实例化一个类，并将其包装成ESValue对象返回。参数为构造函数所需的参数。

参数：

|     参数名     |         类型         | 必填 |       说明       |
| :------------: | :------------------: | :--: | :--------------: |
| ...args (动态) | FixedArray<ESValue> |  否  | 构造函数的参数列表 |

返回值：

|   类型    |            说明             |
| :-------: | :-------------------------: |
|   ESValue | 存储实例对象的ESValue对象 |

示例代码：
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
初始化一个ArkTS-Dyn对象，并将其包装成ESValue对象返回。

返回值：

|   类型    |          说明          |
| :-------: | :--------------------: |
|   ESValue | ESValue包装的ArkTS-Dyn空的object对象 |

示例代码：
```typescript
let obj = ESValue.instantiateEmptyObject();
obj.setProperty('key', ESValue.wrapString('value'));
```
---
### instantiateEmptyArray
`public static instantiateEmptyArray(): ESValue`  
初始化一个ArkTS-Dyn空的Array对象，并将其包装成ESValue对象返回。

返回值：

|   类型    |          说明          |
| :-------: | :--------------------: |
|   ESValue | ESValue包装的ArkTS-Dyn空的Array对象 |

示例代码：
```typescript
let arr = ESValue.instantiateEmptyArray();
arr.push(ESValue.wrapNumber(1));
arr.push(ESValue.wrapNumber(2));
```
---
### getProperty (string版本)
`public getProperty(name: string): ESValue`  
以`name`值为属性名，获取this中保存的动态对象的属性值。

参数：

| 参数名 |  类型  | 必填 |     说明     |
| :----: | :----: | :--: | :---------: |
|  name  | string |  是  |  属性名称  |

返回值：

|   类型    |        说明         |
| :-------: | :-----------------: |
|  ESValue | ESValue包装的获取到属性值 |

示例代码：
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
### getProperty (number版本)
`public getProperty(index: number): ESValue`  
以`index`值为属性名，获取this中保存的动态对象里的数组值或属性值。

参数：

| 参数名 |  类型  | 必填 |    说明    |
| :----: | :----: | :--: | :--------: |
| index  | number |  是  |  数组索引  |

返回值：

|   类型    |        说明         |
| :-------: | :-----------------: |
|  ESValue | ESValue包装的获取到元素值 |

示例代码：
```typescript
// file1.js
export let jsArray = ['foo', 1, true];
// file2.ets
let module = ESValue.load('file1');
let jsArray = module.getProperty('jsArray');
let val = jsArray.getProperty(2);
```
---
### getProperty (ESValue版本)
`public getProperty(property: ESValue): ESValue`  
以`property`保存的动态对象为属性名，获取this保存的动态对象的属性值。

参数：

|  参数名  |   类型   | 必填 |     说明     |
| :------: | :------: | :--: | :----------: |
| property | ESValue |  是  | 属性标识对象 |

返回值：

|   类型    |        说明         |
| :-------: | :-----------------: |
|  ESValue | ESValue包装的获取到属性值 |

示例代码：
```typescript
// file1.js
export let jsArray = ['foo', 1, true];
// file2.ets
let module = ESValue.load('file1');
let jsArray = module.getProperty('jsArray');
let val = jsArray.getProperty(ESValue.wrapNumber(2));
```
---
### setProperty (string版本)
`public setProperty(name: string, value: ESValue): void`  
以`name`值为属性名，`value`保存的动态对象为属性值，设置this保存的动态对象的属性值。

参数：

| 参数名 |       类型        | 必填 |     说明     |
| :----: | :-------: | :--: | :----------: |
|  name  |  string   |  是  |   属性名称   |
| value  |  ESValue  |  是  |   属性值     |

示例代码：
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
### setProperty (number版本)
`public setProperty(index: number, value: ESValue): void`  
以`index`值为属性名，`value`保存的动态对象为属性值，设置this保存的动态对象的属性值。

参数：

| 参数名 |       类型        | 必填 |     说明     |
| :----: | :-------: | :--: | :----------: |
| index  |  number   |  是  |   数组索引   |
| value  |  ESValue  |  是  |   元素值     |

示例代码：
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
### setProperty (ESValue版本)
`public setProperty(property: ESValue, value: ESValue): void`  
以`property`保存的动态对象为属性名，`value`保存的动态对象为属性值，设置this保存的动态对象的属性。

参数：

|  参数名  |       类型        | 必填 |     说明     |
| :------: | :-------: | :--: | :----------: |
| property |  ESValue  |  是  | 属性标识对象 |
|  value   |  ESValue  |  是  |   属性值     |

示例代码：
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
### hasProperty (ESValue版本)
`public hasProperty(property: ESValue): boolean`  
以`property`保存的动态对象为属性值，检查this保存的动态对象中是否包含指定属性。

参数：

|  参数名  |   类型   | 必填 |     说明     |
| :------: | :------: | :--: | :----------: |
| property | ESValue |  是  | 属性标识对象 |

返回值：

|   类型    |                    说明                    |
| :-------: | :----------------------------------------: |
|  boolean | true表示属性存在（包含原型链上的属性） |

示例代码：
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
### hasProperty (string版本)
`public hasProperty(name: string): boolean`  
以`name`为属性名，检查this保存的动态对象中是否包含指定属性。

参数：

| 参数名 |  类型  | 必填 |     说明     |
| :----: | :----: | :--: | :----------: |
|  name  | string |  是  |   属性名称   |

返回值：

|   类型    |                    说明                    |
| :-------: | :----------------------------------------: |
|  boolean | true 表示属性存在（包含原型链上的属性） |

示例代码：
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
### hasOwnProperty (ESValue版本)
`public hasOwnProperty(property: ESValue): boolean`  
以`property`保存的动态对象为属性值，检查this保存的对象中自身（不包含原型链）是否包含指定属性。

参数：

|  参数名  |   类型   | 必填 |     说明     |
| :------: | :------: | :--: | :----------: |
| property | ESValue |  是  | 属性标识对象 |

返回值：

|   类型    |                说明                |
| :-------: | :--------------------------------: |
|  boolean | true表示对象自身包含该属性 |

示例代码：
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
### hasOwnProperty (string版本)
`public hasOwnProperty(name: string): boolean`  
以`name`为属性值，检查this保存的动态对象中自身（不包含原型链）是否包含指定属性。

参数：

| 参数名 |  类型  | 必填 |     说明     |
| :----: | :----: | :--: | :----------: |
|  name  | string |  是  |   属性名称   |

返回值：

|   类型    |                说明                |
| :-------: | :--------------------------------: |
|  boolean | true表示对象自身包含该属性 |

示例代码：
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
执行this中保存的动态对象的函数或方法，`args`为包装的ESValue对象。

参数：

|   参数名   |       类型        | 必填 |     说明     |
| :--------: | :--------------: | :--: | :----------: |
| ...args | FixedArray<ESValue> |  否  | 函数参数列表 |

返回值：

|   类型    |        说明         |
| :-------: | :-----------------: |
|  ESValue | ESValue包装的方法执行结果 |

示例代码：
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
以`recv`为this，`args`为参数，执行ESValue中保存的方法。`args`为包装的ESValue对象。

参数：

| 参数名 |       类型        | 必填 |     说明     |
| :----: | :---------------: | :--: | :----------: |
|  recv  |      ESValue       |  是  |    this值   |
| ...args | FixedArray<ESValue> |  否  | 函数参数列表 |

返回值：

|   类型    |        说明         |
| :-------: | :-----------------: |
|  ESValue | ESValue包装的方法执行结果 |

示例代码：
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
以`mothod`方法名，`args`为参数，获取this中保存的动态对象的方法并执行该方法。`args`为包装的ESValue对象。

参数：

| 参数名 |       类型        | 必填 |     说明     |
| :----: | :---------------: | :--: | :----------: |
| method |      string       |  是  |   方法名称   |
| ...args | FixedArray<ESValue> |  否  | 方法参数列表 |

返回值：

|   类型    |        说明         |
| :-------: | :-----------------: |
|  ESValue | ESValue包装的方法执行结果 |

示例代码：
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
获取this保存的动态对象中自身可枚举属性名的迭代器，迭代得到的值为ESValue包装的动态对象。

返回值：

|            类型             |        说明         |
| :-------------------------: | :-----------------: |
| IterableIterator<ESValue> | ESValue包装的属性名迭代器 |

示例代码：
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
获取this保存的动态对象中自身可枚举属性值的迭代器，迭代得到的值为ESValue包装的动态对象。

返回值：

|            类型             |        说明         |
| :-------------------------: | :-----------------: |
| IterableIterator<ESValue> | ESValue包装的属性值迭代器 |

示例代码：
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
获取this保存的动态对象中自身可枚举键值对的迭代器，迭代得到的键值对都为ESValue包装的动态对象。

返回值：

|                  类型                   |        说明         |
| :-------------------------------------: | :-----------------: |
| IterableIterator<[ESValue, ESValue]> | [ESValue包装的属性名, ESValue包装的属性值]元组迭代器 |

示例代码：
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
检查this对象中保存的动态对象，是否是`type`对象中保存的动态类型的实例。

参数：

| 参数名 |   类型    | 必填 |     说明     |
| :----: | :-----: | :--: | :----------: |
|  type  | ESValue |  是  | 动态类型 |

返回值：

|   类型    |           说明            |
| :-------: | :-----------------------: |
|  boolean | true 表示是目标类型实例 |

示例代码：
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
获取this中保存的动态对象的类型字符串。

返回值：

|   类型    |          说明           |
| :-------: | :---------------------: |
|   String  | ESValue中保存对象类型字符串（小写形式） |

示例代码：
```typescript
ESValue.wrapNumber(42).typeOf();   // 'number'
ESValue.Null.typeOf();       // 'null'
```
---
### load
`public static load(module: string): ESValue`  
根据指定路径名加载ArkTS-Dyn的模块，返回用ESValue包装的对象。

参数：

| 参数名 |  类型  | 必填 |     说明     |
| :----: | :----: | :--: | :----------: |
| module | string |  是  | 模块路径或标识 |

返回值：

|   类型    |          说明          |
| :-------: | :--------------------: |
|   ESValue | ESValue包装ArkTS-Dyn的模块 |

示例代码：
```typescript
// file1.js
let num = 1;
// file2.ets
let module = ESValue.load('file1');
```
---
### getGlobal
`public static getGlobal(): ESValue`  
获取ArkTS-Dyn的globalThis对象，返回用ESValue包装的对象。

返回值：

|   类型    |        说明         |
| :-------: | :-----------------: |
|   ESValue | 运行时全局对象引用 |

示例代码：
```typescript
let global = ESValue.getGlobal();
```
---
### isPromise
`public isPromise(): boolean`  
判断this中保存的动态对象是否是promise对象。

返回值：

|   类型    |            说明             |
| :-------: | :-------------------------: |
|  boolean | true 表示是Promise对象 |

示例代码：
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
获取this保存的动态对象中的迭代器对象，迭代的值为ESValue包装的动态对象。

返回值：

|            类型             |                    说明                    |
| :-------------------------: | :----------------------------------------: |
| IterableIterator<ESValue> | 实现ESValue对象内部成员的可迭代迭代器接口 |

示例代码：
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
