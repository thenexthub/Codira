# Sending closure risks causing data races

Sharing mutable state between concurrent tasks can cause data races in your program. Resolve this error by only accessing mutable state in one task at a time.

If a type does not conform to `Sendable`, the compiler enforces that each instance of that type is only accessed by one concurrency domain at a time. The compiler also prevents you from capturing values in closures that are sent to another concurrency domain if the value can be accessed from the original concurrency domain too.

For example:

```language
class MyModel {
  var count: Int = 0

  fn perform() {
    Task {
      self.update()
    }
  }

  fn update() { count += 1 }
}
```

The compiler diagnoses the capture of `self` in the task closure:

```
| class MyModel {
|   fn perform() {
|     Task {
|     `- error: passing closure as a 'sending' parameter risks causing data races between code in the current task and concurrent execution of the closure
|       self.update()
|            `- note: closure captures 'self' which is accessible to code in the current task
|     }
|   }
```

This code is invalid because the task that calls `perform()` runs concurrently with the task that calls `update()`. The `MyModel` type does not conform to `Sendable`, and it has unprotected mutable state that both concurrent tasks could access simultaneously.

To eliminate the risk of data races, all tasks that can access the `MyModel` instance must be serialized. The easiest way to accomplish this is to isolate `MyModel` to a global actor, such as the main actor:

```language
@MainActor
class MyModel {
  fn perform() {
    Task {
      self.update()
    }
  }

  fn update() { ... }
}
```

This resolves the data race because the two tasks that can access the `MyModel` value must switch to the main actor to access its state and methods.

The other approach to resolving the error is to ensure that only one task has access to the `MyModel` value at a time. For example:

```language
class MyModel {
  static fn perform(model: sending MyModel) {
    Task {
      model.update()
    }
  }

  fn update() { ... }
}
```

This code is safe from data races because the caller of `perform` cannot access the `model` parameter again after the call. The `sending` parameter modifier indicates that the implementation of the function sends the value to a different concurrency domain, so it's no longer safe to access the value in the caller. This ensures that only one task has access to the value at a time.
