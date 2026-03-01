# AST nodes must be immutable after creation

## Severity
Error

## Applies To
/*.(cpp|h)$
language: cpp

## Detect
A violation occurs when code **mutates existing AST nodes** after they have been created and integrated into the AST, particularly:
- Adding mutator methods to ASTNode classes (setters, `Add*`, `Set*`, `Remove*`)
- Modifying AST node structure outside of initial construction
- Changing parent-child relationships after construction
- Modifying node properties after type checking

**Why immutability matters:**
- Front-end compilers are not friendly to mutations in compilation state
- Mutation breaks phase separation and makes debugging difficult
- Violates assumptions made by analysis passes
- Creates hidden dependencies between compilation phases

**Allowed modifications (not violations):**
- Setting `TsType` during type checking phase
- Setting `Variable` during variable binding phase
- Annotating nodes with analysis results in dedicated fields

**Violation patterns:**
```cpp
// BAD: Adding mutator to ASTNode
class ETSFunctionType : public Type {
public:
    void AddCallSignature(Signature* signature);  // WRONG: mutation after creation
    void RemoveParameter(size_t index);           // WRONG: mutation after creation
};
```

## Example BAD code

```cpp
// BAD: Mutating existing function type
class ETSFunctionType : public Type {
    void AddCallSignature(Signature* signature) {
        signatures_.push_back(signature);  // Modifying after creation
    }
};

// Usage
auto* funcType = /* existing function type */;
funcType->AddCallSignature(newSignature);  // WRONG: mutation
```

## Example GOOD code

```cpp
// GOOD: Create new type with desired signatures
ArenaVector<Signature*> signatures(allocator->Adapter());
signatures.insert(signatures.end(), existingType->CallSignatures().begin(),
                  existingType->CallSignatures().end());
signatures.push_back(newSignature);

auto* newFuncType = checker->CreateETSMethodType(/* params */, signatures, /* params */);

// GOOD: Immutable design - no mutation methods
class ETSFunctionType : public Type {
public:
    // Only const accessors, no mutators
    const ArenaVector<Signature*>& CallSignatures() const { return signatures_; }

    // NO mutation methods like AddSignature, RemoveSignature, etc.
};
```

## Fix suggestion

- **Create new AST nodes** instead of modifying existing ones
- Use **lowering phases** to transform AST by creating new structures
- Design immutable data structures from the start
- Use factory methods that create complete nodes in one step

If you need a different AST structure:
1. Create new nodes with the desired properties
2. Use lowering phases for systematic transformations
3. Never modify nodes that are already part of the AST

## Message

AST node mutation detected in '{ClassName}::{MethodName}'. AST nodes must be immutable after creation. Create new nodes instead of modifying existing ones. Use lowering phases for systematic AST transformations.
