# No hardcoded AST pattern matching for type checking

## Severity
Error

## Applies To
/*.(cpp|h)$
language: cpp

## Detect

A violation occurs when type checking logic relies on **hardcoded AST node patterns** using `Is<Node>Expression/Statement` methods, such as:
- Pattern matching expression types: `IsCallExpression`, `IsBinaryExpression`, `IsArrayExpression`
- Checking statement structure: `IsExpressionStatement`, `IsBlockStatement`
- Deep pattern matching with multiple nested `Is*` checks
- Using AST shape to determine type behavior

**Why this is problematic:**
- AST structure is an implementation detail, not semantic contract
- Breaks when AST changes (e.g., during parsing refactoring)
- Misses semantic equivalents with different AST representations
- Creates fragile, hard-to-maintain code
- Violates separation of concerns (syntax vs semantics)

**Allowed exceptions** (not violations):
- `MemberExpression` in `CallExpression` for context `a.b()` (spec-defined)
- `ETS_NUMERIC` type check for `+` and arithmetic operators (spec-defined)

## Example BAD code

```cpp
// BAD: Hardcoded AST pattern for type checking
void ETSChecker::CheckThisOrSuperCallInConstructor(ETSObjectType* classType,
                                                   Signature* ctorSig) {
    for (auto it : ctorSig->Function()->Body()->AsBlockStatement()->Statements()) {
        if (it->IsExpressionStatement() &&
            it->AsExpressionStatement()->GetExpression()->IsCallExpression() &&
            (it->AsExpressionStatement()->GetExpression()->AsCallExpression()
                ->Callee()->IsThisExpression() ||
             it->AsExpressionStatement()->GetExpression()->AsCallExpression()
                ->Callee()->IsSuperExpression())) {
            // Can detect "this.fn();" but misses "this.fn() + 1;"
        }
    }
}

// BAD: Complex AST pattern matching
void CheckExpressionsInConstructor(const ArenaVector<const ir::Expression*>& arguments) {
    for (auto* arg : arguments) {
        if (arg->IsETSNewClassInstanceExpression()) {
            // ...
        } else if (arg->IsArrayExpression()) {
            // ...
        } else if (arg->IsBinaryExpression()) {
            // ...
        } else if (arg->IsAssignmentExpression()) {
            // ...
        } else if (arg->IsTSAsExpression()) {
            // ...
        }
        // WRONG: Too many AST shape checks
    }
}
```

## Example GOOD code

```cpp
// GOOD: Use checker context status instead of AST patterns
void ETSChecker::CheckConstructorBody(ir::BlockStatement* body) {
    ScopedCheckerStatus constructorContext(this, CheckerStatus::IN_CONSTRUCTOR);

    // Semantic checks based on context, not AST shape
    for (auto* stmt : body->Statements()) {
        stmt->Check(this);  // Type checking uses context status
    }
}

// GOOD: Use type information, not AST structure
void ProcessExpression(ir::Expression* expr, ETSChecker* checker) {
    Type* exprType = expr->TsType();

    if (checker->Relation()->IsSupertypeOf(checker->GlobalETSObjectType(), exprType)) {
        // Handle object types semantically
    }
}

// GOOD: AST traversal in lowering phase (not checker)
class ConstructorInitializationPhase : public Phase {
    void ProcessNode(ir::AstNode* node) {
        node->Iterate([this](ir::AstNode* child) {
            // Systematic traversal in lowering is appropriate
            if (RequiresInitialization(child)) {
                TransformNode(child);
            }
        });
    }
};
```

## Fix suggestion

**Use semantic information, not AST patterns:**
- Use **Type information** from `node->TsType()`
- Use **TypeRelation API** for type-based decisions
- Use **CheckerContext statuses** (e.g., `CheckerStatus::IN_CONSTRUCTOR`)
- Use **node visitor patterns** in lowering phases (not checker)
- For complex patterns, ask: "Does this correspond to spec semantics?"

If you need AST traversal, do it in **lowering phases** with proper visitors.


## Message

Hardcoded AST pattern matching detected using Is{NodeType} methods in '{FunctionName}'. Type checking should use semantic information (Type, TypeRelation, CheckerContext) not AST structure. Move AST traversal to lowering phases if needed.
