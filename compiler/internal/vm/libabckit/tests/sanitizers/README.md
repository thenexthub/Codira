# Known and suppressed leaks:

- leak:NewMutatorLock - `static_core/runtime` has known memory leak issues, check `runtime_core/static_core/runtime/locks.cpp:45`
- leak:CreateJSVM - dynamic VM has memory leak issues
- leak:CreateSharedClass - dynamic VM has memory leak issues
