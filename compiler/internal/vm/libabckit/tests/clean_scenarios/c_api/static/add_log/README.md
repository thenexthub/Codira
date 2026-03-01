## Event tracking: Add log
## Add log
We want to add log to all functions, where the log content is the function full name and execute time.
```js
// before AOP

class MyClass {
    handle() {
        print('abckit');
    }
}
```

```js
// after AOP

export class MyClass {
  handle() {
    console.log("file: src/MyClass, function: MyClass.handle");
    let t1 = new Date().getTime();
    // business logic
    let t2 = new Date().getTime();
    console.log("Ellapsed time\n:", t2 - t1);
  }
}
```
### Output before aop
```
abckit
```

### Output after aop
```
file: src/MyClass, function: MyClass.handle
abckit
Ellapsed time:
12
```