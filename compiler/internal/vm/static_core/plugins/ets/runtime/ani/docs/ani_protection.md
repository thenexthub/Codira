# Possible protection modes in ANI

The ANI allows to support various the protection modes to protect the VM from malfunctions or crashes.
The table below shows the possible modes and the degrees of protection they provide.

- **Fast mode** - no protection
- **Security mode** - validate argument types passed to ANI API
- **Strong security mode** - validate argument types passed to ANI API, protect an invalid pointer dereference, the stack corruption protection
- **Micro-service mode** - executing the native code in another process

| Ways to crash the VM                                       |Fast mode|Security mode|Strong security mode|Micro-service mode|
| ---------------------------------------------------------- | ------- | ----------- | ------------------ | ---------------- |
| Pass invalid args to the ANI API [N->M]                    |    -    |      full   |           full     |        full      |
| Incorrect declaration of the method signature [M->N]       |    -    |    partial  |         partial    |        full      |
| Dereference an invalid pointer                             |    -    |        -    |           full     |        full      |
| Mess up the stack and other memory in the VM               |    -    |        -    |         partial    |        full      |
| Resource leaks (memory leaks, file descriptor leaks, etc.) |    -    |        -    |             -      |        full      |
| Unhandled C++ exception                                    |    -    |        -    |             -      |        full      |
| fork()/exec() VM process                                   |    -    |        -    |             -      |        full      |
