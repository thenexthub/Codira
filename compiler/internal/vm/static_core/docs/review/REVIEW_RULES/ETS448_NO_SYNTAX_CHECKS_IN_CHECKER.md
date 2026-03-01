# No syntax checks in checker

## Severity
Error

### Applies To

/*.(cpp|h)$
language: cpp

### Detect

A violation occurs if logic added to the checker does not involve any reference to types (`checker::Type *`) or function signatures (`checker::Signature *`). Violation also occurs if code added to the checker does not refer to the scopes (`varbinder::Scope *`) or variables (`varbinder::Variable *`).

### Fix suggestion

If added code does not refer to types or signatures, you should move it either
- into the parser, or
- into a separate phase that executes before the checker.

If added code does not refer to scopes or ariables, it should be performed inside `ScopesInitPhase` or `ResolveIdentifiers` classes.