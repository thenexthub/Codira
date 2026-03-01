# Verifier should pass all stages turned on

## Severity
Error

## Applies To
/*.(verifier.config)$

## Detect

A violation occurs when verifier config file contains section `check` but one of the option from the list is missed. List: `cflow`, `resolve-id`, `typing`, `absint`

## Example BAD code

```.config
// BAD: `absint` check is missed
debug {
  method_options {
    verifier {
      default {
        check {
          cflow, resolve-id, typing
        }
      }
    }
  }
}
```

## Example GOOD code

```.config
// GOOD: all verification options are listed
debug {
  method_options {
    verifier {
      default {
        check {
          cflow, resolve-id, typing, absint
        }
      }
    }
  }
}
```

## Fix suggestion

- **List all options** `cflow`, `resolve-id`, `typing`, `absint`
- **Do not specify  check option at all**

## Message

One of the option in verifier.config file is missed
