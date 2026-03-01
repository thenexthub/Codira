# No flags to manage data structure state

## Severity
Error

## Applies To
/*.(cpp|h)$
language: cpp

## Detect

A violation occurs when **new flag enums** or **flag members** are added to manage state or control behavior of data structures, particularly:
- Extending `AstNodeFlags`, `CheckerStatus`, `ETSObjectFlags` enumerations
- Adding boolean flags to control compilation behavior
- Using flags to work around missing type information or incorrect design

**Why flags are harmful:**
- Harm data encapsulation and integrity
- Create hidden state dependencies
- Indicate incomplete fixes or workarounds
- Make code difficult to understand and maintain
- Often symptom of deeper architectural issues

**Common flag anti-patterns:**
```cpp
// BAD: Adding more flags to already problematic enums
enum class AstNodeFlags : uint16_t {
    NO_OPTS = 0,
    CHECKCAST = 1U << 0U,
    GENERATE_VALUE_OF = 1U << 4U,
    TMP_CONVERT_PRIMITIVE_CAST_METHOD_CALL = 1U << 8U,  // WRONG: temporary workaround flag
    MY_NEW_WORKAROUND = 1U << 9U  // WRONG: another workaround
};

enum class CheckerStatus : uint32_t {
    IN_GETTER = 1U << 26U,
    IN_SETTER = 1U << 27U,
    MY_NEW_CONTEXT_FLAG = 1U << 28U,  // WRONG: expanding problematic pattern
};
```

## Example BAD code

```cpp
// BAD: Using flags to control behavior
node->AddAstNodeFlags(ir::AstNodeFlags::GENERATE_VALUE_OF);
if (node->HasAstNodeFlags(ir::AstNodeFlags::GENERATE_VALUE_OF)) {
    // Special generation logic
}

// BAD: Expanding checker status flags
if (checker->HasStatus(CheckerStatus::IN_EXTENSION_ACCESSOR_CHECK)) {
    // Different behavior based on flag
}
```

## Example GOOD code

```cpp
// GOOD: Use explicit phase for systematic transformation
// See UnboxPhase, LambdaConversionPhase, etc. for reference
class ValueOfGenerationPhase : public Phase {
    void ProcessNode(ir::Expression* expr) {
        if (RequiresValueOfMethod(expr)) {
            ir::CallExpression* valueOfCall = CreateValueOfCall(expr);
            // Replace node properly in lowering
        }
    }
};

// GOOD: Use context objects instead of global flags
class AccessorCheckContext {
    bool in_extension_accessor_;
public:
    explicit AccessorCheckContext(bool in_extension)
        : in_extension_accessor_(in_extension) {}
    bool InExtensionAccessor() const { return in_extension_accessor_; }
};
```

## Fix suggestion

Instead of adding flags, address root causes:
- **Missing boxing logic**: Fix boxing/unboxing phases properly
- **Type conversion bugs**: Use TypeRelation API correctly
- **State management**: Use proper phase separation and context objects
- **Temporary workarounds**: Never acceptable; find proper solution

**Alternative approaches:**
- Use TypeRelation API for type-based decisions
- Create dedicated context objects for phase-specific state
- Implement proper lowering phases for transformations
- Use type information already available in checked AST

## Message

New flag '{FlagName}' added to '{EnumName}'. Flags harm encapsulation and indicate incomplete solutions. Address root cause: use TypeRelation API, proper lowering phases, or dedicated context objects instead of flags.
