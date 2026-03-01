
## Annotation: Replace call-site
Suppose that we use annotation to represent call-site replacement info:

```ts
// layout.ets
export class LinearLayout {
    num_: number;

    setOrientation(num: number) {
        this.num_ = num;
    }

    getOrientation(): number {
        return this.num_;
    }
}

// ApiControl.ets
import {
    LinearLayout
} from './layout'

@interface __$$ETS_ANNOTATION$$__CallSiteReplacement {
    targetClass: string;
    methodName: string;
}

export class ApiControl {
    @__$$ETS_ANNOTATION$$__CallSiteReplacement({
        targetClass: 'LinearLayout',
        methodName: 'setOrientation'
    })
    public static fixOrientationLinearLayout(target: LinearLayout, orientation: number) {
        print('fixOrientationLinearLayout was called');
        target.setOrientation(orientation);
    }
}

```
### Requirement

collect all `CallSiteReplacement` annotations and do the replacement.
```ts
// before AOP
import {
    LinearLayout
} from './modules/layout'

class MyClass {
    foo() {
        let appColumn = new LinearLayout();
        appColumn.setOrientation(3);
        print(appColumn.getOrientation());
    }
}

// after AOP
import {
    LinearLayout
} from './modules/layout'

import {
    ApiControl
} from "./ApiControl"

class MyClass {
    foo() {
        let appColumn = new LinearLayout();
        appColumn.setOrientation(3);
        ApiControl.fixOrientationLinearLayout(appColumn, 5);
        print(appColumn.getOrientation());
    }
}
```

### Output before AOP
```
3
```

### Output after AOP
```
fixOrientationLinearLayout was called
5
```
