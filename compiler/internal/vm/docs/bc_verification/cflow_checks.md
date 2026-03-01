## Current layout model of a method

```
=========
   ...
  code
   ...
---------
   ...
exception
handler
   ...
---------
   ...
  code
   ...
---------
   ...
exception
handler
   ...
 -------
 inner
 exc.
 handler
 -------
   ...
---------
   ...
  code
   ...
=========

```

I.e. layout of exception handlers is rather flexible, even
handler in handler is allowed.

## Cflow transitions, which are subjects for checks

### Execution beyond method body

```
=========
   ...
  code
   ...
---------
   ...
exception
handler
   ...
---------
   ...
  code
   ...
  ldai 0 ---\
=========   |
       <----/
```

```
=========
   ...
  code
   ...
---------
   ...
exception
handler
   ...
   jmp -----\
   ...      |
=========   |
       <----/
```

Mis-jumps, or improper termination of cflow at the end of the body are prohibited.

```
=========
   ...
  code
   ...
---------
   ...
exception
handler
   ...
---------
   ...
lbl:  <-----\
   ...      |
  code      |
   ...      |
  jeqz lbl -+
=========   |
       <----/
```

Conditional jumps are in grey zone, if they may be proven as always jump
into code, then they will be considered ok. Currently, due to imprecision
of verifier, conditional jumps at the end of the method are prohibited.

### Code to exception handler

direct jumps:

```
=========
   ...
  code
   ...
   jmp catch1--\
   ...         |
---------      |
catch1: <------/
   ...
exception
handler
   ...
---------
   ...
```

fallthrough:

```
=========
   ...
  code
   ...
   ldai 3 --\
---------   |
catch1: <---/
   ...
exception
handler
   ...
---------
   ...
```

By default only `throw` transition is allowed. Neither `jmp`, nor
fallthrough on beginning of exception handler are allowed.

This behavior may be altered by option `C-TO-H`.

### Code into exception handler

```
=========
   ...
  code
   ...
   jmp lbl1  --\
   ...         |
---------      |
catch:         |
   ...         |
lbl1:     <----/
   ldai 3
   ...
exception
handler
   ...
---------
   ...
```

Jumps into body of exception handler from code is prohibited by default.

### Handler to handler

direct jumps:

```
=========
   ...
  code
   ...
---------
catch1:
   ...
exception
handler
   ...
   jmp catch2--\
   ...         |
---------      |
catch2: <------/
   ...
exception
handler
   ...
---------
   ...
```

fallthrough:

```
=========
   ...
  code
   ...
---------
catch1:
   ...
exception
handler
   ...
   ldai 3 --\
---------   |
catch2: <---/
   ...
exception
handler
   ...
---------
   ...
```

By default such transition of control flow is prohibited.

### Handler into handler

direct jumps:

```
=========
   ...
  code
   ...
---------
catch1:
   ...
exception
handler
   ...
   jmp lbl  ---\
   ...         |
---------      |
catch2:        |
   ...         |
lbl:    <------/
   ldai 3
   ...
exception
handler
   ...
---------
   ...
```

fallthrough from inner handler:

```
=========
   ...
  code
   ...
---------
catch1:
   ...
outer
exception
handler
   ...
 -------
catch2:
   ...
lbl:
   ldai 3
   ...
 inner
 exc.
 handler
   ...
  ldai 0  --\
 -------    |
   ...   <--/
outer
exc.
handler
   ...
---------
   ...
```

By default such cflow transitions are prohibited.

### Handler into code

```
=========
   ...
  code
   ...
lbl:   <-------\
   ...         |
---------      |
   ...         |
exception      |
handler        |
   ...         |
   jmp lbl  ---/
   ...
---------
   ...
```

By default such jumps are prohibited currently.
