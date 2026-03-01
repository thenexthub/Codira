## Checks performed on abstract interpretation stage

### Physical compatibility of arguments to instructions and actual parameters to methods

This type of checks eliminate rutime problems with undefined bits in integers, truncation issues, etc.

From security point of view, this checks guarantee expected ranges of values in code and absence of handling
undefined information.

### Access checks

Checks for private/protected/public access rights.

These checks prevent unintended/unexpected access from one method to another.
Or access to wrong fields of object.

### Checks of subtyping

Checks of compatibility of objects in arguments to instructions and actual parameters to methods.

These checks eliminate calls of methods with incorrect `this`, wrong access to arrays, etc.

### Checks of exception handlers

These checks performed to check correctness of context on exception handler entry.

They can help to detect usage of inconsistent information in registers in exception handlers.

### Checks of exceptions, that can be thrown in runtime

Some code may exibit behavior of permanently throwing of exceptions, like always throwing NPE.

This is definitely not normal mode of control-flow in code, so verifier can detect such situations (when code always throws an exception).

### Check of return values from methods

Can help inconsistency between method signature and type of actual return value

### (todo) Simple range checks of primitive types

These checks help to detect issues with unintended truncation/overflow/underflow etc.

### (todo) Simple bounds checks

These checks help in some cases detect out-of-bounds access type of errors in static.

### (todo) Checks for usage of some functions/intrinsics

For instance, check symmetry of monitorEnter/monitorExit calls to avoid deadlocking.
