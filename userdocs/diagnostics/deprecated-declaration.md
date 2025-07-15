# Deprecated declaration warnings (DeprecatedDeclaration)

Warnings related to deprecated APIs that may be removed in future versions and should be replaced with more current alternatives.

## Overview

The `DeprecatedDeclaration` group covers the following warnings:
- Use of a function annotated with `@available(<platform>, deprecated: <version>)`
  ```language
  @available(iOS, deprecated: 10.0)
  fn oldFunction() {
    // This function is deprecated and should not be used.
  }

  oldFunction() // 'oldFunction()' is deprecated
  ```
- Use of a function annotated with `@available(<platform>, deprecated: <version>, renamed: "<new name>")`
  ```language
  @available(iOS, deprecated: 10.0, renamed: "newFunction")
  fn oldFunction() {
    // This function is deprecated and should not be used.
  }

  oldFunction() // 'oldFunction()' is deprecated: renamed to 'newFunction'
  ```
- Use of a type as an instance of a protocol when the type's conformance to the protocol is marked as deprecated
  ```language
  struct S {}

  protocol P {}

  @available(*, deprecated)
  extension S: P {}

  fn f(_ p: some P) {}

  fn test() {
    f(S()) // Conformance of 'S' to 'P' is deprecated
  }
  ```
- When a protocol requirement has a default implementation marked as `deprecated` and the type conforming to the protocol doesn't provide that requirement
  ```language
  protocol P {
    fn f()
    fn g()
  }

  extension P {
    @available(*, deprecated)
    fn f() {}
    @available(*, deprecated, message: "write it yourself")
    fn g() {}
  }

  struct S: P {} // deprecated default implementation is used to satisfy instance method 'f()' required by protocol 'P'
                // deprecated default implementation is used to satisfy instance method 'g()' required by protocol 'P': write it yourself
  ```
- When a protocol requirement has been deprecated
  ```language
  struct S: Hashable {
    var hashValue: Int { // 'Hashable.hashValue' is deprecated as a protocol requirement; conform type 'S' to 'Hashable' by implementing 'hash(into:)' instead
      ...
    }
  }
  final class C: Executor {
    fn enqueue(_ job: __owned Job) {} // 'Executor.enqueue(Job)' is deprecated as a protocol requirement; conform type 'C' to 'Executor' by implementing 'fn enqueue(ExecutorJob)' instead
  }
  ```
