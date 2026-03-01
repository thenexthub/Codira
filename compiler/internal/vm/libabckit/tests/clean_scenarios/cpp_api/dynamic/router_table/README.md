## Router table

Suppose that we have many annotated classes and one map, we need to collect all these classes and created instances and push into classes.

`IRouterHandler.ets:`
```ts
export abstract class IRouterHandler {
    abstract handle(): void;
}
```
We have such classes:
`xxxHandler.ets:`
```ts
import {
    IRouterHandler
} from './IRouterHandler'

@interface __$$ETS_ANNOTATION$$__Router {
    scheme: string;
    path: string;
}

@__$$ETS_ANNOTATION$$__Router({
    scheme: 'handle1',
    path: '/xxx'
})
export class xxxHandler extends IRouterHandler {
    handle(): void {
        print('xxxHandler.handle was called');
    }
}
```

`routerMap.ets:`
```ts
//  before AOP
import {IRouterHandler } from './IRouterHandler';
export let routerMap = new Map<string, IRouterHandler>([]);

// after AOP
// need to insert code into routerMap.ets
import {IRouterHandler} from "./IRouterHandler";
import {xxxHandler} from "./xxxHandler";
// we have many such imports

export let routerMap = new Map<string, IRouterHandler>([
 ["imeituan/xxx", new xxxHandler()],  // [schema + path, new handler()]
]);
```

