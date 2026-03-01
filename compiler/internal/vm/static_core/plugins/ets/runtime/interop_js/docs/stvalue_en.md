# STValue

## Introduce

STValue is a wrapper class that provides a set of interfaces enabling ArkTS-Dyn to call and manipulate data from statically-typed ArkTS-Sta modules.

The STValue object implemented in the dynamic operation of ArkTS-Dyn will store a reference to a static (ArkTS-Sta) object, through which the value of the corresponding object in ArkTS-Sta can be manipulated. This process is shown in the figure below.

<img src='data:image/png;base64,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'/>

---

`STValue` provides six types of interfaces: `accessor`, `check`, `instance`, `invoke`, `unwrap`, and `wrap`, including:

- `accessor` provides interfaces for accessing properties of ArkTS-Sta objects, array elements, and namespace variables.
- `check` provides interfaces for type checking of both primitive types and reference types.
- `instance` provides interfaces for instance creation, type lookup, and inheritance relationship inspection.
- `invoke` provides interfaces for dynamically invoking methods of ArkTS-Sta objects, static methods of classes, and namespace functions.
- `wrap` provides interfaces for converting ArkTS-Dyn values into ArkTS-Sta values and encapsulating them as STValue objects.
- `unwrap` provides interfaces for extracting STValue objects back into ArkTS-Dyn values.

---

### Type enumeration SType

The STValue partial interface requires the ArkTS-Sta type of the specified operation. For this we provide the type enum `SType`:

| Enumeration Name | Description |
| :-------: | :----------------------------------------------
| BOOLEAN | Boolean type, corresponding to boolean in ArkTS-Sta |
| CHAR | Character type, corresponding to char in ArkTS-Sta |
| BYTE | Byte type, corresponding to byte in ArkTS-Sta |
| SHORT | Short integer type, corresponding to short in ArkTS-Sta |
| INT | integer type, corresponding to int in ArkTS-Sta |
| LONG | Long integer type, corresponding to long in ArkTS-Sta |
| FLOAT | Single-precision floating-point type, corresponding to float in ArkTS-Sta |
| DOUBLE | Double-precision floating-point type, corresponding to double/number in ArkTS-Sta |
| REFERENCE | Reference type, corresponding to the reference in ArkTS-Sta |
| VOID | Untyped, corresponding to void in ArkTS-Sta |
---
### Reference STValue

Currently you can get STValue and SType in the following way:

```typescript
let STValue = globalThis.Panda.STValue;
let SType = globalThis.Panda.SType;
```
---
### Name Modifier (Mangling) Rules

Mangling is a special encoding processing method for function signatures. It distinguishes overloaded functions by encoding parameter types and return types, thereby encoding function names as unique symbols. Its format is `parameter type: return type`.

**Usage Example:**
In the interface provided by STValue, `classInstantiate`, `namespaceInvokeFunction`, `objectInvokeMethod` and `classInvokeStaticMethod` all involve Mangling`s function signature rules.

An example of Mangling's function signature using `namespaceInvokeFunction` as an example is shown below:
```typescript
// ArkTS-Dyn
let nsp = STValue.findNamespace('stvalue_invoke.Invoke');
let b1 = STValue.wrapBoolean(false);
let b2 = STValue.wrapBoolean(false);
let b = nsp.namespaceInvokeFunction('BooleanInvoke', 'zz:z', [b1, b2]); // BooleanInvoke(b1: boolean, b2: boolean): boolean
```
For more detailed information on the name mangling rules and usage, please refer to: [Mangling Rules](https://gitcode.com/openharmony/arkcompiler_runtime_core/blob/OpenHarmony_feature_20250702/static_core/plugins/ets/runtime/ani/docs/mangling.md)。

### Other Notes

**1. How to invoke the JSON.stringify() method (which processes static data) in Ark-Sta via STValue and return the serialized result?**

A simple example is shown below, with the main steps including:
- Obtain the instance variable `student` via the namespace.
- Locate and get the JSON class using `findClass()`.
- Invoke the static method `stringify()` of the JSON class through `classInvokeStaticMethod()` to convert the corresponding instance object `student` in Ark-Sta. The result of `stringify()` is an STValue object wrapping a String type.
- Finally, unwrap the returned wrapped object to a String type.

```typescript
// ArkTS-Dyn
let tns = STValue.findNamespace('stvalue_test.testNameSpace');
let student = tns.namespaceGetVariable('student', SType.REFERENCE);
let JSONCls = STValue.findClass("escompat.JSON");
// Invoke stringify; the function signature indicates the input parameter is of Object type and the return value is of String type
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

**2. Notes on the Use of SType**

The use of SType is involved when using interfaces in STValue. It should be noted that the enumeration values of SType must correspond to the corresponding types. For example, in `enumGetValueByName(memberName: string, valueType: SType)` (obtain an enumeration value by its name), the second input parameter needs to be the same type as the enumeration value. If the enumeration value is of int type, **SType.INT** should be passed in. Meanwhile, if the value corresponding to SType is a reference type (such as String, Array, and class instances), the corresponding type is **SType.REFERENCE**.

---
## 1 STValue_accessor

### 1.1 findClass

`static findClass(desc: string): STValue`

Used to find class definitions based on ArkTS-Sta class names, accept a fully qualified path string of a class as a parameter, and return an STValue object that encapsulates the class.

**Parameters:**

| Parameter Name | Type | Required | Description |
| :----: | :----: |  :--------: | :----------: |
| desc | string | yes | class fully qualified path |

**Return value:**

| Type | Description |
| :-----: | :---------------------: |
| STValue | encapsulates the STValue object of this class |


**Example:** 

```typescript
// ArkTS-Dyn
let studentCls = STValue.findClass('stvalue_accessor.Student');
```

```typescript
// stvalue_accessor.ets ArkTS-Sta
export class Student {}
```
**Error:**
When the number of parameters passed during invocation is not 1, a compilation error `Invalid parameter count` shall be raised; when the passed path does not comply with the class descriptor specification (e.g., invalid class name format, class name containing illegal characters, etc.), a runtime error `Invalid class name` shall be thrown; when the default class loader fails to find the class, a runtime error `Lookup failed` shall be thrown; for any errors other than `Lookup failed`, other types of runtime error exceptions shall be thrown.

Example:
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

Used to find a namespace based on its name, it accepts a string parameter (a fully qualified path to the namespace) and returns an STValue object representing the namespace.

**Parameters:**

| Parameter Name | Type | Required | Description |
| :----: | :----: | :--: | :----------------: |
| desc | string | yes | namespace fully qualified path |

**Return value:**

| Type | Description |
| :-----: | :-------------------------: |
| STValue | STValue object representing this namespace |

**Example:**

```typescript
// ArkTS-Dyn
let ns = STValue.findNamespace('stvalue_accessor.MyNamespace');
```

```typescript
// stvalue_accessor.ets ArkTS-Sta
export namespace MyNamespace {}
```
**Error:**
When the number of parameters passed during invocation is not 1, a compilation error `Invalid parameter count` shall be raised; when the passed namespace path does not comply with the specification (e.g., invalid format, namespace name containing illegal characters, etc.), a runtime error `Invalid namespace name` shall be thrown; when the namespace cannot be found, a runtime error `Lookup failed` shall be thrown; for any errors other than `Lookup failed`, other types of runtime error exceptions shall be thrown.

Example:
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

Used to find enumeration type definitions by name, accepts a string parameter (the fully qualified path of the enumeration), and returns an STValue object representing the enumeration. If the enum does not exist or the argument is wrong, an exception is thrown.

**Parameters:**

| Parameter Name | Type | Required | Description |
| :----: | :----: | :------: | :------------: |
| desc | string | yes | enumerate fully qualified paths |

**Return value:**
| Type | Description |
| :-----: | :---------------------: |
| STValue | STValue object representing the enumeration |

**Example:**

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

**Error:**
When the number of parameters passed during invocation is not 1, a compilation error `Invalid parameter count` shall be raised; when the passed enumeration path does not comply with the specification (e.g., invalid format, enumeration name containing illegal characters, etc.), a runtime error `Invalid enumeration name` shall be thrown; when the enumeration cannot be found, a runtime error `Lookup failed` shall be thrown; for any errors other than `Lookup failed`, other types of runtime error exceptions shall be thrown.

Example:
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

Used to get the parent class definition of a class, does not accept any parameters, and returns an STValue object representing the parent class. If the current class has no parent class (such as Object class), null will be returned; if `this` is not a class, an exception will be thrown.

**Parameters:** None

**Return value:**

| Type | Description |
| :-----: | :-------------------: |
| STValue | STValue object representing the parent class |

**Example:**

```typescript
// ArkTS-Dyn
let subClass = STValue.findClass('stvalue_accessor.Dog');
let parentClass = subClass.classGetSuperClass(); // STValue object representing the parent class 'stvalue_accessor.Animal'
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

**Error:**
When the number of parameters passed during invocation is not 0, a compilation error `Invalid parameter count` shall be raised; when `this` is not a valid object, a runtime error `Illegal instance object` shall be thrown; for any other errors, other types of runtime error exceptions shall be thrown.

Example:
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

Used to get the number of elements of an array of fixed length, does not accept any parameters, and returns a numeric value representing the length of the array. If the calling object is not of fixed length array type, an exception will be thrown.

**Parameters:** None

**Return value:**

| Type | Description |
| :----: | :------: |
| number | array length |

**Example:**

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

**Error:**
When the number of parameters passed during invocation is not 0, a compilation error `Invalid parameter count` shall be raised; when `this` is not a valid fixed array object, a runtime error `Illegal instance object` shall be thrown; for any other errors, other types of runtime error exceptions shall be thrown.

Example:
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

`enumGetIndexByName(name: string): number`

Used to query its index position in an enumeration based on its member name, it accepts a string parameter (member name) and returns a numeric value representing the member`s index. If the member does not exist or the argument is wrong, an exception is thrown.


**Parameters:**

| Parameter Name | Type | Required | Description |
| :----: | :----: | :-----: | :------: |
| name | string | yes | enumeration name |

**Return value:**

| Type | Description |
| :----: | :------: |
| number | enumeration index |

**Example:**

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
**Error:**
If the number of parameters passed during invocation is not 1, a compilation error `Invalid parameter count` is raised; if the passed parameter is not a valid string, a runtime error `Invalid parameter type` is thrown; if `this` is not a valid enumeration object, a runtime error `Illegal instance object` is thrown; if the corresponding enumeration item cannot be found, a runtime error `Enumeration does not exist` is thrown; for any other errors, other types of runtime error exceptions are thrown.

Example:
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

`enumGetValueByName(name: string, valueType: SType): STValue`

Used to get the value of an enumeration member based on its name, it accepts two parameters: the member name and the value type, and returns an STValue object of the corresponding value. Enumeration values of integer and string types are supported, and exceptions are thrown if the member does not exist, the type does not match, or the argument is wrong.

**Parameters:**

| Parameter Name | Type | Required | Description |
| :-------: | :----:| :-------: | :--------: |
| name | string | yes | enumeration name |
| valueType | SType | yes | enumeration value type |

**Return value:**

| Type | Description |
| :-----: | :-----------------: |
| STValue | STValue object of enumeration value |

**Example:**

```typescript
// ArkTS-Dyn
let colorEnum = STValue.findEnum('stvalue_accessor.COLOR');
let redValue = colorEnum.enumGetValueByName('Red', SType.INT);
// The enumeration value obtained is an STValue object, which needs to be unboxed to obtain the corresponding primitive value
let unwrapRedValue = redValue.unwrapToNumber(); // 1
```

```typescript
// stvalue_accessor.ets ArkTS-Sta
export enum COLOR{
    Red = 1,
    Green = 3,
}
```

**Error:**
If the number of parameters passed during invocation is not 2, a compilation error `Invalid parameter count` is raised; if the passed parameters are not of the valid corresponding types, a runtime error `Invalid parameter type` is thrown; if `this` is not a valid enumeration object, a runtime error `Illegal instance object` is thrown; if the corresponding enumeration item cannot be found, a runtime error `Enumeration does not exist` is thrown; for any other errors, other types of runtime error exceptions are thrown.

Example:
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

`classGetStaticField(name: string, fieldType: SType): STValue`

Used to get the static field value of a class, it accepts two parameters: field name and field type, and returns the STValue object of the corresponding value. An exception is thrown if the field does not exist, the type does not match, or the argument is wrong.

**Parameters:**

| Parameter Name | Type | Required | Description |
| :-------: | :----:  | :-----: | :------: |
| fieldName | string | yes | field name |
| fieldType | SType | yes | field Type |

**Return value:**

| Type | Description |
| :-----: | :-----------------: |
| STValue | STValue object of field value |

**Example:**

```typescript
// ArkTS-Dyn
let personClass = STValue.findClass('stvalue_accessor.Person');
let name = personClass.classGetStaticField('name', SType.REFERENCE).unwrapToString(); // 'Person'
let age = personClass.classGetStaticField('age', SType.INT).unwrapToNumber(); // 18
```

```typescript
// stvalue_accessor.ets ArkTS-Sta
export class Person {
    static name: string = 'Person';
    static age: number = 18;
}
```
**Error:**
If the number of parameters passed during invocation is not 2, a compilation error `Invalid parameter count` is raised; if the passed parameters are not of the valid corresponding types, a runtime error `Invalid parameter type` is thrown; if `this` is not a valid class object, a runtime error `Illegal instance object` is thrown; if the corresponding field cannot be found, a runtime error `Field does not exist` is thrown; for any other errors, other types of runtime error exceptions are thrown.

Example:
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

`classSetStaticField(name: string, val: STValue, fieldType: SType): void`

A static field value used to set a class that accepts three parameters: the field name, the value to be set, and the field type). The main function is to modify the static member variables of the class to support basic types and reference types. An exception is thrown if the type does not match, the field does not exist, or the argument is wrong.

**Parameters:**

| Parameter Name | Type | Required | Description |
| :-------: | :-----: | :-------: | :--------: |
| fieldName | string | yes | field name |
| val | STValue | yes | value to set |
| fieldType | SType | yes | field type |

**Return value:** None

**Example:**

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

**Error:**
If the number of parameters passed during invocation is not 3, a compilation error `Invalid parameter count` is raised; if the passed parameters are not of the valid corresponding types, a runtime error `Invalid parameter type` is thrown; if `this` is not a valid class object, a runtime error `Illegal instance object` is thrown; if the type of the val parameter does not match the type of the fieldType field, a runtime error `Mismatched field type` is thrown; if the corresponding field cannot be found, a runtime error `Field does not exist` is thrown; for any other errors, other types of runtime error exceptions are thrown.

Example:
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

`objectGetProperty(name: string, propType: SType): STValue`

Used to get the property value of an object instance, it accepts two parameters: property name and property type, and returns an STValue object with the corresponding value. The main function is to access the member variables of the object, supporting basic types and reference types. An exception is thrown if the property does not exist, the type does not match, or the argument is wrong.


**Parameters:**

| Parameter Name | Type | Required | Description |
| :------: |:---: | :-----: | :------: |
| propName | string | yes | property name |
| propType | SType | yes | property type |

**Return value:**

| Type | Description |
| :-----: | :-----------------: |
| STValue | STValue object of property value |

**Example:**

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

**Error:**
If the number of parameters passed during invocation is not 2, a compilation error `Invalid parameter count` is raised; if the passed parameters are not of the valid corresponding types, a runtime error `Invalid parameter type` is thrown; if `this` is not a valid instance object, a runtime error `Illegal instance object` is thrown; if the corresponding property cannot be found, a runtime error `Property does not exist` is thrown; if the property types do not match, a runtime error `Mismatched property type` is thrown; for any other errors, other types of runtime error exceptions are thrown.
Example:
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

`objectSetProperty(name: string, val: STValue, propType: SType): void`

Used to set the property value of an object instance, it accepts three parameters: the property name, the value to be set, and the property type. The main function is to modify the member variables of the object, supporting basic types and reference types. An exception is thrown if the type does not match, the property does not exist, or the argument is wrong.

**Parameters:**

| Parameter Name | Type | Required | Description |
| :------: | :-----:| :-------: | :--------: |
| propName | string | yes | property name |
| val | STValue | yes | value to set |
| propType | SType | yes | property type |

**Return value:** None

**Example:**

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

**Error:**
If the number of parameters passed during invocation is not 3, a compilation error `Invalid parameter count` is raised; if the passed parameters are not of the valid corresponding types, a runtime error `Invalid parameter type` is thrown; if `this` is not a valid instance object, a runtime error `Illegal instance object` is thrown; if the corresponding property cannot be found, a runtime error `Property does not exist` is thrown; if the property types do not match, a runtime error `Mismatched property type` is thrown; for any other errors, other types of runtime error exceptions are thrown.

Example:
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

It is used to get the element value of a specified index in a fixed-length array, accepts two parameters: index position and element type, and returns the STValue object of the corresponding value. The main function is to access array elements, supporting basic types and reference types. An exception is thrown if the index is out of bounds, the type does not match, or the argument is wrong.

**Parameters:**

| Parameter Name | Type | Required | Description |
| :---------: |  :--: | :--------: | :----------: |
| idx | number | yes | array index |
| elementType | SType | yes | array element type |

**Return value:**

| Type | Description |
| :-----: | :---------------: |
| STValue | STValue object of an element |

**Example:**

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
**Error:**
If the number of parameters passed during invocation is not 2, a compilation error `Invalid parameter count` is raised; if the passed parameters are not of the valid corresponding types, a runtime error `Invalid parameter type` is thrown; if `this` is not a valid array object, a runtime error `Illegal instance object` is thrown; if the index idx is out of the array range, a runtime error `Index out of bounds` is thrown; if the array element types do not match, a runtime error `Mismatched element type` is thrown; for any other errors, other types of runtime error exceptions are thrown.

Example:
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

Used to set the value of an element at a specified index in a fixed-length array, it accepts three parameters: the index position, the value to be set, and the element type. The main function is to modify array elements and support basic types and reference types. An exception is thrown if the type does not match, the index is out of bounds, or the argument is wrong.

**Parameters:**

| Parameter Name | Type | Required | Description |
| :---------: | :-----: | :--------: | :----------: |
| idx | number | yes | array index |
| val | STValue | yes | value to set |
| elementType | SType | yes | array element type |

**Return value:** None

**Example:**

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

**Error:**
If the number of parameters passed during invocation is not 3, a compilation error `Invalid parameter count` is raised; if the passed parameters are not of the valid corresponding types, a runtime error `Invalid parameter type` is thrown; if `this` is not a valid array object, a runtime error `Illegal instance object` is thrown; if the index idx is out of the array range, a runtime error `Index out of bounds` is thrown; if the array element types or values do not match, a runtime error `Mismatched element type` is thrown; for any other errors, other types of runtime error exceptions are thrown.

Example:
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

`namespaceGetVariable(name: string, variableType: SType): STValue | null`

Used to get the variable value in the namespace, it accepts two parameters: variable name and variable type, and returns the STValue object of the corresponding value. The main function is to access global variables in the namespace, supporting base types and reference types. An exception is thrown if the variable does not exist, the type does not match, or the argument is wrong.


**Parameters:**

| Parameter Name | Type | Required | Description |
| :----------: | :----: | :--: | :-----------: |
| name | string | yes | variable name |
| variableType | SType | yes | variable type enumeration value |

**Return value:**

| Type | Description |
| :-----: | :-----------------: |
| STValue | STValue object of variable value |

**Example:**

```typescript
// ArkTS-Dyn
let ns = STValue.findNamespace('stvalue_accessor.MyNamespace');
let data = ns.namespaceGetVariable('data', SType.INT);
let num = data.unwrapToNumber(); // 42
```

```typescript
// stvalue_accessor.ets ArkTS-Sta
export namespace MyNamespace {
    export let data: int = 42;
}
```

**Error:**
If the number of parameters passed during invocation is not 2, a compilation error `Invalid parameter count` is raised; if the passed parameters are not of the valid corresponding types, a runtime error `Invalid parameter type` is thrown; if `this` is not a valid namespace, a runtime error `Invalid namespace` is thrown; if a variable does not match its variable type, a runtime error `Mismatched variable type` is thrown; if a variable does not exist, a runtime error `Variable does not exist` is thrown; for any other errors, other types of runtime error exceptions are thrown.

Example:
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

`namespaceSetVariable(name: string, value: STValue, variableType: SType): boolean`

Used to set the value of a variable in a namespace, it accepts three parameters: the variable name, the value to be set, and the variable type, and returns a Boolean value indicating whether the operation was successful. The main function is to modify global variables in the namespace to support base types and reference types. An exception is thrown if the variable does not exist, the type does not match, or the argument is wrong.

**Parameters:**

| Parameter Name | Type | Required | Description |
| :----------: | :-----: | :--: | :-----------: |
| name | string | yes | variable name |
| value | STValue | yes | value to set |
| variableType | SType | yes | variable type enumeration value |

**Return value:**

| Type | Description |
| :-----: | :-----------------------------: |
| boolean | Set success to return true, failure to return false |

**Example:**

```typescript
// ArkTS-Dyn
let ns = STValue.findNamespace('stvalue_accessor.MyNamespace');
ns.namespaceSetVariable('data', STValue.wrapInt(0), SType.INT);
let data = ns.namespaceGetVariable('data', SType.INT);
let num = data.unwrapToNumber(); // 0
```

```typescript
// stvalue_accessor.ets ArkTS-Sta
export namespace MyNamespace {
    export let data: int = 42;
}
```

**Error:**
If the number of parameters passed during invocation is not 3, a compilation error `Invalid parameter count` is raised; if the passed parameters are not of the valid corresponding types, a runtime error `Invalid parameter type` is thrown; if `this` is not a valid namespace, a runtime error `Invalid namespace` is thrown; if the set value does not match the variable type, a runtime error `Mismatched variable type` is thrown; if a variable does not exist, a runtime error `Variable does not exist` is thrown; for any other errors, other types of runtime error exceptions are thrown.

Example:
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

Used to get the type information of the reference type object, does not accept any parameters, and returns an STValue object representing the object type. An exception is thrown if `this` is not wrapped with an ArkTS-Sta object reference.

**Parameters:** None

**Return value:**

| Type | Description |
| :-----: | :-------------------: |
| STValue | STValue object of type information |

**Example:**

```typescript
// ArkTS-Dyn
let strWrap = STValue.wrapString('hello world');
let strType = strWrap.objectGetType();
let isString = strWrap.objectInstanceOf(strType); // true
```

**Error:**
If the number of parameters passed during invocation is not 0, a compilation error `Invalid parameter count` is raised; if `this` is not a valid reference type object, a runtime error `Invalid instance object` is thrown; for any other errors, other types of runtime error exceptions are thrown.

Example:
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

Used to retrieve the element at the specified index of a dynamic array, encapsulate it as an STValue object and return it. The object invoking this method must be a valid STValue reference-type array object.

**Parameters:** 

| Parameter | Type   | Required | Description                |
| :-------: | :-----: | :------: | :------------------------- |
|   idx     | number |    Yes   | Index of the array element |

**Return Value:**

| Type     | Description                                          |
| :------: | :--------------------------------------------------- |
| STValue  | STValue object encapsulating the element at the specified position |

**Example:**

```typescript
// Ark-Dyn
let tns = STValue.findNamespace('stvalue_accessor.testNameSpace')
let intArray = tns.namespaceGetVariable('intArray', SType.REFERENCE);
// Returns an STValue object encapsulating an Int type
let arrayValue0 = intArray.arrayGet(0);
let num = arrayValue0.objectInvokeMethod('toInt', ':i', []).unwrapToNumber(); // 1
```

```typescript
// Ark-Sta
export namespace testNameSpace {
    export let intArray: Array<int> = [1, 2, 3, 4, 5];
}
```

**Error:**
A compilation error `Incorrect number of arguments` is thrown if the number of parameters passed during invocation is not 1; a runtime error `Invoking object is not a reference type` is thrown if the invoking object `this` is not a valid dynamic array reference or is null; other types of runtime errors are thrown if other errors occur.

Example:
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

Used to set the value of the element at the specified index of a dynamic array. The object invoking this method must be a valid STValue reference-type array object, and the array value `val` to be set must be an STValue object encapsulating a reference-type object.

**Parameters:** 

| Parameter | Type     | Required | Description                          |
| :-------: | :-------: | :------: | :----------------------------------- |
|   idx     | number   |    Yes   | Index of the array element to set    |
|   val     | STValue  |    Yes   | STValue encapsulated object to store |

**Return Value:** None

**Example:**

```typescript
// Ark-Dyn
let tns = STValue.findNamespace('stvalue_accessor.testNameSpace')
let intArray = tns.namespaceGetVariable('intArray', SType.REFERENCE);
let intClass = STValue.findClass('std.core.Int');
let intObj = intClass.classInstantiate('i:', [STValue.wrapInt(10)]);
// The passed parameter must be a valid Int reference-type object
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

**Error:**

A compilation error `Incorrect number of arguments` is thrown if the number of parameters passed during invocation is not 2; a runtime error `Incorrect parameter type` is thrown if the `val` parameter is not a valid STValue reference; a runtime error `Invoking object is not a reference type` is thrown if the invoking object `this` is not a valid dynamic array reference or is null; other types of runtime errors are thrown if other errors occur.

Example:
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

Adds a new element to the end of a dynamic array. The object invoking this method must be a valid STValue reference-type array object, and the new array value `val` to be added must be an STValue object encapsulating a reference-type object.

**Parameters:** 

| Parameter | Type     | Required | Description                          |
| :-------: | :-------: | :------: | :----------------------------------- |
|   val     | STValue  |    Yes   | STValue encapsulated object to add   |

**Return Value:** None

**Example:**

```typescript
// Ark-Dyn
let tns = STValue.findNamespace('stvalue_accessor.testNameSpace')
let intArray = tns.namespaceGetVariable('intArray', SType.REFERENCE);
let intClass = STValue.findClass('std.core.Int');
let intObj = intClass.classInstantiate('i:', [STValue.wrapInt(10)]);
// The passed parameter must be a valid Int reference-type object
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

**Error:**

A compilation error `Incorrect number of arguments` is thrown if the number of parameters passed during invocation is not 1; a runtime error `Incorrect parameter type` is thrown if the `val` parameter is not a valid STValue reference; a runtime error `Invoking object is not a reference type` is thrown if the invoking object `this` is not a valid dynamic array reference or is null; other types of runtime errors are thrown if other errors occur.

Example:
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

Removes and returns the last element of a dynamic array. The object invoking this method must be a valid STValue reference-type array object.

**Parameters:** None

**Return Value:** 

| Type     | Description                                          |
| :------: | :--------------------------------------------------- |
| STValue  | STValue object of the removed last element |

**Example:**

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

**Error:**

A compilation error `Incorrect number of arguments` is thrown if the number of parameters passed during invocation is not 0; a runtime error `Invoking object is not a reference type` is thrown if the invoking object `this` is not a valid dynamic array reference or is null; other types of runtime errors are thrown if other errors occur.

Example:
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

Used to retrieve the length of a dynamic array. The object invoking this method must be a valid STValue reference-type array object.

**Parameters:** None

**Return Value:** 

| Type    | Description                          |
| :-----: | :----------------------------------- |
| number  | Current number of elements in the dynamic array |

**Example:**

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

**Error:**

A compilation error `Incorrect number of arguments` is thrown if the number of parameters passed during invocation is not 0; a runtime error `Invoking object is not a reference type` is thrown if the invoking object `this` is not a valid dynamic array reference or is null; other types of runtime errors are thrown if other errors occur.

---

Example:
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

Used to check whether the STValue object is wrapped with an ArkTS-Sta string, does not accept any parameters, and returns a Boolean result.

**Parameters:** None

**Return value:**

| Type | Description |
| :-----: | :-------------------------------------
| boolean | returns true if it is a string type, false otherwise |

**Example:**

```typescript
// ArkTS-Dyn
let str = STValue.wrapString('Hello');
let isStr = str.isString(); // true

let num = STValue.wrapInt(42);
let isStr1 = num.isString(); // false

let ns = STValue.findNamespace('stvalue_check.Check');
data = ns.namespaceGetVariable('shouldBeString', SType.REFERENCE);
let isStr2 = str.isString(); // true
```

```typescript
// stvalue_check.ets ArkTS-Sta
export namespace Check {
   export let shouldBeString: string = "I am a string";
}
```

**Error:**
When the number of parameters passed during invocation is not 0, a compilation error `Invalid parameter count` shall be raised; for any other errors, other types of runtime error exceptions shall be thrown.

Example:
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

Used to check whether the STValue object is wrapped is an ArkTS-Sta BigInt object, which does not accept any parameters and returns a Boolean value result.

**Parameters:** None

**Return value:**

| Type | Description |
| :-----: | :-------------------------------------
| boolean | returns true if it is a large integer type, false otherwise |

**Example:**

```typescript
// ArkTS-Dyn
let bigNum = STValue.wrapBigInt(1234567890n);
let isBigInt = bigNum.isBigInt(); // true
let num = STValue.wrapInt(42);
let isBigInt1 = num.isBigInt(); // false
```

**Error:**
When the number of parameters passed during invocation is not 0, a compilation error `Invalid parameter count` shall be raised; for any other errors, other types of runtime error exceptions shall be thrown.

Example:
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

Used to check whether the STValue object is wrapped with `null` of ArkTS-Sta, does not accept any parameters, and returns a Boolean result.

**Parameters:** None

**Return value:**

| Type | Description |
| :-----: | :---------------------------------: |
| boolean | returns true if null, false otherwise |

**Example:**

```typescript
// ArkTS-Dyn
let nullValue = STValue.getNull();
let isNull = nullValue.isNull(); // true
let intValue = STValue.wrapNumber(42);
let isNull1 = intValue.isNull(); // false
```

**Error:**
When the number of parameters passed during invocation is not 0, a compilation error `Invalid parameter count` shall be raised; for any other errors, other types of runtime error exceptions shall be thrown.

Example:
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

Used to check whether the STValue object is wrapped with `undefined` of ArkTS-Sta, does not accept any parameters, and returns a Boolean result.

**Parameters:** None

**Return value:** 

| Type | Description |
| :-----: | :--------------------------------------
| boolean | Returns true if undefined, false otherwise |

**Example:** 

```typescript
// ArkTS-Dyn
let undefValue = STValue.getUndefined();
let isUndef = undefValue.isUndefined(); // true
let intValue = STValue.wrapNumber(42);
let isUndef1 = intValue.isUndefined(); // false
```

**Error:**
When the number of parameters passed during invocation is not 0, a compilation error `Invalid parameter count` shall be raised; for any other errors, other types of runtime error exceptions shall be thrown.


Example:
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

Used to compare whether the ArkTS-Sta object references wrapped with `this` and `other` are equal, returning a Boolean result. The `this` and `other` wrappers need to wrap ArkTS-Sta object references. If the parameters are wrong or the types do not match, an exception will be thrown.

**Parameters:** 

| Parameter Name | Type | Required | Description |
| :----: | :-----: | :-----: | :-----------------------
| other | STValue | yes | another STValue object to compare |

**Return value:** 

| Type | Description |
| :-----: | :---------------------------------------------
| boolean | If two references are equal true, otherwise return false |

**Example:** 

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

**Error:**
If the number of parameters passed during invocation is not 1, a compilation error `Invalid parameter count` is raised; if either `this` or `other` is not a reference type, a runtime error `Invalid instance object` is thrown; for any other errors, other types of runtime error exceptions are thrown.

Example:
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

ArkTS-Sta object references used to compare `this` and `other` wrappers are strictly equal, returning a Boolean result. The `this` and `other` wrappers need to wrap ArkTS-Sta object references.

**Parameters:** 

| Parameter Name | Type | Required | Description |
| :----: | :-----:| :-----: | :-----------------------
| other | STValue | yes | another STValue object to compare |

**Return value:** 

| Type | Description |
| :-----: | :-----------------------------------------
| boolean | Returns true if two objects are strictly equal, false otherwise |

**Example:** 

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
    export let magicString2: string = 'Hello World!' ;
}
```

**Error:**
If the number of parameters passed during invocation is not 1, a compilation error `Invalid parameter count` is raised; if either `this` or `other` is not a reference type, a runtime error `Invalid instance object` is thrown; for any other errors, other types of runtime error exceptions are thrown.

Example:
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

Used to check whether the STValue object is wrapped with the boolean value of ArkTS-Sta, does not accept any parameters, and returns a boolean result.

**Parameters:** None

**Return value:** 

| Type | Description |
| :-----: | :-----------------------------------: |
| boolean | Returns true if boolean, false otherwise |

**Example:** 

```typescript
// ArkTS-Dyn
let boolValue = STValue.wrapBoolean(true);
let isBool = boolValue.isBoolean(); // true
let numValue = STValue.wrapInt(1);
let isBool1 = numValue.isBoolean(); // false
```

**Error:**
If the number of parameters passed during invocation is not 0, a compilation error `Invalid parameter count` is raised; if `this` is not a valid STValue object, a runtime error `Invalid instance object` is thrown; for any other errors, other types of runtime error exceptions are thrown.

Example:
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

Used to check whether the STValue object is wrapped with the byte value of ArkTS-Sta, does not accept any parameters, and returns a Boolean result.

**Parameters:** None

**Return value:** 

| Type | Description |
| :-----: | :-----------------------------------: |
| boolean | Returns true if it is a byte type, false otherwise |

**Example:** 

```typescript
// ArkTS-Dyn
let byteValue = STValue.wrapByte(127);
let isByte = byteValue.isByte(); // true
let intValue = STValue.wrapInt(42);g
let isByte1 = intValue.isByte(); // false
```
**Error:**
If the number of parameters passed during invocation is not 0, a compilation error `Invalid parameter count` is raised; if `this` is not a valid STValue object, a runtime error `Invalid instance object` is thrown; for any other errors, other types of runtime error exceptions are thrown.

Example:
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

Used to check whether the STValue object is wrapped with the char value of ArkTS-Sta, does not accept any parameters, and returns a Boolean result.

**Parameters:** None

**Return value:** 

| Type | Description |
| :-----: | :-----------------------------------: |
| boolean | Returns true if it is a character type, false otherwise |

**Example:** 

```typescript
// ArkTS-Dyn
let charValue = STValue.wrapChar('A');
let isChar = charValue.isChar(); // true
let strValue = STValue.wrapString('Hello');
let isChar1 = strValue.isChar(); // false
```

**Error:**
If the number of parameters passed during invocation is not 0, a compilation error `Invalid parameter count` is raised; if `this` is not a valid STValue object, a runtime error `Invalid instance object` is thrown; for any other errors, other types of runtime error exceptions are thrown.

Example:
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

Used to check whether the STValue object is wrapped with a short value of ArkTS-Sta, does not accept any parameters, and returns a Boolean result.

**Parameters:** None

**Return value:** 

| Type | Description |
| :-----: | :-------------------------------------
| boolean | Returns true if it is a short integer type, false otherwise |

**Example:** 

```typescript
// ArkTS-Dyn
let shortValue = STValue.wrapShort(32767);
let isShort = shortValue.isShort(); // true
let intValue = STValue.wrapInt(32767);
let isShort1 = intValue.isShort(); // false
```

**Error:**
If the number of parameters passed during invocation is not 0, a compilation error `Invalid parameter count` is raised; if `this` is not a valid STValue object, a runtime error `Invalid instance object` is thrown; for any other errors, other types of runtime error exceptions are thrown.

Example:
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

Used to check whether the STValue object is wrapped with an int value of ArkTS-Sta, does not accept any parameters, and returns a Boolean result.

**Parameters:** None

**Return value:** 

| Type | Description |
| :-----: | :-----------------------------------: |
| boolean | Returns true if it is an integer type, false otherwise |

**Example:** 

```typescript
// ArkTS-Dyn
let intValue = STValue.wrapInt(44);
let isInt = intValue.isInt(); // true
let longValue = STValue.wrapLong(1024);
let isInt1 = longValue.isInt(); // false
```

**Error:**
If the number of parameters passed during invocation is not 0, a compilation error `Invalid parameter count` is raised; if `this` is not a valid STValue object, a runtime error `Invalid instance object` is thrown; for any other errors, other types of runtime error exceptions are thrown.

Example:
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

The long value of ArkTS-Sta is used to check whether the STValue object is wrapped. It does not accept any parameters and returns a Boolean result.

**Parameters:** None

**Return value:** 

| Type | Description |
| :-----: | :-------------------------------------
| boolean | Returns true if it is a long integer type, false otherwise |

**Example:** 

```typescript
// ArkTS-Dyn
let longValue = STValue.wrapLong(1024);
let isLong = longValue.isLong(); // true
let intValue = STValue.wrapInt(44);
let isLong1 = intValue.isLong(); // false
```

**Error:**
If the number of parameters passed during invocation is not 0, a compilation error `Invalid parameter count` is raised; if `this` is not a valid STValue object, a runtime error `Invalid instance object` is thrown; for any other errors, other types of runtime error exceptions are thrown.

Example:
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

The STValue object is used to check whether it is wrapped with the float value of ArkTS-Sta. It does not accept any parameters and returns a Boolean result.

**Parameters:** None

**Return value:** 

| Type | Description |
| :-----: | :-----------------------------------------
| boolean | Returns true if it is a single-precision floating-point type, false otherwise |

**Example:** 

```typescript
// ArkTS-Dyn
let floatValue = STValue.wrapFloat(3.14);
let isFloat = floatValue.isFloat(); // true
let intValue = STValue.wrapInt(42);
let isFloat1 = intValue.isFloat(); // false
```

**Error:**
If the number of parameters passed during invocation is not 0, a compilation error `Invalid parameter count` is raised; if `this` is not a valid STValue object, a runtime error `Invalid instance object` is thrown; for any other errors, other types of runtime error exceptions are thrown.

Example:
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

Used to check whether the STValue object is wrapped with the number/double value of ArkTS-Sta, does not accept any parameters, and returns a Boolean result.

**Parameters:** None

**Return value:** 

| Type | Description |
| :-----: | :-----------------------------------------
| boolean | Returns true if double-precision floating-point type, false otherwise |

**Example:** 

```typescript
// ArkTS-Dyn
let doubleValue = STValue.wrapNumber(3.14);
let isNumber = doubleValue.isNumber(); // true
let intValue = STValue.wrapInt(42);
let isNumber1 = intValue.isNumber(); // false
```

**Error:**
If the number of parameters passed during invocation is not 0, a compilation error `Invalid parameter count` is raised; if `this` is not a valid STValue object, a runtime error `Invalid instance object` is thrown; for any other errors, other types of runtime error exceptions are thrown.

Example:
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

Used to check assignment compatibility between types, it accepts two parameters: source type (fromType) and target type (toType), and returns a Boolean result. fromType and toType generally come from the return values of `findClass`, `objectGetType` and `classGetSuperClass`. Based on the rules of the underlying type system, it is determined whether the value of the source type can be safely assigned to a variable of the target type.

**Parameters:** 

| Parameter Name | Type | Required | Description |
| :------: | :-----: | :------: | :------------------------
| fromType | STValue | yes | Source type (the type being assigned) |
| toType | STValue | yes | Target Type (assigned target type) |

**Return value:** 

| Type | Description |
| :-----: | :-------------------------------------------------
| boolean | Returns true if fromType can be assigned to toType, false otherwise |

**Example:** 

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

**Error:**
If the number of parameters passed during invocation is not 2, a compilation error `Invalid parameter count` is raised; if the source type or target type is not a reference type, or is null or undefined, a runtime error `Invalid type` is thrown; for any other errors, other types of runtime error exceptions are thrown.

Example:
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

Used to check whether an object is an instance of a specified type, accepts a parameter (type object), and returns a Boolean result. Returns true if the object is an instance of the specified type, false otherwise.

**Parameters:** 

| Parameter Name | Type | Required | Description |
| :-----: | :-----: | :------: | :----------------------
| typeArg | STValue | yes | Type to check (class or interface) |

**Return value:** 

| Type | Description |
| :-----: | :---------------------------------------
| boolean | Returns true if an instance of this type, false otherwise |

**Example:** 

```typescript
// ArkTS-Dyn
let studentCls = STValue.findClass('stvalue_check.Student');
let stuObj = studentCls.classInstantiate(':', []);
let isInstance = stuObj.objectInstanceOf(studentCls); // true
```

```typescript
// stvalue_check.ets ArkTS-Sta
export class Student {
    constructor() {}
}
```

**Error:**
If the number of parameters passed during invocation is not 1, a compilation error `Invalid parameter count` is raised; if the object pointed to by `this` is not a valid instance object, a runtime error `Invalid instance object` is thrown; if the passed typeArg parameter is invalid, a runtime error `Invalid parameter type` is thrown; for any other errors, other types of runtime error exceptions are thrown.

Example:
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

Used to instantiate an object through the class`s constructor, accepting two parameters: the constructor signature and the parameter array, and returning the newly created object. Dynamically create class instances, support constructor calls with parameters, and throw exceptions if the class does not exist, the constructor does not match, or the parameters are incorrect.

**Parameters:** 

| Parameter Name | Type | Required | Description |
| :----------: | :--------: | :--: | :-----: |
| ctorSignature | string | yes | constructor(`parameter type: return type`) |
| args | STValue[] | yes | constructor parameter array |

**Return value:** 

| Type | Description |
| :-----: | :--------------: |
| STValue | Newly created object instance |

**Example:** 

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

**Error:**
If the number of parameters passed is not 2, a compilation error `Invalid parameter count` is raised; if the passed parameters are not of the valid corresponding types, a runtime error `Invalid parameter type` is thrown; if `this` is not a valid class object, a runtime error `Illegal class object` is thrown; if the specified constructor is not found (signature mismatch), a runtime error `Invalid constructor` is thrown; if object instantiation fails (constructor parameter mismatch), a runtime error `Instantiation failed` is thrown; for any other errors, other types of runtime error exceptions are thrown.

Example:
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

Used to create a base type array of fixed length in preallocated memory, it only accepts two parameters: array length and element type (SType enumeration), and returns the created array object. Each basic type has a fixed default value (for example, SType.BOOLEAN defaults to false and SType.INT defaults to 0), so there is no need to pass in the initial value of the array element. You only need to specify the array length and element type. All basic data types are supported, and an exception is thrown if the element type is not supported or the parameters are incorrect.

**Parameters:** 

| Parameter Name | Type | Required | Description |
| :----------: | :--------: | :--: | :-----: |
| length | number | yes | array length |
| elementType | SType | yes | Array element type (numeric form) |

**Return value:** 

| Type | Description |
| :-----: | :-------------------------------: |
| STValue | STValue object of a newly created fixed-length array |

**Example:** 

```typescript
// ArkTS-Dyn
let ns = STValue.findNamespace('stvalue_instance.Instance');
// The initial length of the created SType.BOOLEAN type array boolArray is 5, and the initial value is false
let boolArray = STValue.newFixedArrayPrimitive(5, SType.BOOLEAN);
let isArray = ns.namespaceInvokeFunction('checkFixPrimitiveBooleanArray', 'A{z}:z', [boolArray]).unwrapToBoolean(); // true
```

```typescript
// stvalue_instance.ts ArkTS-Sta
export namespace Instance {
    export function checkFixPrimitiveBooleanArray(arr: FixedArray<boolean>): boolean {
        if (arr.length ! = 5) {
            return false;
        }
        for (let i = 0; i < 5; i++) {
            console.log(arr[i])
            if (arr[i] ! = false) {
                return false;
            }
        }
        return true;
    }
}
```

**Error:**
If the number of parameters passed is not 2, a compilation error `Invalid parameter count` is raised; if the passed parameters are not of the valid corresponding types, a runtime error `Invalid parameter type` is thrown; if the array length is not a valid length, a runtime error `Invalid length` is thrown; for any other errors, other types of runtime error exceptions are thrown.

Example:
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

Used to create an array of reference types of fixed length in preallocated memory, it accepts three parameters: array length, element type and **initial element**, and returns the created array object. Since there is no unified default value for reference types, when creating an array of reference types, in addition to the length and element type, you also need to specify **initial element** to initialize all array elements to references of that element. Classes, interfaces, etc. are supported to reference type elements. All elements of the array are initialized to the same initial value. If the parameters are wrong or the types do not match, an exception will be thrown.

**Parameters:** 

| Parameter Name | Type | Required | Description |
| :----------: | :--------: | :--: | :-----: |
| length | number | yes | array length |
| elementType | STValue | yes | Array element type |
| initialElement | STValue | yes | the initial value of the array element |

**Return value:** 

| Type | Description |
| :-----: | :-----------------------------------: |
| STValue | STValue object of a newly created fixed-length reference array |

**Example:** 

```typescript
// ArkTS-Dyn
let ns = STValue.findNamespace('stvalue_instance.Instance');
let intClass = STValue.findClass('std.core.Int');
let intObj = intClass.classInstantiate('i:', [STValue.wrapInt(5)]);
// The initial length of the created Int reference type array refArray is 3, and the initial value is a boxed Int reference type with a value of 5
let refArray = STValue.newFixedArrayReference(3, intClass, intObj);
let res = ns.namespaceInvokeFunction('checkFixReferenceObjectArray', 'A{C{std.core.Object}}:z', [refArray])res.unwrapToBoolean(); // true
```

```typescript
// stvalue_instance.ts ArkTS-Sta
export namespace Instance {
    function checkFixReferenceObjectArray(arr: FixedArray<Object>): boolean {
        if (arr.length ! = 3) {
            return false;
        }
        for (let i = 0; i < 3; i++) {
            if (!( arr[i] instanceof Int)) {
                return false;
            }
        }
        return true;
    }
}
```

**Error:**
If the number of parameters passed is not 3, a compilation error `Invalid parameter count` is raised; if the passed parameters are not of the valid corresponding types, a runtime error `Invalid parameter type` is thrown; if the array length is not a valid length, a runtime error `Invalid length` is thrown; if the initial value does not match the type, a runtime error `Invalid initial value` is thrown; for any other errors, other types of runtime error exceptions are thrown.

Example:
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

A static method of STValue, used to create a new dynamic array of the specified length and populate all positions of the array with the provided initial element. The initial element (an STValue object) must encapsulate a reference type.

**Parameters:**

| Parameter      | Type     | Required | Description                                                                 |
| :------------: | :------: | :------: | :-------------------------------------------------------------------------- |
| len            | number   |    Yes   | Initial length of the array                                                 |
| initialElement | STValue  |    Yes   | Initial element used to populate each position of the array (must be a reference type) |

**Return Value:**

| Type    | Description                                                |
| :-----: | :--------------------------------------------------------- |
| STValue | STValue object encapsulating the newly created array       |

**Example:**

```typescript
let intObj = intClass.classInstantiate('i:', [STValue.wrapInt(0)]);
// The passed object is an Int reference type
let intArray = STValue.newArray(5, intObj); // [0, 0, 0, 0, 0]
```

**Error:**

A compilation error `Incorrect number of arguments` is thrown if the number of passed parameters is not 2; a runtime error `Incorrect parameter type` is thrown if the passed parameters are not valid for their corresponding types; other types of runtime errors are thrown if other errors occur.

Example:
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

Used to call a specified function in a namespace, it accepts three parameters: function name, function signature and parameter array, and returns the result of the function call. The main function is to execute global functions or static functions in the namespace, supporting function calls with parameters. If the function does not exist, the signature does not match, or the parameters are wrong, an exception will be thrown.

**Parameters:** 

| Parameter Name | Type | Required | Description |
| :----------: | :--------: | :--: | :-----: |
| functionName | string | yes | function name |
| signature | string | yes | function signature(`parameter type: return type`) |
| args | STValue[] | yes | function parameter array |

**Return value:** 

| Type | Description |
| :-----: | :-----------------------: |
| STValue | STValue object of function call result |

**Example:** 


```typescript
// ArkTS-Dyn
let nsp = STValue.findNamespace('stvalue_invoke.Invoke');
let b1 = STValue.wrapBoolean(false);
let b2 = STValue.wrapBoolean(false);
let b = nsp.namespaceInvokeFunction('BooleanInvoke', 'zz:z', [b1, b2]).unwrapToBoolean(); // false
```

```typescript
// stvalue_invoke.ets ArkTS-Sta
export namespace Invoke{
  // boolean type
  export function BooleanInvoke(b1: Boolean, b2: Boolean): Boolean{
      return b1 & b2;
  }    
}
```

**Error:**
If the number of parameters passed is not 3, a compilation error `Invalid parameter count` is raised; if the passed parameters are not of the valid corresponding types, a runtime error `Invalid parameter type` is thrown; if the namespace pointed to by `this` is invalid, a runtime error `Invalid namespace` is thrown; if the specified function is not found (mismatched function name or signature), a runtime error `Function not found` is thrown; if function execution fails (parameter mismatch or invalid function), a runtime error `Function call failed` is thrown; for any other errors, other types of runtime error exceptions are thrown.

Example:
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

Used to call functional objects (such as lambda expressions or function objects), it accepts an array of parameters (must be reference types and needs to be boxed in advance if you need to use primitive), and returns the result of the function call. The main function is to execute functional objects and support calls with parameters. If the parameters are non-reference types or the function object is invalid, an exception will be thrown.

**Parameters:** 

| Parameter Name | Type | Required | Description |
| :----------: | :--------: | :--: | :-----: |
| args | STValue[] | yes | Function parameter array (must be reference type) |

**Return value:** 

| Type | Description |
| :-----: | :-----------------------------------: |
| STValue | STValue object (reference type) of function call result |

**Example:** 

```typescript
// ArkTS-Dyn
let nsp = STValue.findNamespace('stvalue_invoke.Invoke');
let getNumberFn = nsp.namespaceGetVariable('getNumberFn', SType.REFERENCE); // The obtained functional object getNumberFn is of reference type
let numRes = getNumberFn.functionalObjectInvoke([]); // The function call result numRes is the reference type after boxing
let jsNum = numRes.objectInvokeMethod('toInt', ':i', []); // Convert the result to STValue boxed int type
let unwrapJsNum = jsNum.unwrapToNumber(); // Get the function result in STValue by unboxing: 123

let numberCls = STValue.findClass('std.core.Double');
let numberObj3 = numberCls.classInstantiate('d:', [STValue.wrapNumber(3)]); // Create an instance of Double type
let numberObj5 = numberCls.classInstantiate('d:', [STValue.wrapNumber(5)]);
let getSumFn = nsp.namespaceGetVariable('getSumFn', SType.REFERENCE);
let sumRes = getSumFn.functionalObjectInvoke([numberObj3, numberObj5]); // Need to pass in a reference type object, here is the Double type
let sumNum = sumRes.objectInvokeMethod('toDouble', ':d', []); // Convert the result to the double type boxed by STValue
let unwrapSumNum = sumNum.unwrapToNumber(); // 8
```

```typescript
// stvalue_invoke.ets ArkTS-Sta
export namespace Invoke {
    export let getNumberFn = () => { return 123; }
    export let getSumFn = (n1: number, n2: number) => { return n1 + n2; }
}
```

**Error:**
If the number of parameters passed is not 1, a compilation error `Invalid parameter count` is raised; if the passed parameters are not of the valid corresponding types, a runtime error `Invalid parameter type` is thrown; if the functional object pointed to by `this` is invalid, a runtime error `Invalid function object` is thrown; if function execution fails (parameter mismatch or invalid function), a runtime error `Function call failed` is thrown; for any other errors, other types of runtime error exceptions are thrown.

Example:
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

A method used to dynamically call an object that accepts three parameters: the method name, the method signature, and an array of parameters, and returns the result of the method call. The main function is to execute the instance method of the object, supporting method calls with parameters. If the method does not exist, the signature does not match or the parameters are wrong, an exception will be thrown.


**Parameters:** 

| Parameter Name | Type | Required | Description |
| :----------: | :--------: | :--: | :-----: |
| name | string | yes | method name |
| signature | string | yes | method signature(`parameter type: return type`) |
| args | STValue[] | yes | method parameter array |

**Return value:** 

| Type | Description |
| :-----: | :-------------------------------------------
| STValue | STValue object of method call result (void method returns null) |

**Example:** 

```typescript
// ArkTS-Dyn
let studentCls = STValue.findClass('stvalue_invoke.Student');
let clsObj = studentCls.classInstantiate('iC{std.core.String}:', [STValue.wrapInt(18), STValue.wrapString('stu1')]);
let stuAge = clsObj.objectInvokeMethod('getStudentAge', ':i', []); 
let unwrapStuAge = stuAge.unwrapToNumber(); // 18
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

**Error:**
If the number of parameters passed is not 3, a compilation error `Invalid parameter count` is raised; if the passed parameters are not of the valid corresponding types, a runtime error `Invalid parameter type` is thrown; if the object pointed to by `this` is invalid, a runtime error `Invalid instance object` is thrown; if the specified function is not found (mismatched function name or signature), a runtime error `Function not found` is thrown; if function execution fails (parameter mismatch or invalid function), a runtime error `Function call failed` is thrown; for any other errors, other types of runtime error exceptions are thrown.

Example:
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

A static method used to call a class that accepts three parameters: the method name, the method signature, and an array of parameters, and returns the result of the method call. The main function is to execute static methods of the class, supporting method calls with parameters. If the method does not exist, the signature does not match, or the parameters are wrong, an exception will be thrown.

**Parameters:** 

| Parameter Name | Type | Required | Description |
| :-------: | :-------: | :--: | :-------: |
| name | string | yes | method name |
| signature | string | yes | method signature(`parameter type: return type`) |
| args | STValue[] | yes | method parameter array |

**Return value:** 

| Type | Description |
| :-----: | :----: |
| STValue | STValue object of method call result (void method returns null) |

**Example:** 

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

**Error:**
If the number of parameters passed is not 3, a compilation error `Invalid parameter count` is raised; if the passed parameters are not of the valid corresponding types, a runtime error `Invalid parameter type` is thrown; if the class pointed to by `this` is invalid, a runtime error `Invalid class instance object` is thrown; if the specified function is not found (mismatched function name or signature), a runtime error `Function not found` is thrown; if function execution fails (parameter mismatch, non-static function, or function execution exception), a runtime error `Function call failed` is thrown; for any other errors, other types of runtime error exceptions are thrown.

Example:
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

Used to unpack STValue objects into numbers, not accept any parameters, and return numeric value results. Supports unpacking of basic numeric types (`boolean`, `byte`, `short`, `int`, `long`, `float`, `double`), which will throw an exception if the value type is not supported or the object is a reference type.

**Parameters:** None

**Return value:** 

| Type | Description |
| :----: | :------------: |
| number | Unpacked number value |

**Example:** 

```typescript
// ArkTS-Dyn
let intValue = STValue.wrapInt(42);
let num = intValue.unwrapToNumber(); // 42
```

**Error:**
If the number of parameters passed is not 0, a compilation error `Invalid parameter count` is raised; if the STValue object pointed to by `this` is not a primitive type (reference type, null, or undefined), a runtime error `Non-primitive type` is thrown; for any other errors, other types of runtime error exceptions are thrown.

Example:
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

Used to unpack the STValue object into a string, do not accept any parameters, and return a string result. Only unwrapping of string objects (`std.core.String`) is supported, other types will throw exceptions.

**Parameters:** None

**Return value:** 

| Type | Description |
| :----: | :--------------: |
| string | Unwrapped string value |

**Example:** 

```typescript
// ArkTS-Dyn
let strValue = STValue.wrapString('Hello World');
let str = strValue.unwrapToString(); // 'Hello World'
```

**Error:**
If the number of parameters passed is not 0, a compilation error `Invalid parameter count` is raised; if the STValue object pointed to by `this` is not a string object, a runtime error `Non-string type` is thrown; for any other errors, other types of runtime error exceptions are thrown.

Example:
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

Used to unpack the STValue object into a Boolean value, not accept any parameters, and return a Boolean result. Unpacking of basic types is supported, but reference types are not. If the value type is not supported, an exception will be thrown.

**Parameters:** None

**Return value:** 

| Type | Description |
| :-----: | :------------: |
| boolean | Unwrapped boolean value |

**Example:** 

```typescript
// ArkTS-Dyn
let boolValue = STValue.wrapBoolean(true);
let bool = boolValue.unwrapToBoolean(); // true
let intValue = STValue.wrapInt(1);
let bool1 = intValue.unwrapToBoolean(); // true
let zeroValue = STValue.wrapInt(0);
let bool2 = zeroValue.unwrapToBoolean(); // false
```

**Error:**
If the number of parameters passed is not 0, a compilation error `Invalid parameter count` is raised; if the STValue object pointed to by `this` is not a string object, a runtime error `Non-string type` is thrown; for any other errors, other types of runtime error exceptions are thrown.

Example:
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

Used to unpack large integer objects in STValue into BigInt type, does not accept any parameters, and returns large integer value results. Supports a specific large integer type (`escompat.BigInt`), other types will throw exceptions.

**Parameters:** None

**Return value:** 

| Type | Description |
| :----: | :--------------: |
| bigint | large integer value after unpacking |

**Example:** 

```typescript
// ArkTS-Dyn
let bigIntValue = STValue.wrapBigInt(12345678901234567890n); 
let bigInt = bigIntValue.unwrapToBigInt(); // 12345678901234567890n
```

**Error:**
If the number of parameters passed is not 0, a compilation error `Invalid parameter count` is raised; if the STValue object pointed to by `this` is not a big integer class, a runtime error `Non-big integer type` is thrown; for any other errors, other types of runtime error exceptions are thrown.

Example:
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

An STValue object used to wrap numbers into byte bytes (8-bit signed integers), accepts a numeric argument, and returns the wrapped STValue object. If the value is outside the byte range (-128 to 127), an exception is thrown.

**Parameters:** 

| Parameter Name | Type | Required | Description |
| :----: | :----: | :--: | :-------------: | 
| value | number | yes | number value to wrap |

**Return value:** 

| Type | Description |
| :-----: | :-------------------------: |
| STValue | Wrapped byte type STValue object |

**Example:** 

```typescript
// ArkTS-Dyn
let byteValue = STValue.wrapByte(127);
let isByte = byteValue.isByte(); // true
```

**Error:**
If the number of parameters passed is not 1, a compilation error `Invalid parameter count` is raised; if the passed parameter type is incorrect, a runtime error `Invalid parameter type` is thrown; if the passed parameter value exceeds the byte range, a runtime error `Value is out of range for byte type` is thrown; for any other errors, other types of runtime error exceptions are thrown.

Example:
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

An STValue object used to wrap a string as a character type (16-bit Unicode characters), accepts a string parameter, and returns the wrapped STValue object. is a string that converts a single character to a character type. If the string length is not 1, an exception will be thrown.

**Parameters:** 

| Parameter Name | Type | Required | Description |
| :----: | :----: | :--: | :----------------: |
| str | string | yes | a single-byte character to wrap |

**Return value:** 

| Type | Description |
| :-----: | :-------------------------: |
| STValue | Wrapped character type STValue object |

**Example:** 

```typescript
// ArkTS-Dyn
let charValue = STValue.wrapChar('A');
let isChar = charValue.isChar(); // true
```

**Error:**
If the number of parameters passed is not 1, a compilation error `Invalid parameter count` is raised; if the passed parameter type is incorrect, a runtime error `Invalid parameter type` is thrown; if the length of the passed string is not 1, a runtime error `The length of the input string must be 1` is thrown; for any other errors, other types of runtime error exceptions are thrown.

Example:
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

An STValue object used to wrap numbers into short integers (16-bit signed integers), accepts a numeric argument, and returns the wrapped STValue object. If the value exceeds the short integer range (-2<sup>15</sup> to 2<sup>15</sup>-1), an exception will be thrown.

**Parameters:** 

| Parameter Name | Type | Required | Description |
| :----: | :----: | :--: | :----: |
| value | number | yes | number value to wrap (-2<sup>15</sup> to 2<sup>15</sup>-1) |

**Return value:** 

| Type | Description |
| :-----: | :-----------------------: |
| STValue | Wrapped short STValue object |

**Example:** 

```typescript
// ArkTS-Dyn
let shortValue = STValue.wrapShort(32767);
let isShort = shortValue.isShort(); // true
```
**Error:**
If the number of parameters passed is not 1, a compilation error `Invalid parameter count` is raised; if the passed parameter type is incorrect, a runtime error `Invalid parameter type` is thrown; if the passed parameter value exceeds the range of short integer, a runtime error `Value is out of range for short type` is thrown; for any other errors, other types of runtime error exceptions are thrown.

Example:
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

An STValue object used to wrap numbers into an integer (a 32-bit signed integer), accepts a numeric argument, and returns the wrapped STValue object. If the value exceeds the integer range (-2<sup>31</sup> to 2<sup>31</sup>-1), an exception will be thrown.

**Parameters:** 

| Parameter Name | Type | Required | Description |
| :----: | :----: | :--: | :----: | 
| value | number | yes | number value to wrap (-2<sup>31</sup> to 2<sup>31</sup>-1) |

**Return value:** 

| Type | Description |
| :-----: | :---------------------: |
| STValue | Wrapped integer STValue object |

**Example:** 

```typescript
// ArkTS-Dyn
let intValue = STValue.wrapInt(123);
let isInt = intValue.isInt(); // true
```
**Error:**
If the number of parameters passed is not 1, a compilation error `Invalid parameter count` is raised; if the passed parameter type is incorrect, a runtime error `Invalid parameter type` is thrown; if the passed parameter value exceeds the range of integer, a runtime error `Value is out of range for int type` is thrown; for any other errors, other types of runtime error exceptions are thrown.

Example:
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


An STValue object used to wrap a number or large integer into a long integer (a 64-bit signed integer), accepting a number or BigInt parameter, and returning the wrapped STValue object. If the input number type value exceeds the integer precision range (-2<sup>53</sup>-1 to 2<sup>53</sup>-1), or the input value exceeds the long integer range (-2<sup>63</sup> to 2<sup>63</sup>-1), An exception will be thrown.

**Parameters:** 

| Parameter Name | Type | Required | Description |
| :----: | :----: | :--: | :----: |
| value | number\|BigInt | yes | the number value to wrap (-2<sup>63</sup> to 2<sup>63</sup>-1) |

**Return value:** 

| Type | Description |
| :-----: | :-----------------------: |
| STValue | Wrapped long STValue object |

**Example:** 

```typescript
// ArkTS-Dyn
let longValue = STValue.wrapLong(123);
let isLong = longValue.isLong(); // true

let longValue = STValue.wrapLong(123n);
let isLong = longValue.isLong(); // true
```

**Error:**
If the number of parameters passed is not 1, a compilation error `Invalid parameter count` is raised; if the passed parameter type is incorrect, a runtime error `Invalid parameter type` is thrown; if the passed parameter value exceeds the integer range, or the passed big integer exceeds the long integer range, a runtime error `Value is out of range for long type` is thrown; for any other errors, other types of runtime error exceptions are thrown.

Example:
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

An STValue object used to wrap numbers as single-precision floating-point numbers (32-bit floating-point numbers). For the conversion from double-precision floating-point value to single-precision floating-point float, the actual effect is the same as `static_cast<float>(value)` in C++. Accepts a numeric argument and returns the wrapped STValue object. If a numeric parameter exceeds the representable range of a single-precision floating-point number, it will be converted to positive infinity (+Infinity) or negative infinity (-Infinity).

**Parameters:** 

| Parameter Name | Type | Required | Description |
| :----: | :----: | :--: | :-------------: |
| value | number | yes | number value to wrap |

**Return value:** 

| Type | Description |
| :-----: | :-----------------------------: |
| STValue | Wrapped single precision floating point STValue object |

**Example:** 

```typescript
// ArkTS-Dyn
let floatValue = STValue.wrapFloat(3.14);
let isFloat = floatValue.isFloat(); // true
```
**Error:**
When the number of parameters passed in is not 1, an error `Wrong number of parameters` is reported; when the parameter type passed in is wrong, an error `Wrong parameter type` is reported; when other errors are encountered, other types of errors are reported.

Example:
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

An STValue object used to wrap numbers into double-precision floating-point types (64-bit floating-point numbers), accepts a number parameter, and returns the wrapped STValue object. If a numeric parameter exceeds the representable range of a double-precision floating-point number, it will be converted to positive infinity (+Infinity) or negative infinity (-Infinity).

**Parameters:** 

| Parameter Name | Type | Required | Description |
| :----: | :----: | :--: | :-------------: | 
| value | number | yes | number value to wrap |

**Return value:** 

| Type | Description |
| :-----: | :-----------------------------: |
| STValue | Wrapped double-precision floating-point STValue object |

**Example:** 

```typescript
// ArkTS-Dyn
let doubleValue = STValue.wrapNumber(3.14);
let isDouble = doubleValue.isNumber(); // true
```

**Error:**
When the number of parameters passed in is not 1, an error `Wrong number of parameters` is reported; when the parameter type passed in is wrong, an error `Wrong parameter type` is reported; when other errors are encountered, other types of errors are reported.

Example:
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

It is used to wrap Boolean values into STValue objects of Boolean type, accept a Boolean parameter, and return the wrapped STValue object. It supports both true and false values.

**Parameters:** 

| Parameter Name | Type | Required | Description |
| :----: | :-----: | :--: | :------------: | 
| value | boolean | yes | numeric value to wrap |

**Return value:** 

| Type | Description |
| :-----: | :-------------------------: |
| STValue | Wrapped Boolean STValue object |

**Example:** 

```typescript
// ArkTS-Dyn
let boolValue = STValue.wrapBoolean(true);
let isBool = boolValue.isBoolean(); // true
```

**Error:**
If the number of passed parameters is not 1, raise compilation error `Invalid parameter count`; if the passed parameter type is wrong, throw runtime error `Invalid parameter type`; for other errors, throw other types of runtime exceptions.

Example:
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

Used to wrap a string into an STValue object of type string, accept a string parameter, and return the wrapped STValue object.

**Parameters:** 

| Parameter Name | Type | Required | Description |
| :----: | :----: | :--: | :-----: |
| value | string | yes | string value to wrap |

**Return value:** 

| Type | Description |
| :-----: | :---------------------------: |
| STValue | Wrapped string type STValue object |

**Example:** 

```typescript
// ArkTS-Dyn
let strValue = STValue.wrapString('Hello World');
let isStr = strValue.isString(); // true
```

**Error:**
If the number of passed parameters != 1, raise compilation error `Invalid parameter count`; if passed parameter type is wrong, throw runtime error `Invalid parameter type`; for other errors, throw other runtime exceptions.

Example:
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

Used to wrap the ArkTS-Dyn BigInt object into an STValue object of the ArkTS-Sta BigInt type, accept a BigInt parameter, and return the wrapped STValue object.


**Parameters:** 

| Parameter Name | Type | Required | Description |
| :----: | :----: | :--: | :-----: | 
| value | bigint | yes | large integer value to wrap |

**Return value:** 

| Type | Description |
| :-----: | :---------------------------: |
| STValue | Wrapped BigInt type STValue object |

**Example:** 

```typescript
// ArkTS-Dyn
let stBigInt = STValue.wrapBigInt(12345678901234567890n);
let isBigInt = stBigInt.isBigInt(); // true
```

**Error:**
When the number of parameters passed is not 1, a compilation error `Invalid parameter count` shall be raised; when the passed parameter type is incorrect, a runtime error `Invalid parameter type` shall be thrown; for any other errors, other types of runtime error exceptions shall be thrown.

Example:
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

Used to get the STValue object representing ArkTS-Sta null, does not accept any parameters, and returns a special STValue object. This object returns the same STValue instance in all calls.

**Parameters:** None

**Return value:** 

| Type | Description |
| :-----: | :-------------------: |
| STValue | STValue object representing null |

**Example:** 

```typescript
// ArkTS-Dyn
let stNull = STValue.getNull();
let isNull = stNull.isNull(); // true
let stNull1 = STValue.getNull();
stNull === stNull1; // true
```

**Error:**
When the number of parameters passed is not 0, a compilation error `Invalid parameter count` shall be raised; for any other errors, other types of runtime error exceptions shall be thrown.

Example:
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

Used to get the STValue object representing ArkTS-Sta undefined, does not accept any parameters, and returns a special STValue object. This object returns the same STValue instance in all calls.

**Parameters:** None

**Return value:** 

| Type | Description |
| :-----: | :------------------------: |
| STValue | represents an undefined STValue object |

**Example:** 

```typescript
// ArkTS-Dyn
let undefValue = STValue.getUndefined();
let isUndef = undefValue.isUndefined(); // true
let undefValue1 = STValue.getUndefined();
undefValue === undefValue1; // true
```

**Error:**
When the number of parameters passed is not 0, a compilation error `Invalid parameter count` shall be raised; for any other errors, other types of runtime error exceptions shall be thrown.

Example:
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