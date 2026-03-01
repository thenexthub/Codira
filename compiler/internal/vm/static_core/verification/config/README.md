## Verifier configuration

### Debug_breakpoint for managed code

Debug_breakpoint for managed code are used to stop GDB just right before processing
instruction at specified offset in managed code

format: `<method_name>[,<offset>]*`

if no offset specified, default is 0.

offset format: dec or hex (0x...)

### Verifier whitelists

#### Whitelist for methods

List with methods full names, which will be considered as verified.

## Whitelist for method calls

List of methods, calls to which always be considered as correct (signature checks is turned off for them)

### Example of config

```
debug {
  breakpints {
    verifier {
      java.lang.System::arrayCopy, 0x55, 0x66
      java.lang.System::arrayRemove
    }
  }
  whitelist {
    verifier {
      method {
        Object::monitorEnter
        Object::monitorExit
      }
      method_call {
        Object::monitorEnter
        Object::monitorExit
      }
    }
  }
}

```

Note newline after last `}`.