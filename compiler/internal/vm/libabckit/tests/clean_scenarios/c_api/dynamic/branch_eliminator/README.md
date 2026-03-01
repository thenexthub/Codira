
## Bytecode optimization: branch elimination
### Requirement

We have the following source code
```ts
// before AOP
export class Mybar {
    static test1() {
        if (Config.isDebug) {
            print('Mybar.test1: Config.isDebug is true');
        } else {
            print('Mybar.test1: Config.isDebug is false');
        }
    }

    test2() {
        if (!Config.isDebug) {
            print('Mybar.test2: Config.isDebug is false');
        } else {
            print('Mybar.test2: Config.isDebug is true');
        }
    }
}
```
We want to delete all the braches with Config.isDebug == True in condition.
```ts
// after AOP
export class Mybar {
    static test1() {
        print('Mybar.test1: Config.isDebug is false');
    }

    test2() {
        print('Mybar.test2: Config.isDebug is false');
    }
}
```
