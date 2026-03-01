# Do not use labeled statements

## Severity
Warning

## Applies To
/*.(ets|sts)$
language: ts

## Detect
A violation is a loop statement has a label in form `m1: for (variable of expression) { ... }`.

## Example BAD code
```typescript
m:
    for ( ; ; ) {
        console.log(s);
        break m;
    }
}

start: while(true) {
   console.log("K");
   break start;
}
```

## Example GOOD code
```typescript
    for (let i = 0; i < 10; i++) {
        console.log(i);
        break;
    }
}
```

## Fix suggestion
Remove label.

## Message
Usage of label with loop statements is depreacted.