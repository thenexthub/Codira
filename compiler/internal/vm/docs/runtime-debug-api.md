# Runtime debug API

### Requirements

1. Runtime should support debugging on the platforms from low-end IoT devices to hi-end mobile phones.

### Key Design Decisions

1. Runtime doesn't patch apps' bytecode on the fly. Instead of it notification about changing bytecode pc is used.

1. Runtime and debugger work in the same process. Debugger functionality is provided via shared library, that runtime loads when works in debugger mode. Debugger works in the own thread and responsibilies for thread management relies on it, runtime doesn't create/destroy threads.

### Rationale

1. As some low-end targets can store bytecode in ROM, runtime cannot patch app's bytecode on the fly. So it uses slower approach with interpreter instrumentation.

1. To simplify communication beetween debugger and runtime (especially on microcontrollers) they are work in the same process. Debugger is loaded as shared library when it's necessary.

### Specification / Implementation

To start runtime in the debug mode a debugger [`LoadableAgent`](../runtime/include/loadable_agent.h) is loaded, which usually represents a debugger shared library. It is created by the VM (see `PandaVM::LoadDebuggerAgent()`) as it is language-specific.

The loaded agent starts a debug session when necessary (see `Runtime::StartDebugSession()`). When starting the session, the JIT is disabled, and the [`tooling::Debugger`](../runtime/tooling/debugger.h) object that implements [`tooling::DebugInterface`](../runtime/include/tooling/debug_interface.h) - the point of interaction with the runtime - is created. The agent may access this using the `Runtime::DebugSession::GetDebugger()` method.

Runtime provides [`RuntimeNotificationManager`](../runtime/include/runtime_notification.h) class that allows to subscript for different events:
* `LoadModule` - occurs when panda file is loaded by the runtime
* `BytecodePcChanged` - occurs when bytecode pc is changed during interpretation (only if runtime works in debug mode)

[`tooling::Debugger`](../runtime/tooling/debugger.h) subscribes to these events and notificates debugger via hooks.
