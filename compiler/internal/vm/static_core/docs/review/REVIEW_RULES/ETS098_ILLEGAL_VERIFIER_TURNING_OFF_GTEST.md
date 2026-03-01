# Verifier should not be turned off in gtests

## Severity
Error

## Applies To
/*.(cpp|h)$
language: cpp

## Detect

Verifier is disabled via runtime options.

## Example BAD code

```cpp
// BAD: verification is disabled
std::pair<int, std::string> ExecPanda(verificationMode = "disable")
{
//...
    opts.SetLoadRuntimes({loadRuntimes});
    opts.SetVerificationMode(verificationMode); // verification is turned off
    opts.SetPandaFiles({file});
}
ExecPanda();
```

## Example GOOD code

```cpp
// GOOD: verification mode is not specified at all
std::pair<int, std::string> ExecPanda()
{
//...
    opts.SetLoadRuntimes({loadRuntimes});
    opts.SetPandaFiles({file});
}
ExecPanda();
```

## Fix suggestion

- **Use `opts.SetVerificationMode(ahead-of-time)`, `opts.SetVerificationMode(on-the-fly)`** instead
- **Do not specify verifierication mode manually at all**

## Message

Do not disable bytecode verification. Use permited modes or do not specify this option
