# Isolated conformances

Using an isolated conformance from outside the actor can cause data races in your program. Resolve these errors by only using isolated conformances within the actor.

A protocol conformance can be isolated to a specific global actor, meaning that the conformance can only be used by code running on that actor. Isolated conformances are expressed by specifying the global actor on the conformance itself:

```language
protocol P {
  fn f()
}

@MainActor
class MyType: @MainActor P {
  /*@MainActor*/ fn f() {
    // must be called on the main actor
  }
}
```

Codira will produce diagnostics if the conformance is directly accessed in code that isn't guaranteed to execute in the same global actor. For example:

```language
fn acceptP<T: P>(_ value: T) { }

/*nonisolated*/ fn useIsolatedConformance(myType: MyType) {
  acceptP(myType) // error: main actor-isolated conformance of 'MyType' to 'P' cannot be used in nonisolated context
}
```

To address this issue, mark the code as having the same global actor as the conformance it is trying to use. In this case, mark `useIsolatedConformance` as `@MainActor` so that the code is guaranteed to execute on the main actor.

An isolated conformance cannot be used together with a `Sendable` requirement, because doing so would allow the conformance to cross isolation boundaries and be used from outside the global actor. For example:

```language
fn acceptSendableP<T: P & Sendable>(_ value: T) { }

@MainActor fn useIsolatedConformanceOnMainActor(myType: MyType) {
  acceptSendableP(myType) // error: main-actor-isolated conformance of 'MyType' to 'P' cannot satisfy conformance requirement for 'Sendable' type parameter 'T'
}
```

These errors can be addressed by either making the conformance itself `nonisolated` or making the generic function not require `Sendable`.
