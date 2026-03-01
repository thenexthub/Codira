# PreferredType should be set locally

## Severity
Error

### Applies To

/*.(cpp|h)$
language: cpp

### Detect

Violation occurs if one of the following is true:

- `SetPreferredType` method for typed node is invoked after `Check` is invoked for the child node.
- Kind of child node is checked when calling `SetPreferredType` method.
- `preferredType_` of a node is changed, but its type is not dropped by invoking `node->SetTsType(nullptr)`.
- `preferredType_` of a node is used outside the `Check` method for that node.

### Example BAD code:

```typescript
void ArrayExpression::SetPreferredTypeBasedOnFuncParam(checker::ETSChecker *checker, checker::Type *param)
{
    // ... process other possible types
    if (param->IsETSArrayType()) {
        auto *elementType = param->AsETSArrayType()->ElementType();
        for (auto *const elem : elements_) {
            elem->SetPreferredType(elementType);
        }
    }
}

bool ETSChecker::ValidateSignature(ir::CallExpression *call, Signature *sig) {
    auto &args = call->Arguments();
    for (size_t ix = 0; ix < args.size(); ix++) {
        auto *arg = args[ix];
        auto *paramType = GetTypeOfArgumentByIndex(sig, ix);
        if (arg->IsArrayExpression()) {
            arg->AsArrayExpression()->SetPreferredTypeBasedOnFuncParam(this, paramType);
        }
        auto *argType = arg->Check(this);
        // ... check that argType is assignable to the paramType
    }
}
```

Here, the `SetPreferredTypeBasedOnFuncParam` method is invoked when processing a call expression node that is the parent of an array literal, not the literal node itself. This method sets the preferred type of the array literal elements, which are not immediate children of the call expression.

Moreover, preferred type is set _only_ for arguments that are array expressions. (In the actual checker code, from which this example is a simplified fragment, other context-sensitive expression types, such as lambdas and number literals, are treated by separate pieces of code and in a different manner, which harms uniformity.)

### Example GOOD code:

```typescript
bool ETSChecker::ValidateSignature(ir::CallExpression *call, Signature *sig) {
    auto &args = call->Arguments();
    for (size_t ix = 0; ix < args.size(); ix++) {
        auto *arg = args[ix];
        auto *paramType = GetTypeOfArgumentByIndex(sig, ix);
        arg->SetPreferredType(paramType);
        auto *argType = arg->Check(this);
        // ... check that argType is assignable to paramType
    }
}

Type *ETSAnalyzer::Check(ir::ArrayExpression *expr) {
    ETSChecker *checker = Checker();
    auto *preferredType = expr->PreferredType();
    auto *elems = expr->Elements()
    for (size_t ix = 0; ix < elems.size(); ix++) {
        auto *expectedElemType = GetExpectedTypeByIndex(preferredType, ix); // different for arrays and tuples
        auto *e = elems[ix];
        e->SetPreferredType(expectedElemType);
        auto *actualElemType = e->Check(checker);
        // ... keep track of element types
    }
    // ... compute and return the actual type of the array expression
}
```
