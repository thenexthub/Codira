# CodeStyle

Our CodeStyle based on [google code style](https://google.github.io/styleguide/cppguide.html) (you can get google config like this: `clang-format -dump-config -style=Google`).
But we have some modifications:

1. Indent: spaces 4. Line length: 120.
2. Delete spaces before public/private/protected.
3. All constants in UPPERCASE.
4. Enums in uppercase
   Example:

   ```cpp
   enum ShootingHand { LEFT, RIGHT };
   ```
5. Unix/Linux line ending for all files.
6. Same parameter names in Method definitions and declarations.
7. No `k` prefix in constant names.
8. No one-line if-clauses: 
   ```cpp
   if (x == kFoo) return new Foo();
   ```
9. Do not use special naming for getters/setters (google allows this: 
   ```cpp
   int count() and void set_count(int count))
   ```
10. Always explicitly mark fall through in switch … case. Google uses its own macro, we can agree on /* fallthrough */.
    ```cpp
    switch (x) {
      case 41:  // No annotation needed here.
      case 43:
        if (dont_be_picky) {
          // Use this instead of or along with annotations in comments.
          /* fallthrough */
        } else {
          CloseButNoCigar();
          break;
        }
      case 42:
        DoSomethingSpecial();
        /* fallthrough */
      default:
        DoSomethingGeneric();
        break;
    }
    ```
11. When a return statement is unreachable, but the language syntax requires it, mark it with something like return nullptr; /* unreachable */, or define UNREACHABLE as assert(0 && "Unreachable") and insert it before such return
12. Use standard notices in comments (e.g. TODO:, NB!, no FIXME: allowed).
13. Use standard flowerbox comments at the top of headers and translation units (agree on the format).
    Temporary you can use this:
    ```
     /**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2021. All rights reserved.
 */
     ```
14. switch and case on the same level
    For example:

    switch (ch) {
    case ‘A’:
        ...
    }
15. Always put { } even if the body is one line:
    For example
    ```cpp
    if (foo) {
        return 5;
    }
    ```
16. Use `maybe_unused` attribute for unused vars/arguments.
    ```cpp
    int foo3([[maybe_unused]] int bar) {
        // ...
    }
    ```

We are using clang-format and clang-tidy to check code style.
