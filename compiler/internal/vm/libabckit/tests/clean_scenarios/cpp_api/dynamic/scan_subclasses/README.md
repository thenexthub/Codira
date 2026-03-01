## Scan subclasses

### Requirement

We want to detect all the subclasses of a given class.

### APP source code example

`modules/base.js:`
```js
export class Base {}
```
`scan_subclasses.js:`
```ts
import { Base } from './modules/base';

class Child1 extends Base {}
class Child2 extends Base {}
```

#### Input

Base class info:
```cpp
// {declaration_file_path, class_name}
{"modules/base", "Base"}
```

#### Output

List of subclasses info:
```cpp
// {file_path, subclassName}
{{"scan_subclasses", "Child1"}, {"scan_subclasses", "Child2"}};
```
