## Event tracking: Add try-catch block for throwable call-site
Suppose that some api is throwable, we need to add try-catch for some particular important call-site
```ts
// before AOP
class Handler {
    run() {
        let tmp;
        tmp = throwableAPI();
        return tmp;
    }
}

// after AOP
class Handler {
  run() {
    let tmp;
    try {
       tmp = ns.throwableAPI();
    } catch(e) {
       console.log(e);
       tmp = 0;
    }
    return tmp;
  }
}
```
