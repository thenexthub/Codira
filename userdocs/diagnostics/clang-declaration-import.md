# Imported Clang declaration warnings (ClangDeclarationImport)

Covers all warnings related to malformed APIs that are imported into Codira from C, C++, and Obj-C headers.


## Overview

As one example of a potential malformed API diagnostic, suppose a Clang module dependency contained the following declaration:

```
typedef int NotificationIdentifier
    __attribute__((language_name("Notification.Identifier")))
```

The Codira compiler would emit the following warning if no type named `Notification` could be found:

```
warning: imported declaration 'NotificationIdentifier' could not be mapped to 'Notification.Identifier’
```
