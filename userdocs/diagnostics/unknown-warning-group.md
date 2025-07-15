# Unknown warning group (UnknownWarningGroup)

Warnings for unrecognized warning groups specified in `-Wwarning` or `-Werror`.


## Overview

```sh
languagec -Werror non_existing_group file.code
<unknown>:0: warning: unknown warning group: 'non_existing_group'
```

Such warnings are emitted after the behavior for all specified warning groups has been processed, which means their behavior can also be specified. For example:

```sh
languagec -Werror UnknownWarningGroup -Werror non_existing_group file.code
<unknown>:0: error: unknown warning group: 'non_existing_group'
```
