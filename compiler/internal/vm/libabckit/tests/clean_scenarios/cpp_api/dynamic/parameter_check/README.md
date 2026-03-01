## Event tracking: Parameter check
Suppose we have the following abstract class,

`modules/base:`
```js
export class Handler {
    handle(arr, idx) {};
}
```
`parameter_check.js:`
We want to add parameter check for all implementations of `handle`, where the check logic is that `idx` should be less than `arr.length`
```ts
// before AOP
import { Handler } from './modules/base';
class StringHandler1 extends Handler {
    handle(arr, idx) {
        return arr[idx];
    }
}

// after AOP

import { Handler } from './modules/base';
class StringHandler1 extends Handler {
    handle(arr, idx) {
        if (idx >= arr.length) {
            return -1;
        }
        return arr[idx];
    }
}
```
