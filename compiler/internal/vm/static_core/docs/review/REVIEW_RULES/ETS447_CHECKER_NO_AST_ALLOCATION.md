# Checker must not allocate or modify AST nodes

## Severity
Error

## Applies To
/*.(cpp|h)$
language: cpp

## Detect

A violation occurs when **type checker code** (`ETSChecker`, `ETSAnalyzer`, or checker visitor methods) performs any of the following:
- **Allocates new AST nodes** using `AllocNode<T>()` or similar
- **Modifies existing AST nodes** (except setting `TsType` or `Variable`)
- **Creates AST fragments** for code generation or transformation
- **Restructures the AST** during type analysis

**Why separation matters:**
- Checker should only do **type resolution** and **type checking**
- AST transformation belongs in **lowering phases** (after checker)
- Mixing concerns makes debugging nearly impossible
- Violates single responsibility principle

**Allowed checker operations:**
- Setting `node->SetTsType(type)` - type annotation
- Setting variable references during binding phase
- Reading AST structure for analysis
- Creating and manipulating Type objects (not AST nodes)

**Violation patterns:**
```cpp
// In ETSChecker or analyzer methods:
void ETSChecker::GenerateGetterSetterBody(...) {  // WRONG: "Generate" in checker
    auto* memberExpression = ProgramAllocNode<ir::MemberExpression>(...);  // WRONG
    auto* identifier = field->Key()->AsIdentifier()->Clone(...);           // WRONG
}
```

## Example BAD code

```cpp
// BAD: Checker generating AST nodes
void ETSChecker::GenerateGetterSetterBody(ir::MethodDefinition* getter,
                                          ir::ClassProperty* field) {
    auto* memberExpr = ProgramAllocNode<ir::MemberExpression>(
        baseExpression,
        field->Key()->AsIdentifier()->Clone(ProgramAllocator(), nullptr),
        ir::MemberExpressionKind::PROPERTY_ACCESS, false, false);

    auto* returnStmt = ProgramAllocNode<ir::ReturnStatement>(memberExpr);
    getter->SetBody(returnStmt);  // WRONG: Modifying AST in checker
}
```

## Example GOOD code

```cpp
// GOOD: Checker only validates types
void ETSChecker::CheckGetterSetterTypes(ir::MethodDefinition* getter,
                                       ir::ClassProperty* field) {
    Type* getterReturnType = getter->ReturnTypeAnnotation()->Check(this);
    Type* fieldType = field->TsType();

    if (!Relation()->IsIdenticalTo(getterReturnType, fieldType)) {
        ThrowTypeError("Getter return type must match field type", getter->Start());
    }
}

// GOOD: Separate lowering phase for AST transformation
class GetterSetterBodyGenerationPhase : public Phase {
    void GenerateGetterBody(ir::MethodDefinition* getter, ir::ClassProperty* field) {
        // Now it's appropriate to allocate AST nodes
        auto* memberExpr = AllocNode<ir::MemberExpression>(...);
        auto* returnStmt = AllocNode<ir::ReturnStatement>(memberExpr);
        getter->SetBody(returnStmt);
    }
};
```

## Fix suggestion

**Move AST generation/modification to post-checker lowering phases:**
1. Create a dedicated lowering phase (e.g., `GetterSetterGenerationPhase`)
2. Implement transformation logic in the lowering
3. Run lowering after checker phase completes
4. Reference existing lowerings: `OptionalArgumentsLowering`, `StringComparisonLowering`, `LambdaConversionPhase`, `UnboxLowering`

**Checker should only:**
- Validate type correctness
- Compute types for expressions
- Check type compatibility
- Report type errors

## Message

Checker code in '{ClassName}::{MethodName}' is allocating or modifying AST nodes. Checker should only perform type resolution and checking. Move AST generation/transformation to post-checker lowering phase (see OptionalArgumentsLowering, LambdaConversionPhase for reference).
