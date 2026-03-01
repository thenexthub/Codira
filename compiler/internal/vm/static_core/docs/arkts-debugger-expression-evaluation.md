# Expression evaluation feature in debugger for ArkTS language

To provide expression evaluation functionality for ArkTS language in the debugger, it is necessary to have the ability to compile expressions in the current execution context.
The approach is implemented in the `ScopedDebugInfoPlugin` in `es2panda` and is concluded in compiling expressions with debug information from the given .abc file. 

Note that apart from debugging information the plugin also uses other information from the given .abc files, e.g. information about classes records.

## List of supported features

Notes:
1. Currently, it is not possible to supports imports, so all features work only if the entity is located in the file where evaluation takes place.

2. In expression that will be evaluated, user can access private and protected fields and methods of objects.

3. Access to all standard classes is supported.

List of features:
1. Local variables
2. Global variables
3. Functions
4. Classes

## Evaluating expression principal scheme

![image info](./images/arkts-expression-evaluation-principle-scheme.svg)

## Explanation

The main problem in expression evaluation feature is compilation with debugging information, so let's look at it in more detail.

The first logical step after starting to execute es2panda in evaluation mode is to wrap the given expression in a class with a static function.
The name of class and name of static function are generated from file name and random number.
This step is necessary, because several evaluations are possible in a single file.
It is necessary to distinguish between generated classes when loading them into the Runtime.
If the last line of an expression received from the user potentially returns a primitive value, then a return statement of this value will be generated in the evaluation method during compilation.

**There are two main ideas that help in solving the problem of expression compilation:**

1. It is enough to insert intrinsic calls in order to recover the context of local variables.

For example, if we have declared an integer variable with name `a` in source code and want to evaluate expression `a = a + 1`, then the following code will be generated:
 
```ts
let a = DebuggerAPIGetLocalInt(reg_number); // get variable from register

a = a + 1;

DebuggerAPISetLocalInt(reg_number, a); // update register state
```

`reg_number` and type of intrinsic are obtained from debugging information due to find local variable in `ScopedDebugInfoPlugin::FindLocalVariable`.

Note that plugin inserts intrinsics calls directly into the AST, but not into the source code.

When bytecode with such intrinsic calls is executed, a local variable of the correct type is obtained from a register.
Then, this local variable may be modified by a given expression, so after modifications, the variable is written back to the register.

2. In case of evaluating an expression containing a class, a class type variable, etc., it is enough to deserialize the class declaration from the debugging information into the class definition in the AST with empty method bodies and empty fields.

Consider the following source code as an example:

```ts
class A {
   public static foo(): int
   {
      return 10 * 10;
   }
   public static x: int = 100;
}

let a = new A();
```

with expression to evaluate after declaration of `a` variable `a.foo() == a.x`.

The plugin deserializes a class from debugging information into an AST class consisting of empty methods and fields, which is equivalent to the following code:
```ts
class A {
   public static foo(): int {}
   public static x: int;
}
```

The following bytecode will be generated:
```
.record A <external> {
   i32 x <external>
}

.function i32 A.foo(A a0) <external, access.function=public>
```

In other words, we create an empty entity for class A in the AST to pass all frontend checks in order to make it possible to address variable of type, that don't exists in current context.
After the frontend emits the bytecode, the bytecode will contain record of class A with `external` annotation for the class itself and for all used fields and methods.

## ScopedDebugPlugin architecture

![image info](./images/arkts-debugger-expression-evaluation.svg)

Note that `PathResolver` can't obtain information from the `ImportExportTable`, because this information currently is not recorded in the debug information.
