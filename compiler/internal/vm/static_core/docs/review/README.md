# DevAgent Review Rules

## Rule codes

- `ETS0XX`: General development rules
  - `ETS0[0..1]X`: general rules
  - `ETS09X`: bytecode verification rules
  - `ETS0[2..8]X`: unused
- `ETS4XX`: ArkTS compiler ("front-end") development rules
  - `ETS40X`: general rules
  - `ETS4[4..5]X`: `checker/` development rules
  - `ETS47X`: `varbinder/` development rules
  - `ETS49X`: `ir/` development rules
  - `ETS4[1..3]X`, `ETS46X`, `ETS48X`: unused
- `ETS5XX`: Rules for contributing to ArkTS test suites
- `ETS8XX`: ArkTS StdLib development rules

## Rules writing guidelines

### Validation

Each rule is required to pass validation. After implementing a new rule,
run corresponding `perl` script using command `./validate` from current
directory. Fix all errors shown by script until it finishes successfully.

### Detection

#### Syntax description

To specify a certain syntax or construct in the rule, it might be more effective to give a 'visualized' syntax description in a simplified form, rather than trying to fully describe it in words. For example, `try { ... } catch (...) { ... }` for a try-catch statement, `for (initialization; condition; increment/decrement) { ... }` or `for (variable of iterable) { ... }` to indicate the certain form of `for` loop syntax. This may help to increase the detection accuracy and reduce the number of false positives that may potentially occur when encountering similar keyword sequence in the code (e.g. a call chain `new Promise().then().catch().finally()`).

#### Wording

If the rule doesn't perform well despite having seemingly good description,
try changing some words or the general phrasing: use different terms to describe
what needs to be detected by the rule; try to highlight significant keywords by
enclosing it with backticks (`word`) or double-stars (**word**).

Sometimes even slight changes in the phrasing may affect the results significantly.

#### Complex logic

The rules tend to perform poor when asked to perform multiple tasks at the same
time (e.g. detect several problematic types in different contexts, or check various
language constructs with different conditions). The detection results may be improved
by splitting complex detection logic into separate rules, with each rule performing
one certain detection task.

#### Code examples

In some cases, adding specific code examples to the rule description can worsen its
detection ability. A possible reason is that agent is being given a lot of different
information at the same time, which together starts to become `contradictory` and
misleads the detection process. In this case, it's better to simplify code examples
or even remove it from rule entirely, simply adding `N/A` in corresponding section.

One of such examples is the rule that validates the declaration naming style. Such
rule stops working properly when given examples of different incorrect naming styles,
presumably because such examples in the context of naming aren't interpreted correctly
and start to mislead the detection.
