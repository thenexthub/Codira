# STValue

## 介绍

STValue作为一个封装类，它主要提供了一系列能够在ArkTS-Dyn中调用和操作来自静态类型ArkTS-Sta中数据的接口。

在ArkTS-Dyn动态运行中实现的STValue对象会保存一个指向静态(ArkTS-Sta)对象的引用，通过这个引用就可以操作对应ArkTS-Sta内对象的值。这一流程如下图所示。

<img src='data:image/png;base64,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'/>

---

`STValue`一共提供了`accessor`、`check`、`instance`、`invoke`、`unwrap`和`wrap`六种类型接口，其中：

- `accessor`提供对ArkTS-Sta对象属性、数组元素和命名空间变量的访问接口。
- `check`提供了基本类型与引用类型的类型检查的接口。
- `instance`提供了实例创建、查找类型和继承关系的接口。
- `invoke`提供了ArkTS-Sta对象方法、类静态方法和命名空间函数动态调用相关的接口。
- `wrap`提供了将ArkTS-Dyn值转换为ArkTS-Sta值并封装为STValue对象的接口。
- `unwrap`提供了将STValue对象反解为ArkTS-Dyn值的接口。

---

### 类型枚举SType

STValue部分接口需要指定操作的ArkTS-Sta类型。为此我们提供了类型枚举`SType`：

|  枚举名   |                       说明                       |
| :-------: | :----------------------------------------------: |
|  BOOLEAN  |        布尔类型，对应ArkTS-Sta中的boolean        |
|   CHAR    |         字符类型，对应ArkTS-Sta中的char          |
|   BYTE    |         字节类型，对应ArkTS-Sta中的byte          |
|   SHORT   |        短整数类型，对应ArkTS-Sta中的short        |
|    INT    |          整数类型，对应ArkTS-Sta中的int          |
|   LONG    |        长整数类型，对应ArkTS-Sta中的long         |
|   FLOAT   |     单精度浮点数类型，对应ArkTS-Sta中的float     |
|  DOUBLE   | 双精度浮点数类型，对应ArkTS-Sta中的double/number |
| REFERENCE |         引用类型，对应ArkTS-Sta中的引用          |
|   VOID    |          无类型，对应ArkTS-Sta中的void           |

---
### 引用STValue

目前可以通过以下方式获取STValue以及SType:

```typescript
let STValue = globalThis.Panda.STValue;
let SType = globalThis.Panda.SType;
```
---
### 名称修饰符（Mangling）规则

Mangling 是一种对函数签名进行的特殊编码处理方法，通过对参数类型和返回类型进行编码来区分重载函数，从而将函数名编码为唯一符号。其格式为`参数类型:返回类型`。

**函数编码示例：**
- `toInt(b: boolean): int` → `z:i`
- `toString(i: int): string` → `i:C{std.core.String}`

ArkTS中常用类型的**类型Mangling参考**如下所示：
- `boolean` → `z`
- `byte`→ `b`
- `char` → `c`
- `short` → `s`
- `int` → `i`
- `long` → `l`
- `float` → `f`
- `double` → `d`
- `number` → `d`
- `string` → `C{std.core.String}`
- `bigint` → `C{std.core.BigInt}`
- `Array`|`int[]` →	`C{std.core.Array}`
- `FixedArray<int>` → `A{i}`
- `null` →	`C{std.core.Object}`

**Mangling规则：**

1. **分隔参数和返回类型**
   - 使用 `:` 来分隔参数和返回类型，例如 `zz:i`（传入参数为两个布尔值，返回一个整数值，即`(boolean, boolean): int`）。
   - 如果是`void`返回类型则可以写成 `i:`（传入参数为一个整数值，返回`void`，即`(int): void`）。
2. **对象格式**
   - 格式：`C{<模块>.<类>}`，如果没有显式声明模块名，默认模块名是文件名。
3. **数组格式**
   - 一维数组：`A{元素类型}`。
   - 多维数组：每增加一维就嵌套一个 `A`，例如 `A{A{i}}`。
4. **其它类型**
   - 泛型类型映射到类型约束。默认类型约束是 `Object` |` null` | `undefined`，在签名中对应 `C{std.core.Object}`。
   - 联合类型映射到**最小上界**类型：`function foo(a: string | number): void` → `"C{std.core.Object}:"`。
5. **可选参数**
   - 可选的基本类型会变成装箱对象：`arg?: int` → `C{std.core.Int}`。
   - 非基本类型的可选类型保持不变
6. **函数作为参数**
   - 使用`C{std.core.FunctionN}`，其中`N`是参数数量，例如：`()=>void` → `C{std.core.Function0}`。
   - 使用`C{std.core.FunctionRN}`，其中`R`表示函数带有剩余参数，例如：`(...args: double[])=>void` → `C{std.core.FunctionR0}`。

**使用示例：**

在STValue提供的接口中，`classInstantiate`、`namespaceInvokeFunction`、`objectInvokeMethod`以及`classInvokeStaticMethod`都涉及到Mangling的函数签名规则。

以`namespaceInvokeFunction`为例的Mangling的函数签名示例如下所示：
```typescript
// ArkTS-Dyn
let nsp = STValue.findNamespace('stvalue_invoke.Invoke');
let b1 = STValue.wrapBoolean(false);
let b2 = STValue.wrapBoolean(false);
let b = nsp.namespaceInvokeFunction('BooleanInvoke', 'zz:z', [b1, b2]); // BooleanInvoke(b1: boolean, b2: boolean): boolean
```
---
### 其它注意事项

**1.如何通过STValue调用Ark-Sta内的处理静态数据的JSON.stringify()方法并将序列化的结果返回?**

简单示例如下所示，其中主要步骤包括：
- 通过命名空间获取实例变量student
- 通过`findClass()`来定位获取JSON类
- 通过`classInvokeStaticMethod()`调用了JSON类的静态方法`stringify()`来转换对应Ark-Sta中的实例对象student，其中`stringify()`结果是包装了String类型的STValue对象
- 最后将返回的包装对象拆包为String类型。

```typescript
// ArkTS-Dyn
let tns = STValue.findNamespace('stvalue_test.testNameSpace');
let student = tns.namespaceGetVariable('student', SType.REFERENCE);
let JOSNCls = STValue.findClass("escompat.JSON");
// 调用stringify，函数签名表示入参为Object类型，返回值为String类型
let stringfyRes = JSONCls.classInvokeStaticMethod('stringify', 'C{std.core.Object}:C{std.core.String}', [student]);
let str = stringfyRes.unwrapToString(); // {"name":"student"}
```

```typescript
// stvalue_test.ets ArkTS-Sta
export class Student {
    name: string;
    constructor(n: string) {
        this.name = n;
    }
}
export namespace testNameSpace {
    export let student: Student = new Student('student');
}
```
**2.SType使用注意事项**

在STValue中使用接口的时候会涉及到SType的使用，需要注意的是，SType的枚举值需要与对应的类型相对应，例如在`enumGetValueByName(memberName: string, valueType: SType)`（通过枚举名获取枚举值）中，第二个入参需要传入与枚举值相同的类型，如果枚举值为int，则应该传入**SType.INT**。同时如果SType对应的值为引用类型，如String，Array以及类实例等，则对应类型则为**SType.REFERENCE**。

---
## 1 STValue_accessor

### 1.1 findClass

`static findClass(desc: string): STValue`

用于根据ArkTS-Sta类名查找类定义，接受一个类的全限定路径字符串作为参数，返回封装了该类的STValue对象。

**参数：**

| 参数名 |  类型  | 必填 |     说明     |
| :----: | :----: | :--: | :----------: |
|  desc  | string |  是  | 类全限定路径 |

**返回值：**

|  类型   |          说明           |
| :-----: | :---------------------: |
| STValue | 封装了该类的STValue对象 |


**示例：** 

```typescript
// ArkTS-Dyn
let studentCls = STValue.findClass('stvalue_accessor.Student');
```

```typescript
// stvalue_accessor.ets ArkTS-Sta
export class Student {}
```
**报错异常：**

当调用时传入了的参数数量不为1时，产生编译错误`参数数量错误`；当传入的路径不符合类描述符规范时（例如类名格式错误，类名包含非法字符等），抛出运行错误`类名无效`；默认类加载器未找到类时，抛出运行错误`查找失败`；遇到除`查找失败`之外的错误时，抛出其它类型的运行错误异常。

示例：
```typescript
try{
    // Invalid Class
    let klass = STValue.findClass('stvalue_accessor/Animal');
}catch (e: Error){
    // Throw Error 
    console.log(e.message); 
}
// FindClass failed, invalid className: className=stvalue_accessor/Animal, aniStatus=4
```
---

### 1.2 findNamespace

`static findNamespace(desc: string): STValue`

用于根据名称查找命名空间，接受一个字符串参数（命名空间的全限定路径），返回表示该命名空间的STValue对象。

**参数：**

| 参数名 |  类型  | 必填 |        说明        |
| :----: | :----: | :--: | :----------------: |
|  desc  | string |  是  | 命名空间全限定路径 |

**返回值：**

|  类型   |            说明             |
| :-----: | :-------------------------: |
| STValue | 代表该命名空间的STValue对象 |

**示例：**

```typescript
// ArkTS-Dyn
let ns = STValue.findNamespace('stvalue_accessor.MyNamespace');
```

```typescript
// stvalue_accessor.ets ArkTS-Sta
export namespace MyNamespace {}
```
**报错异常：**

当调用时传入了的参数数量不为1时，产生编译错误`参数数量错误`；当传入的命名空间路径不符合规范时（例如格式错误，命名空间名包含非法字符等），抛出运行错误`命名空间名无效`;未找到命名空间时，抛出运行错误`查找失败`；遇到除`查找失败`之外的错误时，抛出其它类型的运行错误异常。

示例：
```typescript
try{
    // Invalid Namespace
    let exampleNs = STValue.findNamespace('stvalue_accessor.Namespace#'); 
}catch (e: Error){
    // Throw Error 
    console.log(e.message);  
}
// FindNamespace failed, invalid namespaceName: namespaceName=stvalue_accessor.Namespace#, aniStatus=4
```
---

### 1.3 findEnum

`static findEnum(desc: string): STValue`

用于根据名称查找枚举类型定义，接受一个字符串参数（枚举的全限定路径），返回表示该枚举的STValue对象。如果枚举不存在或参数错误，会抛出异常。

**参数：**

| 参数名 |  类型  | 必填 |      说明      |
| :----: | :----: | :--: | :------------: |
|  desc  | string |  是  | 枚举全限定路径 |

**返回值：**
|  类型   |          说明           |
| :-----: | :---------------------: |
| STValue | 代表该枚举的STValue对象 |

**示例：**

```typescript
// ArkTS-Dyn
let colorEnum = STValue.findEnum('stvalue_accessor.COLOR');
```

```typescript
// stvalue_accessor.ets ArkTS-Sta
export enum COLOR {
    Red,
    Green
}
```

**报错异常：**

当调用时传入了的参数数量不为1时，产生编译错误`参数数量错误`；当传入的枚举路径不符合规范（例如格式错误，枚举名包含非法字符等），抛出运行错误`枚举名无效`;未找到枚举时，抛出运行错误 `查找失败`；遇到除`查找失败`之外的错误时，抛出其它类型运行错误异常。

示例：
```typescript
try{
    // Invalid Enum
    let testEnum = STValue.findEnum('stvalue_accessor.COLOR#'); 
}catch (e: Error){
    // Throw Error 
    console.log(e.message);
}
// FindEnum failed, invalid enumName: enumName=stvalue_accessor.COLOR#, aniStatus=4
```
---

### 1.4 classGetSuperClass

`classGetSuperClass(): STValue | null`

用于获取类的父类定义，不接受任何参数，返回表示父类的STValue对象。如果当前类没有父类（如Object类），会返回null；如果`this`不是类，则会抛出异常。

**参数：** 无

**返回值：**

|  类型   |         说明          |
| :-----: | :-------------------: |
| STValue | 代表父类的STValue对象 |

**示例：**

```typescript
// ArkTS-Dyn
let subClass = STValue.findClass('stvalue_accessor.Dog');
let parentClass = subClass.classGetSuperClass();  // 代表父类'stvalue_accessor.Animal'的STValue对象
```

```typescript
// stvalue_accessor.ets ArkTS-Sta
export class Animal {
  static name: string = 'Animal';
}
export class Dog extends Animal {
  static name: string = 'Dog';
}
```

**报错异常：**

当调用时传入的参数数量不为0时，产生编译错误`参数数量错误`；当`this`不是有效的对象时，抛出运行错误`非法实例对象`；遇到其它错误时，抛出其它类型的运行错误异常。

示例：
```typescript
try {
    // Invalid Instance
    let nonClass = STValue.wrapInt(111);
    let resClass = nonClass.classGetSuperClass();
} catch (e: Error) {
    // Throw Error
    console.log(e.message);
}
// 'this' STValue instance does not wrap a value of type reference
```
---

### 1.5 fixedArrayGetLength

`fixedArrayGetLength(): number`

用于获取固定长度数组的元素数量，不接受任何参数，返回表示数组长度的数字值。如果调用对象不是固定长度数组类型，会抛出异常。

**参数：** 无

**返回值：**

|  类型  |   说明   |
| :----: | :------: |
| number | 数组长度 |

**示例：**

```typescript
// ArkTS-Dyn
let tns = STValue.findNamespace('stvalue_accessor.testNameSpace')
let strArray = tns.namespaceGetVariable('strArray', SType.REFERENCE);
let arrayLength = strArray.fixedArrayGetLength(); // 3
```

```typescript
// stvalue_accessor.ets ArkTS-Sta
export namespace testNameSpace {
    export let strArray: FixedArray<string> = ['ab', 'cd', 'ef'];
}
```

**报错异常：**

当调用时传入的参数数量不为0时，产生编译错误`参数数量错误`；当`this`不是有效的固定数组对象时，抛出运行错误`非法实例对象`；遇到其它错误时，抛出其它类型的运行错误异常。

示例：
```typescript
try {
    // Invalid Instance
    STValue.wrapInt(1).fixedArrayGetLength();
} catch (e: Error) {
    // Throw Error
    console.log(e.message);
}
// 'this' STValue instance does not wrap a value of type reference
```
---

### 1.6 enumGetIndexByName

`enumGetIndexByName(memberName: string): number`

用于根据枚举成员名称查询其在枚举中的索引位置，接受一个字符串参数（成员名称），返回表示该成员索引的数字值。如果成员不存在或参数错误，会抛出异常。


**参数：**

| 参数名 |  类型  | 必填 |   说明   |
| :----: | :----: | :--: | :------: |
|  memberName  | string |  是  | 枚举名称 |

**返回值：**

|  类型  |   说明   |
| :----: | :------: |
| number | 枚举索引 |

**示例：**

```typescript
// ArkTS-Dyn
let colorEnum = STValue.findEnum('stvalue_accessor.COLOR');
let redIndex = colorEnum.enumGetIndexByName('Red'); // 0
```

```typescript
// stvalue_accessor.ets ArkTS-Sta
export enum COLOR{
    Red = 1,
    Green = 3
}
```
**报错异常：**

当调用时传入的参数数量不为1时，产生编译错误`参数数量错误`；当传入的参数不是有效字符串时，抛出运行错误`参数类型错误`；当`this`不是有效的枚举对象时，抛出运行错误`非法实例对象`；未找到对应的枚举项时，抛出运行错误`枚举不存在`；遇到其它错误时，抛出其它类型的运行错误异常。

示例：
```typescript
try {
    // Invalid Argument Type
    colorEnum.enumGetIndexByName(1);
} catch (e: Error) {
    // Throw Error
    console.log(e.message);
}
// param are not string type; param=memberName
```
---

### 1.7 enumGetValueByName

`enumGetValueByName(memberName: string, valueType: SType): STValue`

用于根据成员名称获取枚举成员的值，接受两个参数：成员名称和值类型，返回对应值的STValue对象，其中对应传入的类型枚举SType需符合枚举成员的值类型。支持整型和字符串类型的枚举值，如果成员不存在、类型不匹配或参数错误，会抛出异常。

**参数：**

|  参数名   |  类型  | 必填 |    说明    |
| :-------: | :----: | :--: | :--------: |
|   memberName    | string |  是  |  枚举名称  |
| valueType | SType  |  是  | 枚举值类型 |

**返回值：**

|  类型   |        说明         |
| :-----: | :-----------------: |
| STValue | 枚举值的STValue对象 |

**示例：**

```typescript
// ArkTS-Dyn
let colorEnum = STValue.findEnum('stvalue_accessor.COLOR');  
let redValue = colorEnum.enumGetValueByName('Red', SType.INT);
// 获取的枚举值是一个STValue对象，需要拆箱获取对应primitive值
let unwrapRedValue = redValue.unwrapToNumber(); // 1
```

```typescript
// stvalue_accessor.ets ArkTS-Sta
export enum COLOR{
    Red = 1,
    Green = 3,
}
```

**报错异常：**

当调用时传入的参数数量不为2时，产生编译错误`参数数量错误`；当传入的参数不是有效对应类型时，抛出运行错误`参数类型错误`；当`this`不是有效的枚举对象时，抛出运行错误`非法实例对象`；未找到对应的枚举项时，抛出运行错误`枚举不存在`；遇到其它错误时，抛出其它类型的运行错误异常。

示例：
```typescript
try {
    // Invalid Object
    let nullEnum = STValue.getNull();
    nullEnum.enumGetValueByName('Red', SType.REFERENCE);
} catch (e: Error) {
    // Throw Error
    console.log(e.message);
}
// 'this' STValue instance does not wrap a value of type reference
```
---

### 1.8 classGetStaticField

`classGetStaticField(fieldName: string, fieldType: SType): STValue`

用于获取类的静态字段值，接受两个参数：字段名称和字段类型，返回对应值的STValue对象，其中对应传入的类型枚举SType需符合字段值类型。如果字段不存在、类型不匹配或参数错误，会抛出异常。

**参数：**

|  参数名   |  类型  | 必填 |   说明   |
| :-------: | :----: | :--: | :------: |
| fieldName | string |  是  | 字段名称 |
| fieldType | SType  |  是  | 字段类型 |

**返回值：**

|  类型   |        说明         |
| :-----: | :-----------------: |
| STValue | 字段值的STValue对象 |

**示例：**

```typescript
// ArkTS-Dyn
let personClass = STValue.findClass('stvalue_accessor.Person');
let name = personClass.classGetStaticField('name', SType.REFERENCE).unwrapToString(); // 'Person'
let age = personClass.classGetStaticField('age', SType.INT).unwrapToNumber();  // 18
```

```typescript
// stvalue_accessor.ets ArkTS-Sta
export class Person {
    static name: string = 'Person';
    static age: number = 18;
}
```
**报错异常：**

当调用时传入的参数数量不为2时，产生编译错误`参数数量错误`；当传入的参数不是有效对应类型时，抛出运行错误`参数类型错误`；当`this`不是有效的类对象时，抛出运行错误`非法实例对象`；未找到对应的字段时，抛出运行错误`字段不存在`；遇到其它错误时，抛出其它类型的运行错误异常。

示例：
```typescript
try {
    // Invalid Instance
    let nonClass = STValue.wrapInt(111);
    nonClass.classGetStaticField('male', SType.BOOLEAN);
} catch (e: Error) {
    // Throw Error
    console.log(e.message);
}
// 'this' STValue instance does not wrap a value of type reference
```
---

### 1.9 classSetStaticField

`classSetStaticField(fieldName: string, val: STValue, fieldType: SType): void`

用于设置类的静态字段值，接受三个参数：字段名称、要设置的值和字段类型）。主要作用是修改类的静态成员变量，支持基本类型和引用类型，其中对应传入的类型枚举SType需符合字段值类型。如果类型不匹配、字段不存在或参数错误，会抛出异常。

**参数：**

|  参数名   |  类型   | 必填 |    说明    |
| :-------: | :-----: | :--: | :--------: |
| fieldName | string  |  是  |  字段名称  |
|    val    | STValue |  是  | 要设置的值 |
| fieldType |  SType  |  是  |  字段类型  |

**返回值：** 无

**示例：**

```typescript
// ArkTS-Dyn
let personClass = STValue.findClass('stvalue_accessor.Person');
personClass.classSetStaticField('name', STValue.wrapString('Bob'), SType.REFERENCE);
personClass.classSetStaticField('age', STValue.wrapNumber(21), SType.DOUBLE);
let name = personClass.classGetStaticField('name', SType.REFERENCE).unwrapToString(); // 'Bob'
let age = personClass.classGetStaticField('age', SType.DOUBLE).unwrapToNumber(); // 21
```

```typescript
// stvalue_accessor.ets ArkTS-Sta
export class Person {
    static name: string = 'Person';
    static age: number = 18;
}
```

**报错异常：**

当调用时传入的参数数量不为3时，产生编译错误`参数数量错误`；当传入的参数不是有效对应类型时，抛出运行错误`参数类型错误`；当`this`不是有效的类对象时，抛出运行错误`非法实例对象`；当val参数类型和fieldType字段类型不匹配时，抛出运行错误`不匹配的字段类型`；未找到对应的字段时，抛出运行错误`字段不存在`；遇到其它错误时，抛出其它类型的运行错误异常。

示例：
```typescript
try {
    // Invalid Type
    personClass.classSetStaticField('male', STValue.wrapBoolean(false), SType.INT);
} catch (e: Error) {
    // Throw Error
    console.log(e.message);
}
// value STValue instance does not wrap a value of type int
```
---

### 1.10 objectGetProperty

`objectGetProperty(propName: string, propType: SType): STValue`

用于获取对象实例的属性值，接受两个参数：属性名称和属性类型，返回对应值的STValue对象。主要作用是访问对象的成员变量，支持基本类型和引用类型，其中对应传入的类型枚举SType需符合实例属性值类型。如果属性不存在、类型不匹配或参数错误，会抛出异常。


**参数：**

|  参数名  |  类型  | 必填 |   说明   |
| :------: | :----: | :--: | :------: |
| propName | string |  是  | 属性名称 |
| propType | SType  |  是  | 属性类型 |

**返回值：**

|  类型   |        说明         |
| :-----: | :-----------------: |
| STValue | 属性值的STValue对象 |

**示例：**

```typescript
// ArkTS-Dyn
let tns = STValue.findNamespace('stvalue_accessor.testNameSpace')
let alice = tns.namespaceGetVariable('studentAlice', SType.REFERENCE);
let name = alice.objectGetProperty('name', SType.REFERENCE).unwrapToString(); // 'Alice'
```

```typescript
// stvalue_accessor.ets ArkTS-Sta
export class Student {
    name: string;
    constructor(n: string) {
        this.name = n;
    }
}

export namespace testNameSpace {
    export let studentAlice: Student = new Student('Alice');
}
```

**报错异常：**

当调用时传入的参数数量不为2时，产生编译错误`参数数量错误`；当传入的参数不是有效对应类型时，抛出运行错误`参数类型错误`；当`this`不是有效的实例对象时，抛出运行错误`非法实例对象`；未找到对应的属性时，抛出运行错误`属性不存在`；属性类型不匹配时，抛出运行错误`不匹配的属性类型`；遇到其它错误时，抛出其它类型的运行错误异常。

示例：
```typescript
try {
    // Invalid Instance
    let priObject = STValue.wrapInt(111);
    priObject.objectGetProperty('name', SType.REFERENCE);
} catch (e: Error) {
    // Throw Error
    console.log(e.message);
}
// 'this' STValue instance does not wrap a value of type reference
```
---

### 1.11 objectSetProperty

`objectSetProperty(propName: string, val: STValue, propType: SType): void`

用于设置对象实例的属性值，接受三个参数：属性名称、要设置的值和属性类型。主要作用是修改对象的成员变量，支持基本类型和引用类型，其中对应传入的类型枚举SType需符合实例属性值类型。如果类型不匹配、属性不存在或参数错误，会抛出异常。

**参数：**

|  参数名  |  类型   | 必填 |    说明    |
| :------: | :-----: | :--: | :--------: |
| propName | string  |  是  |  属性名称  |
|   val    | STValue |  是  | 要设置的值 |
| propType |  SType  |  是  |  属性类型  |

**返回值：** 无

**示例：**

```typescript
// ArkTS-Dyn
let tns = STValue.findNamespace('stvalue_accessor.testNameSpace')
let alice = tns.namespaceGetVariable('studentAlice', SType.REFERENCE);
let name = alice.objectSetProperty('name', STValue.wrapString('Bob'), SType.REFERENCE);
let name = alice.objectGetProperty('name', SType.REFERENCE).unwrapToString(); // 'Bob'
```

```typescript
// stvalue_accessor.ets ArkTS-Sta
export class Student {
    name: string;
    constructor(n: string) {
        this.name = n;
    }
}

export namespace testNameSpace {
    export let studentAlice: Student = new Student('Alice');
}
```

**报错异常：**

当调用时传入的参数数量不为3时，产生编译错误`参数数量错误`；当传入的参数不是有效对应类型时，抛出运行错误`参数类型错误`；当`this`不是有效的实例对象时，抛出运行错误`非法实例对象`；未找到对应的属性时，抛出运行错误`属性不存在`；属性类型不匹配时，抛出运行错误`不匹配的属性类型`；遇到其它错误时，抛出其它类型的运行错误异常。

示例：
```typescript
try {
    // Invalid Type
    alice.objectSetProperty('name', STValue.wrapNumber(111), SType.REFERENCE);
} catch (e: Error) {
    // Throw Error
    console.log(e.message);
}
// value STValue instance does not wrap a value of type reference
```
---

### 1.12 fixedArrayGet

`fixedArrayGet(idx: number, elementType: SType): STValue`

用于获取固定长度数组中指定索引的元素值，接受两个参数：索引位置和元素类型，返回对应值的STValue对象。主要作用是访问数组元素，支持基本类型和引用类型，其中对应传入的类型枚举SType需符合数组元素类型。如果索引越界、类型不匹配或参数错误，会抛出异常。

**参数：**

|   参数名    |  类型  | 必填 |     说明     |
| :---------: | :----: | :--: | :----------: |
|     idx     | number |  是  |   数组索引   |
| elementType | SType  |  是  | 数组元素类型 |

**返回值：**

|  类型   |       说明        |
| :-----: | :---------------: |
| STValue | 元素的STValue对象 |

**示例：**

```typescript
// ArkTS-Dyn
let tns = STValue.findNamespace('stvalue_accessor.testNameSpace');
let strArray = tns.namespaceGetVariable('strArray', SType.REFERENCE);
let str = strArray.fixedArrayGet(1, SType.REFERENCE).unwrapToString(); // 'cd'
```

```typescript
// stvalue_accessor.ets ArkTS-Sta
export namespace testNameSpace {
    export let strArray: FixedArray<string> = ['ab', 'cd', 'ef'];
}
```
**报错异常：**

当调用时传入的参数数量不为2时，产生编译错误`参数数量错误`；当传入的参数不是有效对应类型时，抛出运行错误`参数类型错误`；当`this`不是有效的数组对象时，抛出运行错误`非法实例对象`；当索引idx超出数组范围时，抛出运行错误`索引越界`；数组元素类型不匹配时，抛出运行错误`不匹配的元素类型`；遇到其它错误时，抛出其它类型的运行错误异常。

示例：
```typescript
try {
    // Invalid Instance
    let notArray = STValue.wrapInt(111);  
    notArray.fixedArrayGet(-1, SType.REFERENCE);
} catch (e: Error) {
    // Throw Error
    console.log(e.message);
}
// 'this' STValue instance does not wrap a value of type reference
```
---

### 1.13 fixedArraySet

`fixedArraySet(idx: number, val: STValue, elementType: SType): void`

用于设置固定长度数组中指定索引的元素值，接受三个参数：索引位置、要设置的值和元素类型。主要作用是修改数组元素，支持基本类型和引用类型，其中对应传入的类型枚举SType需符合数组元素类型。如果类型不匹配、索引越界或参数错误，会抛出异常。

**参数：**

|   参数名    |  类型   | 必填 |     说明     |
| :---------: | :-----: | :--: | :----------: |
|     idx     | number  |  是  |   数组索引   |
|     val     | STValue |  是  |  要设置的值  |
| elementType |  SType  |  是  | 数组元素类型 |

**返回值：** 无

**示例：**

```typescript
// ArkTS-Dyn
let tns = STValue.findNamespace('stvalue_accessor.testNameSpace');
let strArray = tns.namespaceGetVariable('strArray', SType.REFERENCE);
strArray.fixedArraySet(1, STValue.wrapString('xy'), SType.REFERENCE);
let str = strArray.fixedArrayGet(1, SType.REFERENCE).unwrapToString(); // 'xy'
```

```typescript
// stvalue_accessor.ets ArkTS-Sta
export namespace testNameSpace {
    export let strArray: FixedArray<string> = ['ab', 'cd', 'ef'];
}
```

**报错异常：**

当调用时传入的参数数量不为3时，产生编译错误`参数数量错误`；当传入的参数不是有效对应类型时，抛出运行错误`参数类型错误`；当`this`不是有效的数组对象时，抛出运行错误`非法实例对象`；当索引idx超出数组范围时，抛出运行错误`索引越界`；数组元素类型不匹配或值不匹配时，抛出运行错误`不匹配的元素类型`；遇到其它错误时，抛出其它类型的运行错误异常。

示例：
```typescript
try {
    // Invalid Instance
    let notArray = STValue.wrapInt(111);
    notArray.fixedArraySet(0, STValue.getNull(), SType.REFERENCE);
} catch (e: Error) {
    // Throw Error
    console.log(e.message);
}
// 'this' STValue instance does not wrap a value of type reference
```
---

### 1.14 namespaceGetVariable

`namespaceGetVariable(variableName: string, variableType: SType): STValue | null`

用于获取命名空间中的变量值，接受两个参数：变量名称和变量类型，返回对应值的STValue对象。主要作用是访问命名空间中的全局变量，支持基本类型和引用类型，其中对应传入的类型枚举SType需符合对应变量类型。如果变量不存在、类型不匹配或参数错误，会抛出异常。


**参数：**

|    参数名    |  类型  | 必填 |      说明      |
| :----------: | :----: | :--: | :------------: |
|     variableName     | string |  是  |    变量名称    |
| variableType | SType  |  是  | 变量类型枚举值 |

**返回值：**

|  类型   |        说明         |
| :-----: | :-----------------: |
| STValue | 变量值的STValue对象 |

**示例：**

```typescript
// ArkTS-Dyn
let ns = STValue.findNamespace('stvalue_accessor.MyNamespace');
let data = ns.namespaceGetVariable('data', SType.INT);
let num = data.unwrapToNumber();  // 42
```

```typescript
// stvalue_accessor.ets ArkTS-Sta
export namespace MyNamespace {
    export let data: int = 42;
}
```

**报错异常：**

当调用时传入的参数数量不为2时，产生编译错误`参数数量错误`；当传入的参数不是有效对应类型时，抛出运行错误`参数类型错误`；当`this`不是有效的命名空间时，抛出运行错误`无效的命名空间`；当变量与变量类型不匹配时，抛出运行错误`不匹配的变量类型`；当变量不存在时，抛出运行错误`变量不存在`；遇到其它错误时，抛出其它类型的运行错误异常。

示例：
```typescript
try {
    // Invalid Type
    ns.namespaceGetVariable(1, SType.INT)
} catch (e: Error) {
    // Throw Error
    console.log(e.message);
}
// param are not string type; param=variableName
```
---

### 1.15 namespaceSetVariable

`namespaceSetVariable(variableName: string, value: STValue, variableType: SType): boolean`

用于设置命名空间中的变量值，接受三个参数：变量名称、要设置的值和变量类型，返回操作是否成功的布尔值。主要作用是修改命名空间中的全局变量，支持基本类型和引用类型，其中对应传入的类型枚举SType需符合对应变量类型。如果变量不存在、类型不匹配或参数错误，会抛出异常。

**参数：**

|    参数名    |  类型   | 必填 |      说明      |
| :----------: | :-----: | :--: | :------------: |
|     variableName     | string  |  是  |    变量名称    |
|    value     | STValue |  是  |   要设置的值   |
| variableType |  SType  |  是  | 变量类型枚举值 |

**返回值：**

|  类型   |              说明               |
| :-----: | :-----------------------------: |
| boolean | 设置成功返回true，失败返回false |

**示例：**

```typescript
// ArkTS-Dyn
let ns = STValue.findNamespace('stvalue_accessor.MyNamespace');
ns.namespaceSetVariable('data', STValue.wrapInt(0), SType.INT);
let data = ns.namespaceGetVariable('data', SType.INT);
let num = data.unwrapToNumber();  // 0
```

```typescript
// stvalue_accessor.ets ArkTS-Sta
export namespace MyNamespace {
    export let data: int = 42;
}
```

**报错异常：**

当调用时传入的参数数量不为3时，产生编译错误`参数数量错误`；当传入的参数不是有效对应类型时，抛出运行错误`参数类型错误`；当`this`不是有效的命名空间时，抛出运行错误`无效的命名空间`；当设置值与变量类型不匹配时，抛出运行错误`不匹配的变量类型`；当变量不存在时，抛出运行错误`变量不存在`；遇到其它错误时，抛出其它类型的运行错误异常。

示例：
```typescript
try {
    // Invalid Type
    ns.namespaceSetVariable(1, STValue.wrapInt(44), SType.INT);
} catch (e: Error) {
    // Throw Error
    console.log(e.message);
}
// param are not string type; param=variableName
```
---

### 1.16 objectGetType

`objectGetType(): STValue`

用于获取引用类型对象的类型信息，不接受任何参数，返回表示对象类型的STValue对象。如果`this`包装的不是ArkTS-Sta对象引用，会抛出异常。

**参数：** 无

**返回值：**

|  类型   |         说明          |
| :-----: | :-------------------: |
| STValue | 类型信息的STValue对象 |

**示例：**

```typescript
// ArkTS-Dyn
let strWrap = STValue.wrapString('hello world');
let strType = strWrap.objectGetType();
let isString = strWrap.objectInstanceOf(strType); // true
```

**报错异常：**

当调用时传入的参数数量不为0时，产生编译错误`参数数量错误`；当`this`不是有效的引用类型对象时，抛出运行错误`无效实例对象`；遇到其它错误时，抛出其它类型的运行错误异常。

示例：
```typescript
try {
    // Invalid Instance
    STValue.getUndefined().objectGetType();
} catch (e: Error) {
    // Throw Error
    console.log(e.message);
}
// 'this' STValue instance does not wrap a value of type reference
```
---


### 1.17 arrayGet

`arrayGet(idx: number): STValue`

用于获取动态数组指定索引位置的元素，并将其封装为STValue对象返回，调用该方法的对象需要是有效的STValue引用类型数组对象。

**参数：** 

| 参数名 |  类型  | 必填 |      说明      |
| :----: | :----: | :--: | :------------: |
|  idx   | number |  是  | 数组元素的索引 |

**返回值：**

|  类型   |              说明              |
| :-----: | :----------------------------: |
| STValue | 封装了指定位置元素的STValue对象 |

**示例：**

```typescript
// Ark-Dyn
let tns = STValue.findNamespace('stvalue_accessor.testNameSpace')
let intArray = tns.namespaceGetVariable('intArray', SType.REFERENCE);
// 返回值为封装了Int类型的STValue对象
let arrayValue0 = intArray.arrayGet(0);
let num = arrayValue0.objectInvokeMethod('toInt', ':i', []).unwrapToNumber(); // 1
```

```typescript
// Ark-Sta
export namespace testNameSpace {
    export let intArray: Array<int> = [1, 2, 3, 4, 5];
}
```

**报错异常：**
当调用时传入参数数量不为1时，产生编译错误`参数数量错误`；当调用对象`this`非有效的动态数组引用或为空时，抛出运行错误`调用对象非引用类型`；遇到其它错误时，抛出其它类型的运行错误异常。

示例：
```typescript
try {
    // Invalid Arguments Number
    intArray.arrayGet(0, SType.INT);
} catch (e: Error) {
    // Throw Error
    console.log(e.message);
}
// ArkTS Compiler Error
// Expect 1 args, but got 2 args
```
---

### 1.18 arraySet

`arraySet(idx: number, val: STValue): void`

用于设置动态数组指定索引位置的元素值，调用该方法的对象需要是有效的STValue引用类型数组对象，同时设置的数组值`val`需要是封装了引用类型对象的STValue对象。

**参数：** 

| 参数名 |  类型   | 必填 |          说明           |
| :----: | :-----: | :--: | :---------------------: |
|  idx   | number  |  是  |     要设置的数组索引    |
|  val   | STValue |  是  | 要存入的STValue封装对象 |

**返回值：** 无

**示例：**

```typescript
// Ark-Dyn
let tns = STValue.findNamespace('stvalue_accessor.testNameSpace')
let intArray = tns.namespaceGetVariable('intArray', SType.REFERENCE);
let intClass = STValue.findClass('std.core.Int');
let intObj = intClass.classInstantiate('i:', [STValue.wrapInt(10)]);
// 传入参数需要是有效的Int引用类型对象
intArray.arraySet(0, intObj);
let arrayValue0 = intArray.arrayGet(0);
let num = arrayValue0.objectInvokeMethod('toInt', ':i', []).unwrapToNumber(); // 10
```

```typescript
// Ark-Sta
export namespace testNameSpace {
    export let intArray: Array<int> = [1, 2, 3, 4, 5];
}
```

**报错异常：**

当调用时传入参数数量不为2时，产生编译错误`参数数量错误`；当`val`参数不是合法的STValue引用时，抛出运行错误`参数类型错误`；当调用对象`this`非有效的动态数组引用或为空时，抛出运行错误`调用对象非引用类型`；遇到其它错误时，抛出其它类型的运行错误异常。


示例：
```typescript
try {
    // Invalid Arguments Number
    intArray.arraySet(0);
} catch (e: Error) {
    // Throw Error
    console.log(e.message);
}
// ArkTS Compiler Error
// Expect 2 args, but got 1 args
```
---

### 1.19 arrayPush

`arrayPush(val: STValue): void`

在动态数组的末尾添加一个新元素，调用该方法的对象需要是有效的STValue引用类型数组对象，同时新添加的数组值`val`需要是封装了引用类型对象的STValue对象。

**参数：** 

| 参数名 |  类型   | 必填 |          说明           |
| :----: | :-----: | :--: | :---------------------: |
|  val   | STValue |  是  | 要添加的STValue封装对象 |

**返回值：** 无

**示例：**

```typescript
// Ark-Dyn
let tns = STValue.findNamespace('stvalue_accessor.testNameSpace')
let intArray = tns.namespaceGetVariable('intArray', SType.REFERENCE);
let intClass = STValue.findClass('std.core.Int');
let intObj = intClass.classInstantiate('i:', [STValue.wrapInt(10)]);
// 传入参数需要是有效的Int引用类型对象
intArray.arrayPush(intObj);
let arrayValue0 = intArray.arrayGet(5);
let num = arrayValue0.objectInvokeMethod('toInt', ':i', []).unwrapToNumber(); // 10
```

```typescript
// Ark-Sta
export namespace testNameSpace {
    export let intArray: Array<int> = [1, 2, 3, 4, 5];
}
```

**报错异常：**

当调用时传入参数数量不为1时，产生编译错误`参数数量错误`；当`val`参数不是合法的STValue引用时，抛出运行错误`参数类型错误`；当调用对象`this`非有效的动态数组引用或为空时，抛出运行错误`调用对象非引用类型`；遇到其它错误时，抛出其它类型的运行错误异常。

示例：
```typescript
try {
    // Invalid Arguments Number
    intArray.arrayPush();
} catch (e: Error) {
    // Throw Error
    console.log(e.message);
}
// ArkTS Compiler Error
// Expect 1 args, but got 0 args
```
---

### 1.20 arrayPop

`arrayPop(): STValue`

移除并返回动态数组的最后一个元素，调用该方法的对象需要是有效的STValue引用类型数组对象。

**参数：** 无

**返回值：** 

|  类型   |              说明              |
| :-----: | :----------------------------: |
| STValue | 被移除的末尾元素的STValue对象 |

**示例：**

```typescript
// Ark-Dyn
let tns = STValue.findNamespace('stvalue_accessor.testNameSpace')
let intArray = tns.namespaceGetVariable('intArray', SType.REFERENCE);
let intPop = intArray.arrayPop();
let num = intPop.objectInvokeMethod('toInt', ':i', []).unwrapToNumber(); // 5
```

```typescript
// Ark-Sta
export namespace testNameSpace {
    export let intArray: Array<int> = [1, 2, 3, 4, 5];
}
```

**报错异常：**

当调用时传入参数数量不为0时，产生编译错误`参数数量错误`；当调用对象`this`非有效的动态数组引用或为空时，抛出运行错误`调用对象非引用类型`；遇到其它错误时，抛出其它类型的运行错误异常。

示例：
```typescript
try {
    // Invalid Arguments Number
    intArray.arrayPop(1);
} catch (e: Error) {
    // Throw Error
    console.log(e.message);
}
// ArkTS Compiler Error
// Expect 0 args, but got 1 args
```
---

### 1.21 arrayGetLength

`arrayGetLength(): number`

用于获取动态数组的长度，调用该方法的对象需要是有效的STValue引用类型数组对象。

**参数：** 无

**返回值：** 

|  类型  |          说明          |
| :----: | :--------------------: |
| number | 动态数组当前的元素个数 |

**示例：**

```typescript
// Ark-Dyn
let tns = STValue.findNamespace('stvalue_accessor.testNameSpace')
let intArray = tns.namespaceGetVariable('intArray', SType.REFERENCE);
let num = intArray.arrayGetLength(); // 5
```

```typescript
// Ark-Sta
export namespace testNameSpace {
    export let intArray: Array<int> = [1, 2, 3, 4, 5];
}
```

**报错异常：**

当调用时传入参数数量不为0时，产生编译错误`参数数量错误`；当调用对象`this`非有效的动态数组引用或为空时，抛出运行错误`调用对象非引用类型`；遇到其它错误时，抛出其它类型的运行错误异常。

---

示例：
```typescript
try {
    // Invalid Arguments Number
    let num = intArray.arrayGetLength(1);
} catch (e: Error) {
    // Throw Error
    console.log(e.message);
}
// ArkTS Compiler Error
// Expect 0 args, but got 1 args
```
---

## 2 STValue_check

### 2.1 isString

`isString(): boolean`

用于检查STValue对象是否包装的是ArkTS-Sta字符串，不接受任何参数，返回布尔值结果。

**参数：** 无

**返回值：**

|  类型   |                  说明                   |
| :-----: | :-------------------------------------: |
| boolean | 如果是字符串类型返回true，否则返回false |

**示例：**

```typescript
// ArkTS-Dyn
let str = STValue.wrapString("Hello");
let isStr = str.isString(); // true

let num = STValue.wrapInt(42);
let isStr1 = num.isString(); // false 

let ns = STValue.findNamespace('stvalue_check.Check');
let str1 = ns.namespaceGetVariable('shouldBeString', SType.REFERENCE);
let isStr2 = str1.isString(); // true
```

```typescript
// stvalue_check.ets ArkTS-Sta
export namespace Check {
   export let shouldBeString: string = "I am a string";
}
```

**报错异常：**

当调用时传入的参数数量不为0时，产生编译错误`参数数量错误`；遇到其它错误时，抛出其它类型的运行错误异常。

示例：
```typescript
try {
    // Invalid Arguments Number
let isStr = str.isString(1); 
} catch (e: Error) {
    // Throw Error
    console.log(e.message);
}
// ArkTS Compiler Error
// Expected 0 Arguments, bug got 1.
```
---

### 2.2 isBigInt

`isBigInt(): boolean`

用于检查STValue对象是否包装的是ArkTS-Sta BigInt对象，不接受任何参数，返回布尔值结果。

**参数：** 无

**返回值：**

|  类型   |                  说明                   |
| :-----: | :-------------------------------------: |
| boolean | 如果是大整数类型返回true，否则返回false |

**示例：**

```typescript
// ArkTS-Dyn
let bigNum = STValue.wrapBigInt(1234567890n);
let isBigInt = bigNum.isBigInt(); // true
let num = STValue.wrapInt(42);
let isBigInt1 = num.isBigInt(); // false
```

**报错异常：**

当调用时传入的参数数量不为0时，产生编译错误`参数数量错误`；遇到其它错误时，抛出其它类型的运行错误异常。

示例：
```typescript   
try {
    // Invalid Arguments Number
    let isBigInt = 'STValue'.isBigInt(); 
} catch (e: Error) {
    // Throw Error
    console.log(e.message);
}
// ArkTS Compiler Error
// Expected 0 Arguments, bug got 1.
```
---

### 2.3 isNull

`isNull(): boolean`

用于检查STValue对象是否包装的是ArkTS-Sta的`null`，不接受任何参数，返回布尔值结果。

**参数：** 无

**返回值：**

|  类型   |                说明                 |
| :-----: | :---------------------------------: |
| boolean | 如果是null值返回true，否则返回false |

**示例：**

```typescript
// ArkTS-Dyn
let nullValue = STValue.getNull();
let isNull = nullValue.isNull(); // true
let intValue = STValue.wrapNumber(42);
let isNull1 = intValue.isNull(); // false
```

**报错异常：**

当调用时传入的参数数量不为0时，产生编译错误`参数数量错误`；遇到其它错误时，抛出其它类型的运行错误异常。

示例：
```typescript
try {
    // Invalid Arguments Number
    let isNull = 'STValue'.isNull();
} catch (e: Error) {
    // Throw Error
    console.log(e.message);
}
// ArkTS Compiler Error
// Expected 0 Arguments, bug got 1.
```
---

### 2.4 isUndefined

`isUndefined(): boolean`

用于检查STValue对象是否包装的是ArkTS-Sta的`undefined`，不接受任何参数，返回布尔值结果。

**参数：** 无

**返回值：** 

|  类型   |                   说明                   |
| :-----: | :--------------------------------------: |
| boolean | 如果是undefined值返回true，否则返回false |

**示例：** 

```typescript
// ArkTS-Dyn
let undefValue = STValue.getUndefined();
let isUndef = undefValue.isUndefined(); // true
let intValue = STValue.wrapNumber(42);
let isUndef1 = intValue.isUndefined(); // false
```

**报错异常：**

当调用时传入的参数数量不为0时，产生编译错误`参数数量错误`；遇到其它错误时，抛出其它类型的运行错误异常。

示例：
```typescript
try {
    // Invalid Arguments Number
    let isUndef = 'STValue'.isUndefined(); 
} catch (e: Error) {
    // Throw Error
    console.log(e.message);
}
// ArkTS Compiler Error
// Expected 0 Arguments, bug got 1.
```
---


### 2.5 isEqualTo

`isEqualTo(other: STValue): boolean`

用于比较`this`和`other`包装的ArkTS-Sta对象引用的具体值是否相等，返回布尔值结果。`this`和`other`包装需要包装ArkTS-Sta对象引用。如果参数错误或类型不匹配，会抛出异常。

**参数：** 

| 参数名 |  类型   | 必填 |           说明            |
| :----: | :-----: | :--: | :-----------------------: |
| other  | STValue |  是  | 要比较的另一个STValue对象 |

**返回值：** 

|  类型   |                      说明                       |
| :-----: | :---------------------------------------------: |
| boolean | 如果两个引用相等true，否则返回false |

**示例：** 

```typescript
// ArkTS-Dyn
let ns = STValue.findNamespace('stvalue_check.Check');
let leftRef = ns.namespaceGetVariable('leftRef', SType.REFERENCE);
let rightRef = ns.namespaceGetVariable('rightRef', SType.REFERENCE);
let rightRefNotEqual = ns.namespaceGetVariable('rightRefNotEqual', SType.REFERENCE);
let isEqual = leftRef.isEqualTo(rightRef); // true
let isEqual1 = leftRef.isEqualTo(rightRefNotEqual); // false
```

```typescript
// stvalue_check.ets ArkTS-Sta
export namespace Check {
    export let leftRef: string = 'isEqualTo';
    export let rightRef: string = 'isEqualTo';
    export let rightRefNotEqual: string = 'isEqualToNotEqual';
}
```

**报错异常：**

当调用时传入的参数数量不为1时，产生编译错误`参数数量错误`；当`this`或者`other`不是引用类型时，抛出运行错误`无效实例对象`；遇到其它错误时，抛出其它类型的运行错误异常。

示例：
```typescript
try{
    // Invalid Instance
    STValue.wrapInt(1).isEqualTo(STValue.wrapInt(1));
} catch (e: Error){
    // Throw Error
    console.log(e.message);
}
// 'this' STValue instance does not wrap a value of type reference
```
---

### 2.6 isStrictlyEqualTo

`isStrictlyEqualTo(other: STValue): boolean`

用于比较`this`和`other`包装的ArkTS-Sta对象引用的指针内存地址是否严格相等，返回布尔值结果。`this`和`other`包装需要包装ArkTS-Sta对象引用。

**参数：** 

| 参数名 |  类型   | 必填 |           说明            |
| :----: | :-----: | :--: | :-----------------------: |
| other  | STValue |  是  | 要比较的另一个STValue对象 |

**返回值：** 

|  类型   |                    说明                     |
| :-----: | :-----------------------------------------: |
| boolean | 如果两个对象严格相等返回true，否则返回false |

**示例：** 

```typescript
// ArkTS-Dyn
let ns = STValue.findNamespace('stvalue_check.Check');
let magicNull = STValue.getNull();
let magicUndefined = STValue.getUndefined();
let result1 = magicNull.isStrictlyEqualTo(magicUndefined); // false

let magicString1 = ns.namespaceGetVariable('magicString1', SType.REFERENCE);
let magicString2 = ns.namespaceGetVariable('magicString2', SType.REFERENCE);
let result2 = magicString1.isStrictlyEqualTo(magicString2); // false
let result3 = magicString1.isStrictlyEqualTo(magicString1); // true
```

```typescript
// stvalue_check.ets ArkTS-Sta
export namespace Check {
    export let magicString1: string = 'Hello World';
    export let magicString2: string = 'Hello World!';
}
```

**报错异常：**

当调用时传入的参数数量不为1时，产生编译错误`参数数量错误`；当`this`或者`other`不是引用类型时，抛出运行错误`无效实例对象`；遇到其它错误时，抛出其它类型的运行错误异常。

示例：
```typescript
try{
    // Invalid Instance
    magicString1.isEqualTo(STValue.wrapInt(1));
} catch (e: Error){
    // Throw Error
    console.log(e.message);
}
// 'other' STValue instance does not wrap a value of type reference
```
---

### 2.7 isBoolean

`isBoolean(): boolean`

用于检查STValue对象是否包装的是ArkTS-Sta的boolean值，不接受任何参数，返回布尔值结果。

**参数：**  无

**返回值：** 

|  类型   |                 说明                  |
| :-----: | :-----------------------------------: |
| boolean | 如果是布尔类型返回true，否则返回false |

**示例：** 

```typescript
// ArkTS-Dyn
let boolValue = STValue.wrapBoolean(true);
let isBool = boolValue.isBoolean(); // true
let numValue = STValue.wrapInt(1);
let isBool1 = numValue.isBoolean(); // false
```

**报错异常：**

当调用时传入的参数数量不为0时，产生编译错误`参数数量错误`；当`this`不是有效的STValue对象时，抛出运行错误`无效实例对象`；遇到其它错误时，抛出其它类型的运行错误异常。

示例：
```typescript
try {
    // Invalid Arguments Number
    let isBool = boolValue.isBoolean(1);
} catch (e: Error) {
    // Throw Error
    console.log(e.message);
}
// ArkTS Compiler Error
// Expected 0 Arguments, bug got 1.
```
---

### 2.8 isByte

`isByte(): boolean`

用于检查STValue对象是否包装的是ArkTS-Sta的byte值，不接受任何参数，返回布尔值结果。

**参数：**  无

**返回值：** 

|  类型   |                 说明                  |
| :-----: | :-----------------------------------: |
| boolean | 如果是字节类型返回true，否则返回false |

**示例：** 

```typescript
// ArkTS-Dyn
let byteValue = STValue.wrapByte(127);
let isByte = byteValue.isByte(); // true
let intValue = STValue.wrapInt(42);g
let isByte1 = intValue.isByte(); // false
```
**报错异常：**

当调用时传入的参数数量不为0时，产生编译错误`参数数量错误`；当`this`不是有效的STValue对象时，抛出运行错误`无效实例对象`；遇到其它错误时，抛出其它类型的运行错误异常。

示例：
```typescript
try {
    // Invalid Arguments Number
    let isByte = byteValue.isByte(1);
} catch (e: Error) {
    // Throw Error
    console.log(e.message);
}
// ArkTS Compiler Error
// Expected 0 Arguments, bug got 1.
```
---

### 2.9 isChar

`isChar(): boolean`

用于检查STValue对象是否包装的是ArkTS-Sta的char值，不接受任何参数，返回布尔值结果。

**参数：**  无

**返回值：** 

|  类型   |                 说明                  |
| :-----: | :-----------------------------------: |
| boolean | 如果是字符类型返回true，否则返回false |

**示例：** 

```typescript
// ArkTS-Dyn
let charValue = STValue.wrapChar('A');
let isChar = charValue.isChar(); // true
let strValue = STValue.wrapString("Hello");
let isChar1 = strValue.isChar(); // false
```

**报错异常：**

当调用时传入的参数数量不为0时，产生编译错误`参数数量错误`；当`this`不是有效的STValue对象时，抛出运行错误`无效实例对象`；遇到其它错误时，抛出其它类型的运行错误异常。

示例：
```typescript
try {
    // Invalid Arguments Number
    let isChar = charValue.isChar(1);
} catch (e: Error) {
    // Throw Error
    console.log(e.message);
}
// ArkTS Compiler Error
// Expected 0 Arguments, bug got 1.
```
---

### 2.10 isShort

`isShort(): boolean`

用于检查STValue对象是否包装的是ArkTS-Sta的short值，不接受任何参数，返回布尔值结果。

**参数：**  无

**返回值：** 

|  类型   |                  说明                   |
| :-----: | :-------------------------------------: |
| boolean | 如果是短整型类型返回true，否则返回false |

**示例：** 

```typescript
// ArkTS-Dyn
let shortValue = STValue.wrapShort(32767);
let isShort = shortValue.isShort(); // true
let intValue = STValue.wrapInt(32767);
let isShort1 = intValue.isShort(); // false
```

**报错异常：**

当调用时传入的参数数量不为0时，产生编译错误`参数数量错误`；当`this`不是有效的STValue对象时，抛出运行错误`无效实例对象`；遇到其它错误时，抛出其它类型的运行错误异常。

示例：
```typescript
try {
    // Invalid Arguments Number
    let isShort = shortValue.isShort(1);
} catch (e: Error) {
    // Throw Error
    console.log(e.message);
}
// ArkTS Compiler Error
// Expected 0 Arguments, bug got 1.
```
---

### 2.11 isInt

`isInt(): boolean`

用于检查STValue对象是否包装的是ArkTS-Sta的int值，不接受任何参数，返回布尔值结果。

**参数：**  无

**返回值：** 

|  类型   |                 说明                  |
| :-----: | :-----------------------------------: |
| boolean | 如果是整型类型返回true，否则返回false |

**示例：** 

```typescript
// ArkTS-Dyn
let intValue = STValue.wrapInt(44);
let isInt = intValue.isInt(); // true
let longValue = STValue.wrapLong(1024);
let isInt1 = longValue.isInt(); // false
```

**报错异常：**

当调用时传入的参数数量不为0时，产生编译错误`参数数量错误`；当`this`不是有效的STValue对象时，抛出运行错误`无效实例对象`；遇到其它错误时，抛出其它类型的运行错误异常。

示例：
```typescript
try {
    // Invalid Arguments Number
    let isInt = intValue.isInt(1);
} catch (e: Error) {
    // Throw Error
    console.log(e.message);
}
// ArkTS Compiler Error
// Expected 0 Arguments, bug got 1.
```
---

### 2.12 isLong

`isLong(): boolean`

用于检查STValue对象是否包装的是ArkTS-Sta的long值，不接受任何参数，返回布尔值结果。

**参数：**  无

**返回值：** 

|  类型   |                  说明                   |
| :-----: | :-------------------------------------: |
| boolean | 如果是长整型类型返回true，否则返回false |

**示例：** 

```typescript
// ArkTS-Dyn
let longValue = STValue.wrapLong(1024);
let isLong = longValue.isLong(); // true
let intValue = STValue.wrapInt(44);
let isLong1 = intValue.isLong(); // false
```

**报错异常：**

当调用时传入的参数数量不为0时，产生编译错误`参数数量错误`；当`this`不是有效的STValue对象时，抛出运行错误`无效实例对象`；遇到其它错误时，抛出其它类型的运行错误异常。

示例：
```typescript
try {
    // Invalid Arguments Number
    let isLong = longValue.isLong(1); 
} catch (e: Error) {
    // Throw Error
    console.log(e.message);
}
// ArkTS Compiler Error
// Expected 0 Arguments, bug got 1.
```
---

### 2.13 isFloat

`isFloat(): boolean`

用于检查STValue对象是否包装的是ArkTS-Sta的float值，不接受任何参数，返回布尔值结果。

**参数：**  无

**返回值：** 

|  类型   |                    说明                     |
| :-----: | :-----------------------------------------: |
| boolean | 如果是单精度浮点类型返回true，否则返回false |

**示例：** 

```typescript
// ArkTS-Dyn
let floatValue = STValue.wrapFloat(3.14);
let isFloat = floatValue.isFloat(); // true
let intValue = STValue.wrapInt(42);
let isFloat1 = intValue.isFloat(); // false
```

**报错异常：**

当调用时传入的参数数量不为0时，产生编译错误`参数数量错误`；当`this`不是有效的STValue对象时，抛出运行错误`无效实例对象`；遇到其它错误时，抛出其它类型的运行错误异常。

示例：
```typescript
try {
    // Invalid Arguments Number
    let isFloat = floatValue.isFloat(1); 
} catch (e: Error) {
    // Throw Error
    console.log(e.message);
}
// ArkTS Compiler Error
// Expected 0 Arguments, bug got 1.
```
---

### 2.14 isNumber

`isNumber(): boolean`

用于检查STValue对象是否包装的是ArkTS-Sta的number/double值，不接受任何参数，返回布尔值结果。

**参数：**  无

**返回值：** 

|  类型   |                    说明                     |
| :-----: | :-----------------------------------------: |
| boolean | 如果是双精度浮点类型返回true，否则返回false |

**示例：** 

```typescript
// ArkTS-Dyn
let doubleValue = STValue.wrapNumber(3.14);
let isNumber = doubleValue.isNumber(); // true
let intValue = STValue.wrapInt(42);
let isNumber1 = intValue.isNumber(); // false
```

**报错异常：**

当调用时传入的参数数量不为0时，产生编译错误`参数数量错误`；当`this`不是有效的STValue对象时，抛出运行错误`无效实例对象`；遇到其它错误时，抛出其它类型的运行错误异常。

示例：
```typescript
try {
    // Invalid Arguments Number
    let isNumber = doubleValue.isNumber(1);
} catch (e: Error) {
    // Throw Error
    console.log(e.message);
}
// ArkTS Compiler Error
// Expected 0 Arguments, bug got 1.
```
---

### 2.15 typeIsAssignableFrom

`static typeIsAssignableFrom(fromType: STValue, toType: STValue): boolean`

用于检查类型之间的赋值兼容性，接受两个参数：源类型（fromType）和目标类型（toType），返回布尔值结果。fromType和toType一般来源于`findClass`、`objectGetType`以及`classGetSuperClass`的返回值。基于底层类型系统的规则，判断源类型的值是否可以安全赋值给目标类型的变量。

**参数：** 

|  参数名  |  类型   | 必填 |            说明            |
| :------: | :-----: | :--: | :------------------------: |
| fromType | STValue |  是  |   源类型（被赋值的类型）   |
|  toType  | STValue |  是  | 目标类型（赋值的目标类型） |

**返回值：** 

|  类型   |                        说明                         |
| :-----: | :-------------------------------------------------: |
| boolean | 如果fromType可以赋值给toType返回true，否则返回false |

**示例：** 

```typescript
// ArkTS-Dyn
let studentCls = STValue.findClass('stvalue_example.Student');
let subStudentCls = STValue.findClass('stvalue_example.SubStudent');
let isAssignable = STValue.typeIsAssignableFrom(subStudentCls, studentCls); // true
let isAssignable1 = typeIsAssignableFrom(studentCls, subStudentCls); // false

// stvalue_example.ets ArkTS-Sta
export class Student {}
export class SubStudent extends Student {}
```

**报错异常：**

当调用时传入的参数数量不为2时，产生编译错误`参数数量错误`；当源类型或目标类型非引用类型或者为null和undefined，抛出运行错误`无效类型`；遇到其它错误时，抛出其它类型的运行错误异常。

示例：
```typescript
try {
    // Invalid Arguments Number
    let isAssignable = STValue.typeIsAssignableFrom(subStudentCls);
} catch (e: Error) {
    // Throw Error
    console.log(e.message);
}
// ArkTS Compiler Error
// Expected 2 Arguments, bug got 1.
```
---

### 2.16 objectInstanceOf

`objectInstanceOf(typeArg: STValue): boolean`

用于检查对象是否为指定类型的实例，接受一个参数（类型对象），返回布尔值结果。如果对象是指定类型的实例返回true，否则返回false。

**参数：** 

| 参数名  |  类型   | 必填 |           说明           |
| :-----: | :-----: | :--: | :----------------------: |
| typeArg | STValue |  是  | 要检查的类型（类或接口） |

**返回值：** 

|  类型   |                   说明                    |
| :-----: | :---------------------------------------: |
| boolean | 如果是该类型的实例返回true，否则返回false |

**示例：** 

```typescript
// ArkTS-Dyn
let studentCls = STValue.findClass('stvalue_check.Student');
// 调用类构造函数，签名为无传参无返回值
let stuObj = studentCls.classInstantiate(':', []);
let isInstance = stuObj.objectInstanceOf(studentCls); // true
```

```typescript
// stvalue_check.ets ArkTS-Sta
export class Student {
    constructor() {}
}
```

**报错异常：**

当调用时传入的参数数量不为1时，产生编译错误`参数数量错误`；当`this`指向对象不是有效实例对象，抛出运行错误`无效实例对象`；当传入的typeArg参数无效，抛出运行错误`无效的参数类型`；遇到其它错误时，抛出其它类型的运行错误异常。

示例：
```typescript
try {
    // Invalid Arguments Number
    let isInstance = stuObj.objectInstanceOf(); 
} catch (e: Error) {
    // Throw Error
    console.log(e.message);
}
// ArkTS Compiler Error
// Expected 1 Arguments, bug got 0.
```
---

## 3 STValue_instance

### 3.1 classInstantiate

`classInstantiate(ctorSignature: string, args: STValue[]): STValue`

用于通过类的构造函数实例化对象，接受两个参数：构造函数签名和参数数组，返回新创建的对象。动态创建类实例，支持带参数的构造函数调用，如果类不存在、构造函数不匹配或参数错误，会抛出异常。

**参数：** 

|    参数名     |   类型    | 必填 |              说明               |
| :-----------: | :-------: | :--: | :-----------------------------: |
| ctorSignature |  string   |  是  | 构造函数（`参数类型:返回类型`） |
|     args      | STValue[] |  是  |        构造函数参数数组         |

**返回值：** 

|  类型   |       说明       |
| :-----: | :--------------: |
| STValue | 新创建的对象实例 |

**示例：** 

```typescript
// ArkTS-Dyn
let studentCls = STValue.findClass('stvalue_instance.Student');
let clsObj1 = studentCls.classInstantiate(':', []);
let obj1Age = clsObj1.objectGetProperty('age', SType.INT).unwrapToNumber(); // 0
let clsObj2 = studentCls.classInstantiate('i:', [STValue.wrapInt(23)]);
let obj2Age = clsObj2.objectGetProperty('age', SType.INT).unwrapToNumber(); // 23
```

```typescript
// stvalue_instance.ets ArkTS-Sta
export class Student {
    public age: int = 0;
    constructor() {}
    constructor(age: int) { this.age = age; }
}
```

**报错异常：**

当传入的参数数量不为2时，产生编译错误`参数数量错误`；当传入的参数不是有效对应类型时，抛出运行错误`参数类型错误`；当`this`不是有效的类对象时，抛出运行错误`非法类对象`；当未找到指定构造函数（签名不匹配），抛出运行错误`无效构造函数`；当实例化对象失败时（构造函数参数不匹配），抛出运行错误`实例化失败`；遇到其它错误时，抛出其它类型的运行错误异常。

示例：
```typescript
try {
    // Invalid Argument Type
    studentCls.classInstantiate(':', STValue.wrapNumber(1));
} catch (e: Error) {
    // Throw Error
    console.log(e.message);
}
// param are not array type; param=args
```
---

### 3.2 newFixedArrayPrimitive

`static newFixedArrayPrimitive(length: number, elementType: SType): STValue`

用于创建预分配内存固定长度的基本类型数组，只接受两个参数：数组长度和元素类型（SType枚举），返回创建的数组对象，其中对应传入的类型枚举SType需符合数组元素类型。每种基本类型都有固定的默认值（例如SType.BOOLEAN默认false，SType.INT默认0），故不需要传入数组元素初始值，只需要指定数组长度以及元素类型即可。支持所有基本数据类型，如果元素类型不支持或参数错误，会抛出异常。

**参数：** 

|   参数名    |  类型  | 必填 |           说明           |
| :---------: | :----: | :--: | :----------------------: |
|   length    | number |  是  |         数组长度         |
| elementType | SType  |  是  | 数组元素类型（数值形式） |

**返回值：** 

|  类型   |               说明                |
| :-----: | :-------------------------------: |
| STValue | 新创建的固定长度数组的STValue对象 |

**示例：** 

```typescript
// ArkTS-Dyn
let ns = STValue.findNamespace('stvalue_instance.Instance');
// 创建的SType.BOOLEAN类型数组boolArray初始长度为5，初始值为false
let boolArray = STValue.newFixedArrayPrimitive(5, SType.BOOLEAN);
// 调用命名空间函数，其函数签名'A{z}:z'表示入参为FixedArray<boolean>，返回值为boolean
let isArray = ns.namespaceInvokeFunction('checkFixPrimitiveBooleanArray', 'A{z}:z', [boolArray]).unwrapToBoolean(); 
```

```typescript
// stvalue_instance.ts ArkTS-Sta
export namespace Instance {
    export function checkFixPrimitiveBooleanArray(arr: FixedArray<boolean>): boolean {
        if (arr.length != 5) {
            return false;
        }
        for (let i = 0; i < 5; i++) {
            console.log(arr[i])
            if (arr[i] != false) {
                return false;
            }
        }
        return true;
    }
}
```

**报错异常：**

当传入的参数数量不为2时，产生编译错误`参数数量错误`；当传入的参数不是有效对应类型时，抛出运行错误`参数类型错误`；当数组长度非有效长度时，抛出运行错误`无效长度`；遇到其它错误时，抛出其它类型的运行错误异常。

示例：
```typescript
try {
    // Invalid Argument Type
    STValue.newFixedArrayPrimitive(5, SType.REFERENCE);
} catch (e: Error) {
    // Throw Error
    console.log(e.message);
}
// Unsupported SType: SType::REFERENCE
```
---

### 3.3 newFixedArrayReference

`static newFixedArrayReference(length: number, elementType: STValue, initialElement: STValue): STValue`

用于创建预分配内存固定长度的引用类型数组，接受三个参数：数组长度、元素类型和**初始元素** ，返回创建的数组对象。由于引用类型没有统一的默认值，因此创建引用类型数组时，除了长度和元素类型，还需指定**初始元素** ，来将所有数组元素会初始化为该元素的引用。支持类、接口等引用类型元素，数组所有元素初始化为相同的初始值，如果参数错误或类型不匹配，会抛出异常。

**参数：** 

|     参数名     |  类型   | 必填 |       说明       |
| :------------: | :-----: | :--: | :--------------: |
|     length     | number  |  是  |     数组长度     |
|  elementType   | STValue |  是  |   数组元素类型   |
| initialElement | STValue |  是  | 数组元素的初始值 |

**返回值：** 

|  类型   |                 说明                  |
| :-----: | :-----------------------------------: |
| STValue | 新创建的固定长度引用数组的STValue对象 |

**示例：** 



```typescript
// ArkTS-Dyn
let ns = STValue.findNamespace('stvalue_instance.Instance');
let intClass = STValue.findClass('std.core.Int');
let intObj = intClass.classInstantiate('i:', [STValue.wrapInt(5)]);
// 创建的Int引用类型数组refArray初始长度为3，初始值是一个装箱了值为5的Int引用类型
let refArray = STValue.newFixedArrayReference(3, intClass, intObj);
// 调用命名空间函数，其函数签名'A{C{std.core.Object}}:z'表示入参为FixedArray<Object>，返回值为boolean
let res = ns.namespaceInvokeFunction('checkFixReferenceObjectArray', 'A{C{std.core.Object}}:z', [refArray])res.unwrapToBoolean(); // true
```

```typescript
// stvalue_instance.ts ArkTS-Sta
export namespace Instance {
    function checkFixReferenceObjectArray(arr: FixedArray<Object>): boolean {
        if (arr.length != 3) {
            return false;
        }
        for (let i = 0; i < 3; i++) {
            if (!(arr[i] instanceof Int)) {
                return false;
            }
        }
        return true;
    }
}
```

**报错异常：**

当传入的参数数量不为3时，产生编译错误`参数数量错误`；当传入的参数不是有效对应类型时，抛出运行错误`参数类型错误`；当数组长度非有效长度时，抛出运行错误`无效长度`；当初始值与类型不匹配时，抛出运行错误`无效初始值`；遇到其它错误时，抛出其它类型的运行错误异常。

示例：
```typescript
try {
    // Invalid InitialElement
    STValue.newFixedArrayReference(999, intClass, [STValue.getNull()]);
} catch (e: Error) {
    // Throw Error
    console.log(e.message);
}
// initialElement STValue instance does not wrap a value of type reference
```
---          

### 3.4 newArray

`static newArray(len: number, initialElement: STValue): STValue`

STValue静态方法，用于创建一个指定长度的新动态数组，并使用提供的初始元素填充数组的所有位置，其中初始元素STValue需要封装引用类型。

**参数：**

| 参数名 | 类型 | 必填 | 说明 |
| :----: | :--: | :--: | :--: |
| len | number | 是 | 数组的初始长度 |
| initialElement | STValue | 是 | 用于填充数组每一个位置的初始元素（需为引用类型） |

**返回值：**

| 类型 | 说明 |
| :--: | :--: |
| STValue | 封装了新创建数组的 STValue 对象 |

**示例：**

```typescript
let intObj = intClass.classInstantiate('i:', [STValue.wrapInt(0)]);
// 传入对象为Int引用类型
let intArray = STValue.newArray(5, intObj); // [0, 0, 0, 0, 0]
```

**报错异常：**

当传入的参数数量不为2时，产生编译错误`参数数量错误`；当传入的参数不是有效对应类型时，抛出运行错误`参数类型错误`；遇到其它错误时，抛出其它类型的运行错误异常。

示例：
```typescript
try {
    // Invalid InitialElement
    STValue.newArray(999, STValue.wrapInt(123));
} catch (e: Error) {
    // Throw Error
    console.log(e.message);
}
// initialElement STValue instance does not wrap a value of type reference
```
---

                 
## 4 STValue_invoke
### 4.1 namespaceInvokeFunction

`namespaceInvokeFunction(functionName: string, signature: string, args: STValue[]): STValue`

用于在命名空间中调用指定函数，接受三个参数：函数名称、函数签名和参数数组，返回函数调用的结果。主要作用是执行命名空间中的全局函数或静态函数，支持带参数的函数调用，如果函数不存在、签名不匹配或参数错误，会抛出异常。

**参数：** 

|    参数名    |   类型    | 必填 |                 说明                  |
| :----------: | :-------: | :--: | :-----------------------------------: |
| functionName |  string   |  是  |               函数名称                |
|  signature   |  string   |  是  | 函数签名（`参数类型:返回类型`） |
|     args     | STValue[] |  是  |             函数参数数组              |

**返回值：** 

|  类型   |           说明            |
| :-----: | :-----------------------: |
| STValue | 函数调用结果的STValue对象 |

**示例：** 


```typescript
// ArkTS-Dyn
let nsp = STValue.findNamespace('stvalue_invoke.Invoke');
let b1 = STValue.wrapBoolean(false);
let b2 = STValue.wrapBoolean(false);
// 调用命名空间函数，函数签名表示有两个Boolean类型入参，返回值也为Boolean类型
let b = nsp.namespaceInvokeFunction('BooleanInvoke', 'zz:z', [b1, b2]).unwrapToBoolean(); // false
```

```typescript
// stvalue_invoke.ets ArkTS-Sta
export namespace Invoke{
  // boolean type
  export function BooleanInvoke(b1 : Boolean, b2 : Boolean) : Boolean{
      return b1 & b2;
  }    
}
```

**报错异常：**

当传入的参数数量不为3时，产生编译错误`参数数量错误`；当传入的参数不是有效对应类型时，抛出运行错误`参数类型错误`；当`this`指向的命名空间无效时，抛出运行错误`无效的命名空间`；当未找到指定函数时（函数名或签名不匹配），抛出运行错误`未找到对应函数`；当执行函数失败时（参数不匹配，函数无效），抛出运行错误`调用函数失败`；遇到其它错误时，抛出其它类型的运行错误异常。

示例：
```typescript
try {
    // Invalid Parameter Count
    b = nsp.namespaceInvokeFunction('BooleanInvoke', 'zzz:z', [b1, b2]);
} catch (e: Error) {
    // Throw Error
    console.log(e.message);
}
// Failed to find function when namespace invoke function;functionName=BooleanInvoke;signatureName=zzz:z
```
---

### 4.2 functionalObjectInvoke

`functionalObjectInvoke(args: STValue[]): STValue`

用于调用函数式对象（如lambda表达式或函数对象），接受一个参数数组（必须为引用类型如果需要使用primitive需要事先装箱），返回函数调用的结果。主要作用是执行函数式对象，支持带参数的调用，如果参数非引用类型或函数对象无效，会抛出异常。

**参数：** 

| 参数名 |   类型    | 必填 |              说明              |
| :----: | :-------: | :--: | :----------------------------: |
|  args  | STValue[] |  是  | 函数参数数组（必须为引用类型） |

**返回值：** 

|  类型   |                 说明                  |
| :-----: | :-----------------------------------: |
| STValue | 函数调用结果的STValue对象（引用类型） |

**示例：** 

```typescript
// ArkTS-Dyn
let nsp = STValue.findNamespace('stvalue_invoke.Invoke');
let getNumberFn = nsp.namespaceGetVariable('getNumberFn', SType.REFERENCE); // 获取的函数式对象getNumberFn为引用类型
let numRes = getNumberFn.functionalObjectInvoke([]); // 函数调用结果numRes是装箱后的引用类型
let jsNum = numRes.objectInvokeMethod('toInt', ':i', []); // 将结果转换为STValue装箱的int类型
let unwrapJsNum = jsNum.unwrapToNumber(); // 通过拆箱获取STValue内的函数结果: 123

let numberCls = STValue.findClass('std.core.Double');
let numberObj3 = numberCls.classInstantiate('d:', [STValue.wrapNumber(3)]); // 创建Double类型的实例
let numberObj5 = numberCls.classInstantiate('d:', [STValue.wrapNumber(5)]);
let getSumFn = nsp.namespaceGetVariable('getSumFn', SType.REFERENCE);
let sumRes = getSumFn.functionalObjectInvoke([numberObj3, numberObj5]); // 需要传入引用类型对象，这里是Double类型
let sumNum = sumRes.objectInvokeMethod('toDouble', ':d', []); // 将结果转换为STValue装箱的double类型，函数签名表示返回值为double
let unwrapSumNum = sumNum.unwrapToNumber(); // 8
```

```typescript
// stvalue_invoke.ets ArkTS-Sta
export namespace Invoke {
    export let getNumberFn = () => { return 123; }
    export let getSumFn = (n1: number, n2: number) => { return n1 + n2; }
}
```

**报错异常：**

当传入的参数数量不为1时，产生编译错误`参数数量错误`；当传入的参数不是有效对应类型时，抛出运行错误`参数类型错误`；当`this`指向的函数式对象无效时，抛出运行错误`无效函数对象`；当执行函数失败时（参数不匹配，函数无效），抛出运行错误`调用函数失败`；遇到其它错误时，抛出其它类型的运行错误异常。

示例：
```typescript
try {
    // Invalid Function Instance
    STValue.getUndefined().functionalObjectInvoke([]);
} catch (e: Error) {
    // Throw Error
    console.log(e.message);
}
// 'this' STValue instance does not wrap a value of type reference
```
---


### 4.3 objectInvokeMethod

`objectInvokeMethod(name: string, signature: string, args: STValue[]): STValue`

用于动态调用对象的方法，接受三个参数：方法名称、方法签名以及参数数组，返回方法调用的结果。主要作用是执行对象的实例方法，支持带参数的方法调用，如果方法不存在、签名不匹配或参数错误，会抛出异常。


**参数：** 

|  参数名   |      类型      | 必填 |                 说明                  |
| :-------: | :------------: | :--: | :-----------------------------------: |
|   name    |     string     |  是  |               方法名称                |
| signature |     string     |  是  | 方法签名（`参数类型:返回类型`） |
|   args    | STValue[]     |  是  |             方法参数数组              |

**返回值：** 

|  类型   |                     说明                      |
| :-----: | :-------------------------------------------: |
| STValue | 方法调用结果的STValue对象（void方法返回null） |

**示例：** 

```typescript
// ArkTS-Dyn
let studentCls = STValue.findClass('stvalue_invoke.Student');
// 类实例化函数，函数签名表示有两个入参分别为int和String类型，无返回值
let clsObj = studentCls.classInstantiate('iC{std.core.String}:', [STValue.wrapInt(18), STValue.wrapString('stu1')]);
let stuAge = clsObj.objectInvokeMethod('getStudentAge', ':i', []); 
let unwrapStuAge = stuAge.unwrapToNumber(); // 18
// 调用实例方法，函数签名表示无入参，返回值类型为String
let stuName = clsObj.objectInvokeMethod('getStudentName', ':C{std.core.String}', []);
let unwrapstuName.unwrapToString(); // 'stu1'
```

```typescript
// stvalue_invoke.ets ArkTS-Sta
export class Student {
    age:int;
    name: string;
    constructor(age: int, name: string) {
        this.age = age;
        this.name = name;
    }
    getStudentAge(): int {
        return this.age;
    }
    getStudentName(): string {
        return this.name;
    }
}
```

**报错异常：**

当传入的参数数量不为3时，产生编译错误`参数数量错误`；当传入的参数不是有效对应类型时，抛出运行错误`参数类型错误`；当`this`指向的对象无效时，抛出运行错误`无效实例对象`；当未找到指定函数时（函数名或签名不匹配），抛出运行错误`未找到对应函数`；当执行函数失败时（参数不匹配，函数无效），抛出运行错误`调用函数失败`；遇到其它错误时，抛出其它类型的运行错误异常。

示例：
```typescript
try {
    // Invalid Parameter Type
    subClsObj.objectInvokeMethod('setStudentAge', 'i:', [[STValue.getUndefined()]]);
} catch (e: Error) {
    // Throw Error
    console.log(e.message);
}
// param array element are not STValue type; param=args; index=0
```
---

### 4.4 classInvokeStaticMethod

`classInvokeStaticMethod(name: string, signature: string, args: STValue[]): STValue`

用于调用类的静态方法，接受三个参数：方法名称、方法签名和参数数组，返回方法调用的结果。主要作用是执行类的静态方法，支持带参数的方法调用，如果方法不存在、签名不匹配或参数错误，会抛出异常。

**参数：** 

|  参数名   |   类型    | 必填 |                 说明                  |
| :-------: | :-------: | :--: | :-----------------------------------: |
|   name    |  string   |  是  |               方法名称                |
| signature |  string   |  是  | 方法签名（`参数类型:返回类型`） |
|   args    | STValue[] |  是  |             方法参数数组              |

**返回值：** 

|  类型   |                     说明                      |
| :-----: | :-------------------------------------------: |
| STValue | 方法调用结果的STValue对象（void方法返回null） |

**示例：** 

```typescript
// ArkTS-Dyn
let studentCls = STValue.findClass('stvalue_invoke.Student');
let stuId = studentCls.classInvokeStaticMethod('getStudentId', ':i', []);
let unwrapStuId = stuId.unwrapToNumber(); // 999

studentCls.classInvokeStaticMethod('setStudentId', 'i:', [STValue.wrapInt(888)]);
let newStuId = studentCls.classInvokeStaticMethod('getStudentId', ':i', []);
let unwrapNewStuId = stuId.unwrapToNumber(); // 888
```

```typescript
// stvalue_invoke.ets ArkTS-Sta
export class Student {
    private static id: int = 999;
    static getStudentId(): int {
        return Student.id;
    }
    static setStudentId(newId: int): void {
        Student.id = newId;
    }
}
```

**报错异常：**

当传入的参数数量不为3时，产生编译错误`参数数量错误`；当传入的参数不是有效对应类型时，抛出运行错误`参数类型错误`；当`this`指向的类无效时，抛出运行错误`无效类实例对象`；当未找到指定函数时（函数名或签名不匹配），抛出运行错误`未找到对应函数`；当执行函数失败时（参数不匹配，函数非静态函数或函数执行异常），抛出运行错误`调用函数失败`；遇到其它错误时，抛出其它类型的运行错误异常。

示例：
```typescript
try {
    // Invalid Class Instance
    STValue.getUndefined().classInvokeStaticMethod('setUId', 's:', []);
} catch (e: Error) {
    // Throw Error
    console.log(e.message);
}
// 'this' STValue instance does not wrap a value of type reference
```
---

## 5 STValue_unwrap

### 5.1 unwrapToNumber

`unwrapToNumber(): number`

用于将STValue对象解包为数字，不接受任何参数，返回数字值结果。支持基本数值类型(`boolean`, `byte`, `short`, `int`, `long`, `float`, `double`)的解包，如果值类型不支持或对象为引用类型，会抛出异常。

**参数：**  无

**返回值：** 

|  类型  |      说明      |
| :----: | :------------: |
| number | 解包后的数字值 |

**示例：** 

```typescript
// ArkTS-Dyn
let intValue = STValue.wrapInt(42);
let num = intValue.unwrapToNumber(); // 42
```

**报错异常：**

当传入的参数数量不为0时，产生编译错误`参数数量错误`；当`this`指向的STValue对象非基本类型时（引用类型，null或undefined），抛出运行错误`非基本类型`；遇到其它错误时，抛出其它类型的运行错误异常。

示例：
```typescript
try {
    // Invalid Type
    let magicSTValueNull = STValue.getNull();
    let magicNull = magicSTValueNull.unwrapToNumber();
} catch (e: Error) {
    // Throw Error
    console.log(e.message);
}
// 'this' STValue instance does not wrap a value of type primitive
```
---

### 5.2 unwrapToString

`unwrapToString(): string`

用于将STValue对象解包为字符串，不接受任何参数，返回字符串结果。仅支持字符串对象（`std.core.String`）的解包，其它类型会抛出异常。

**参数：**  无

**返回值：** 

|  类型  |       说明       |
| :----: | :--------------: |
| string | 解包后的字符串值 |

**示例：** 

```typescript
// ArkTS-Dyn
let strValue = STValue.wrapString("Hello World");
let str = strValue.unwrapToString(); // "Hello World"
```

**报错异常：**

当传入的参数数量不为0时，产生编译错误`参数数量错误`；当`this`指向的STValue对象非字符串对象时，抛出运行错误`非字符串类型`；遇到其它错误时，抛出其它类型的运行错误异常。

示例：
```typescript
try {
    // Invalid Type
    let magicSTValueNull = STValue.getNull();
    let magicNull = magicSTValueNull.unwrapToString();
} catch (e: Error) {
    // Throw Error
    console.log(e.message);
}
// 'this' STValue instance does not wrap a value of type std.core.String
```
---

### 5.3 unwrapToBoolean

`unwrapToBoolean(): boolean`

用于将STValue对象解包为布尔值，不接受任何参数，返回布尔值结果。支持基本类型的解包，不支持引用类型。如果值类型不支持，会抛出异常。

**参数：**  无

**返回值：** 

|  类型   |      说明      |
| :-----: | :------------: |
| boolean | 解包后的布尔值 |

**示例：** 

```typescript
// ArkTS-Dyn
let boolValue = STValue.wrapBoolean(true);
let bool = boolValue.unwrapToBoolean(); // true
let intValue = STValue.wrapInt(1);
let bool1 = intValue.unwrapToBoolean(); // true
let zeroValue = STValue.wrapInt(0);
let bool2 = zeroValue.unwrapToBoolean(); // false
```

**报错异常：**

当传入的参数数量不为0时，产生编译错误`参数数量错误`；当`this`指向的STValue对象非基本类型时（引用类型，null或undefined），抛出运行错误`非基本类型`；遇到其它错误时，抛出其它类型的运行错误异常。

示例：
```typescript
try {
    // Invalid Type
    let magicSTValueNull = STValue.getNull();
    let magicNull = magicSTValueNull.unwrapToBoolean();
} catch (e: Error) {
    // Throw Error
    console.log(e.message);
}
// 'this' STValue instance does not wrap a value of type primitive
```
---

### 5.4 unwrapToBigInt

`unwrapToBigInt(): bigint`

用于将STValue中的大整数对象解包为BigInt类型，不接受任何参数，返回大整数值结果。支持特定的大整数类（`std.core.BigInt`），其它类型会抛出异常。

**参数：**  无

**返回值：** 

|  类型  |       说明       |
| :----: | :--------------: |
| bigint | 解包后的大整数值 |

**示例：** 

```typescript
// ArkTS-Dyn
let bigIntValue = STValue.wrapBigInt(12345678901234567890n); 
let bigInt = bigIntValue.unwrapToBigInt(); // 12345678901234567890n
```

**报错异常：**

当传入的参数数量不为0时，产生编译错误`参数数量错误`；当`this`指向的STValue对象非大整数类时，抛出运行错误`非大整数类型`；遇到其它错误时，抛出其它类型的运行错误异常。

示例：
```typescript
try {
    // Invalid Type
    let magicSTValueNull = STValue.getNull();
    let magicNull = magicSTValueNull.unwrapToBigInt();
} catch (e: Error) {
    // Throw Error
    console.log(e.message);
}
// Expected BigInt object, but got different type.
```
---


## 6 STValue_wrap

### 6.1 wrapByte

`static wrapByte(value: number): STValue`

用于将数字包装为字节 byte（8 位有符号整数）的 STValue 对象，接受一个数字参数，返回包装后的 STValue 对象。如果值超出字节范围（-128到127），会抛出异常。

**参数：** 

| 参数名 |  类型  | 必填 |      说明      |
| :----: | :----: | :--: | :------------: |
| value  | number |  是  | 要包装的数字值 |

**返回值：** 

|  类型   |            说明             |
| :-----: | :-------------------------: |
| STValue | 包装后的字节类型STValue对象 |

**示例：** 

```typescript
// ArkTS-Dyn
let byteValue = STValue.wrapByte(127);
let isByte = byteValue.isByte(); // true
```

**报错异常：**

当传入的参数数量不为1时，产生编译错误`参数数量错误`；当传入参数类型错误，抛出运行错误`参数类型错误`；当传入参数值超出字节范围，抛出运行错误`参数值超出有效字节类型范围`；遇到其它错误时，抛出其它类型的运行错误异常。

示例：
```typescript
try {
    // Out of Range
    let byteValue = STValue.wrapByte(1000);
} catch (e: Error) {
    // Throw Error
    console.log(e.message);
}
// Value is out of range for byte type.
```
---


### 6.2 wrapChar

`static wrapChar(str: string): STValue`

用于将字符串包装为字符类型（16位Unicode字符）的STValue对象，接受一个字符串参数，返回包装后的STValue对象。是将单个字符的字符串转换为字符类型，如果字符串长度不为1时，会抛出异常。

**参数：** 

| 参数名 |  类型  | 必填 |        说明        |
| :----: | :----: | :--: | :----------------: |
| str    | string |  是  | 要包装的单字节字符 |

**返回值：** 

|  类型   |            说明             |
| :-----: | :-------------------------: |
| STValue | 包装后的字符类型STValue对象 |

**示例：** 

```typescript
// ArkTS-Dyn
let charValue = STValue.wrapChar('A');
let isChar = charValue.isChar(); // true
```

**报错异常：**

当传入的参数数量不为1时，产生编译错误`参数数量错误`；当传入参数类型错误，抛出运行错误`参数类型错误`；当传入字符串长度不为1时，抛出运行错误`传入字符串长度必须为1`；遇到其它错误时，抛出其它类型的运行错误异常。

示例：
```typescript
try {
    // Invalid Input String Length
    let charValue = STValue.wrapChar('123');
} catch (e: Error) {
    // Throw Error
    console.log(e.message);
}
// input string length must be 1.
```
---


### 6.3 wrapShort

`static wrapShort(value: number): STValue`

用于将数字包装为短整型short（16 位有符号整数）的STValue对象，接受一个数字参数，返回包装后的STValue对象。如果值超出短整型范围（-2<sup>15</sup> 到2<sup>15</sup> -1），会抛出异常。

**参数：** 

| 参数名 |  类型  | 必填 |              说明               |
| :----: | :----: | :--: | :-----------------------------: |
| value  | number |  是  | 要包装的数字值（-2<sup>15</sup> 到2<sup>15</sup> -1） |

**返回值：** 

|  类型   |           说明            |
| :-----: | :-----------------------: |
| STValue | 包装后的短整型STValue对象 |

**示例：** 

```typescript
// ArkTS-Dyn
let shortValue = STValue.wrapShort(32767);
let isShort = shortValue.isShort(); // true
```
**报错异常：**

当传入的参数数量不为1时，产生编译错误`参数数量错误`；当传入参数类型错误，抛出运行错误`参数类型错误`；当传入参数值超出短整型范围，抛出运行错误`参数值超出有效短整型范围`；遇到其它错误时，抛出其它类型的运行错误异常。

示例：
```typescript
try {
    // Out of Range
    let shortValue = STValue.wrapShort(100000);
} catch (e: Error) {
    // Throw Error
    console.log(e.message);
}
// Value is out of range for short type.
```
---


### 6.4 wrapInt

`static wrapInt(value: number): STValue`

用于将数字包装为整型（32位有符号整数）的STValue对象，接受一个数字参数，返回包装后的STValue对象。如果值超出整型范围（-2<sup>31</sup> 到2<sup>31</sup> -1），会抛出异常。

**参数：** 

| 参数名 |  类型  | 必填 |              说明               |
| :----: | :----: | :--: | :-----------------------------: |
| value  | number |  是  | 要包装的数字值（-2<sup>31</sup> 到2<sup>31</sup> -1） |

**返回值：** 

|  类型   |          说明           |
| :-----: | :---------------------: |
| STValue | 包装后的整型STValue对象 |

**示例：** 

```typescript
// ArkTS-Dyn
let intValue = STValue.wrapInt(123);
let isInt = intValue.isInt(); // true
```
**报错异常：**

当传入的参数数量不为1时，产生编译错误`参数数量错误`；当传入参数类型错误，抛出运行错误`参数类型错误`；当传入参数值超出整型范围，抛出运行错误`参数值超出有效整型范围`；遇到其它错误时，抛出其它类型的运行错误异常。

示例：
```typescript
try {
    // Out of Range
    let intValue = STValue.wrapInt(2147483648);
} catch (e: Error) {
    // Throw Error
    console.log(e.message);
}
// Value is out of range for int type.
```
---


### 6.5 wrapLong

`static wrapLong(value: number|BigInt): STValue`


用于将数字或大整数包装为长整型（64位有符号整数）的STValue对象，接受一个数字或BigInt参数，返回包装后的STValue对象。如果输入的number类型的值超出了整数的精度范围（-2<sup>53</sup> -1到2<sup>53</sup> -1），或者输入的BigInt值超出长整型范围（-2<sup>63</sup> 到2<sup>63 </sup>-1），会抛出异常。

**参数：** 

| 参数名 |  类型  | 必填 |              说明               |
| :----: | :----: | :--: | :-----------------------------: |
| value  | number\|BigInt |  是  | 要包装的数字值（-2<sup>63</sup> 到2<sup>63</sup> -1） |

**返回值：** 

|  类型   |           说明            |
| :-----: | :-----------------------: |
| STValue | 包装后的长整型STValue对象 |

**示例：** 

```typescript
// ArkTS-Dyn
let longValue = STValue.wrapLong(123);
let isLong = longValue.isLong(); // true

let longValue = STValue.wrapLong(123n);
let isLong = longValue.isLong(); // true
```

**报错异常：**

当传入的参数数量不为1时，产生编译错误`参数数量错误`；当传入参数类型错误，抛出运行错误`参数类型错误`；当传入参数值超出整型或者传入的大整数超出长整型范围，抛出运行错误`参数值超出有效长整型范围`；遇到其它错误时，抛出其它类型的运行错误异常。

示例：
```typescript
try {
    // Out of Range
    const MIN_LONG = -(1n << 63n);
    const OVER_MAX_LONG = MAX_LONG + 1n;
    STValue.wrapLong(OVER_MAX_LONG);
} catch (e: Error) {
    // Throw Error
    console.log(e.message);
}
// Value is out of range for long type.
```
---

### 6.6 wrapFloat

`static wrapFloat(value: number): STValue`

用于将数字包装为单精度浮点型（32位浮点数）的STValue对象。针对从双精度浮点数value到单精度浮点数float的转换，实际效果和c++的`static_cast<float>(value)`相同。接受一个数字参数，返回包装后的STValue对象。若数字参数超出单精度浮点数可表示的范围，会被转换为正无穷（+Infinity）或负无穷（-Infinity）。

**参数：** 

| 参数名 |  类型  | 必填 |      说明      |
| :----: | :----: | :--: | :------------: |
| value  | number |  是  | 要包装的数字值 |

**返回值：** 

|  类型   |              说明               |
| :-----: | :-----------------------------: |
| STValue | 包装后的单精度浮点型STValue对象 |

**示例：** 

```typescript
// ArkTS-Dyn
let floatValue = STValue.wrapFloat(3.14);
let isFloat = floatValue.isFloat(); // true
```
**报错异常：**

当传入的参数数量不为1时，产生编译错误`参数数量错误`；当传入参数类型错误，抛出运行错误`参数类型错误`；遇到其它错误时，抛出其它类型的运行错误异常。

示例：
```typescript
try {
    // Invalid Argument Number
    let floatValue = STValue.wrapFloat();
} catch (e: Error) {
    // Throw Error
    console.log(e.message);
}
// ArkTS Compiler Error
// Expected 1 Arguments, bug got 0.
```
---


### 6.7 wrapNumber

`static wrapNumber(value: number): STValue`

用于将数字包装为双精度浮点型（64位浮点数）的STValue对象，接受一个数字参数，返回包装后的STValue对象。若数字参数超出双精度浮点数可表示的范围，会被转换为正无穷（+Infinity）或负无穷（-Infinity）

**参数：** 

| 参数名 |  类型  | 必填 |      说明      |
| :----: | :----: | :--: | :------------: |
| value  | number |  是  | 要包装的数字值 |

**返回值：** 

|  类型   |              说明               |
| :-----: | :-----------------------------: |
| STValue | 包装后的双精度浮点型STValue对象 |

**示例：** 

```typescript
// ArkTS-Dyn
let doubleValue = STValue.wrapNumber(3.14);
let isDouble = doubleValue.isNumber(); // true
```

**报错异常：**

当传入的参数数量不为1时，产生编译错误`参数数量错误`；当传入参数类型错误，抛出运行错误`参数类型错误`；遇到其它错误时，抛出其它类型的运行错误异常。

示例：
```typescript
try {
    // Invalid Argument Number
    let doubleValue = STValue.wrapNumber();
} catch (e: Error) {
    // Throw Error
    console.log(e.message);
}
// ArkTS Compiler Error
// Expected 1 Arguments, bug got 0.
```
---

### 6.8 wrapBoolean

`static wrapBoolean(value: boolean): STValue`

用于将布尔值包装为布尔类型的STValue对象，接受一个布尔参数，返回包装后的STValue对象，支持true和false两种值。

**参数：** 

| 参数名 |  类型   | 必填 |      说明      |
| :----: | :-----: | :--: | :------------: |
| value  | boolean |  是  | 要包装的数字值 |

**返回值：** 

|  类型   |            说明             |
| :-----: | :-------------------------: |
| STValue | 包装后的布尔类型STValue对象 |

**示例：** 

```typescript
// ArkTS-Dyn
let boolValue = STValue.wrapBoolean(true);
let isBool = boolValue.isBoolean(); // true
```

**报错异常：**

当传入的参数数量不为1时，产生编译错误`参数数量错误`；当传入参数类型错误，抛出运行错误`参数类型错误`；遇到其它错误时，抛出其它类型的运行错误异常。

示例：
```typescript
try {
    // Invalid Argument Number
    let boolValue = STValue.wrapBoolean();
} catch (e: Error) {
    // Throw Error
    console.log(e.message);
}
// ArkTS Compiler Error
// Expected 1 Arguments, bug got 0.
```
---

### 6.9 wrapString

`static wrapString(value: string): STValue`

用于将字符串包装为字符串类型的STValue对象，接受一个字符串参数，返回包装后的STValue对象。

**参数：** 

| 参数名 |  类型  | 必填 |       说明       |
| :----: | :----: | :--: | :--------------: |
| value  | string |  是  | 要包装的字符串值 |

**返回值：** 

|  类型   |             说明              |
| :-----: | :---------------------------: |
| STValue | 包装后的字符串类型STValue对象 |

**示例：** 

```typescript
// ArkTS-Dyn
let strValue = STValue.wrapString("Hello World");
let isStr = strValue.isString(); // true
```

**报错异常：**

当传入的参数数量不为1时，产生编译错误`参数数量错误`；当传入参数类型错误，抛出运行错误`参数类型错误`；遇到其它错误时，抛出其它类型的运行错误异常。

示例：
```typescript
try {
    // Invalid Argument Number
    let strValue = STValue.wrapString();
} catch (e: Error) {
    // Throw Error
    console.log(e.message);
}
// ArkTS Compiler Error
// Expected 1 Arguments, bug got 0.
```
---

### 6.10 wrapBigInt

`static wrapBigInt(value: bigint): STValue`

用于将ArkTS-Dyn BigInt对象包装为ArkTS-Sta BigInt类型的STValue对象，接受一个BigInt参数，返回包装后的STValue对象。


**参数：** 

| 参数名 |  类型  | 必填 |       说明       |
| :----: | :----: | :--: | :--------------: |
| value  | bigint |  是  | 要包装的大整数值 |

**返回值：** 

|  类型   |             说明              |
| :-----: | :---------------------------: |
| STValue | 包装后的BigInt类型STValue对象 |

**示例：** 

```typescript
// ArkTS-Dyn
let stBigInt = STValue.wrapBigInt(12345678901234567890n);
let isBigInt = stBigInt.isBigInt(); // true
```

**报错异常：**

当传入的参数数量不为1时，产生编译错误`参数数量错误`；当传入参数类型错误，抛出运行错误`参数类型错误`；遇到其它错误时，抛出其它类型的运行错误异常。

示例：
```typescript
try {
    // Invalid Argument Number
    let stBigInt = STValue.wrapBigInt();
} catch (e: Error) {
    // Throw Error
    console.log(e.message);
}
// ArkTS Compiler Error
// Expected 1 Arguments, bug got 0.
```
---

### 6.11 getNull

`static getNull(): STValue`

用于获取表示ArkTS-Sta内表示`null`的STValue对象，不接受任何参数，返回一个特殊的STValue对象。该对象在所有调用中返回相同的STValue实例。

**参数：**  无

**返回值：** 

|  类型   |         说明          |
| :-----: | :-------------------: |
| STValue | 表示`null`的STValue对象 |

**示例：** 

```typescript
// ArkTS-Dyn
let stNull = STValue.getNull();
let isNull = stNull.isNull(); // true
let stNull1 = STValue.getNull();
stNull === stNull1; // true
```

**报错异常：**

当传入的参数数量不为0时，产生编译错误`参数数量错误`；遇到其它错误时，抛出其它类型的运行错误异常。

示例：
```typescript
try {
    // Invalid Argument Number
    let stNull = STValue.getNull(1);
} catch (e: Error) {
    // Throw Error
    console.log(e.message);
}
// ArkTS Compiler Error
// Expected 1 Arguments, bug got 0.
```
---

### 6.12 getUndefined

`static getUndefined(): STValue`

用于获取表示ArkTS-Sta内表示`undefined`的STValue对象，不接受任何参数，返回一个特殊的STValue对象。该对象在所有调用中返回相同的STValue实例。

**参数：**  无

**返回值：** 

|  类型   |            说明            |
| :-----: | :------------------------: |
| STValue | 表示`undefined`的STValue对象 |

**示例：** 

```typescript
// ArkTS-Dyn
let undefValue = STValue.getUndefined();
let isUndef = undefValue.isUndefined(); // true
let undefValue1 = STValue.getUndefined();
let tst = (undefValue === undefValue1); // true
```

**报错异常：**

当传入的参数数量不为0时，产生编译错误`参数数量错误`；遇到其它错误时，抛出其它类型的运行错误异常。

示例：
```typescript
try {
    // Invalid Argument Number
    let undefValue = STValue.getUndefined(1);
} catch (e: Error) {
    // Throw Error
    console.log(e.message);
}
// ArkTS Compiler Error
// Expected 1 Arguments, bug got 0.
```
---
 
 