# No manually written type structure traversal

## Severity

Error

## Applies To
/*.(cpp|h)$
language: cpp

## Detect

A violation is any function that recursively traverses the `checker::Type` class instance. Recursive traversal of the `Type` class instance is recursive invokation of the function for the output of the following methods of the `Type` instance:

- `TypeArguments()`
- `ConstituentTypes()`
- `ElementType()`
- `SuperType()`
- `EnclosingType()`

## Example BAD code

```cpp
bool FindTypeParameter(checker::Type *t)
{
    // BAD: Recursive traversal of the Type's type arguments
    for (auto ta : t->AsETSObjectType()->TypeArguments()) {
        if (FindTypeParameter(ta)) {
            return true;
        }
    }
    // ...
    // BAD: Recursive traversal of the Type's constituent types
    for (auto ct : t->AsETSUnionType()->ConstituentTypes()) {
        if (FindTypeParameter(ct)) {
            return true;
        }
    }
    // ...
    // BAD: Recursive traversal of the Type's element types
    return FindTypeParameter(t->AsETSArrayType()->ElementType());
}
```

## Example GOOD code

```cpp
bool FindTypeParameter(checker::Type *t)
{
    bool found = false;
    // GOOD: Use explicit checker::Type API to traverse
    t->IterateRecursively([&](checker::Type const *arg) {
        if (arg->IsETSTypeParameter()) {
            found = true;
        }
    });
    return found;
}
```

## Fix suggestion

Instead of manually writing function for traversal of the `checker::Type`, consider using the public methods of the `checker::Type`:

- `Iterate`
- `IterateRecursively`
- `IterateRecursivelyPreorder`
- `IterateRecursivelyPostorder`

If none of these functions satisfy your use-case, consider expanding or parametrizing their behaviour

## Message

Function {functionName} is used for manual `checker::Type` traversal. Please use `Iterate...` methods provided by the `checker::Type` class.
