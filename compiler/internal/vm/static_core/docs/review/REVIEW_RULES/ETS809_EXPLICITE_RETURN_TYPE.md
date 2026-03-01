# Explicite return type

## Severity
Warning

## Applies To
/*.(ets|sts)$
language: ts

## Detect
A violation is a function declaration or method declaration or lambda declaration which have no return type.

## Example BAD code
```typescript
function Test() {
    return "Test";
}

class C {
    static Create() {
        return 5;
    }
}

let f = () => { return 0; }
```

## Example GOOD code
```typescript
    function Toto(): string {
        return "toto";
    }
}

class Good {
    public count(): number { return 7; }
}

let tata = (): void =>  { return; }
```

## Fix suggestion
Add return type.

## Message
Defining return type for function-like declarations is mandatory.