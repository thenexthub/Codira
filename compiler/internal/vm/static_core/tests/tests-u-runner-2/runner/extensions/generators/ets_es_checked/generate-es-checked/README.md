# ETS cross-validator with TS

This generator works as follows:
- reads configuration files ([example](../../../../plugins/ets/tests/ets_es_checked/array.yaml))
- recursively traverses configuration tree and looks for [properties](#configuration-properties)
- generates `.ts` file based upon configuration
- executes `.ts` file to get json of golden answers
- generates `.ets` file with in-place assertions
- (optionally) launches this `.ets` file, note that test-u-runner should be used outside of initial testing

Example of how to run:
```bash
cd "$PROJECT"
./urunner/extensions/generators/ets_es_checked/generate-es-checked/main.rb \
    --out /tmp/eschecked/out \
    --tmp /tmp/eschecked/tmp \
    --run-ets "$(readlink -f ./build)" \
    --ts-node=npx:--prefix:./tests/tests-u-runner/tools/generate-es-checked/:ts-node:-P:./tests/tests-u-runner/tools/generate-es-checked/tsconfig.json \
    --filter 'Array.*' \
    plugins/ets/tests/ets_es_checked/*.yaml
```

See generator `--help` option for detailed description of console flags.

## Configuration structure
```haskell
RubyExpr = String
ArkTSType = String

File
  = Tree
  * top_scope?: String
  * category?: String

Tree
  = Description
  | Endpoint

Description
  = CommonDescription
  * sub?: Array Tree
  * vars?: Map String YAMLObject

CommonDescription
  = excluded?: Bool
  * self_type?: ArkTSType
  * self?: Array String | null
  * setup?: String

Endpoint
  = (method: String) * EndpointDescription
  | (expr: String) * EndpointDescription

EndpointDescription
  = CommonDescription
  * params?: Array RubyExpr
  * rest?: RubyExpr
  * mandatory?: UInt
  * ret_type?: ArkTSType
  * special:? Array Special

special
  = match: RegExp
  * action: "exclude" | "warn as error" | "silence warn" | "report"
```

## Configuration properties
- `ignored` skip subtree
- `category` prefix name of tests
- `self_type` type of self, if it is complex
- `vars` variables that are available in ruby code
- `sub` recursively traverse it's children
- endpoints, i.e. expressions to test
  - `method` name of method to test
  - `expr` expression to test; if it contains word `pars`, it will be substituted with generated parameters
  - endpoint configuration:
    - `params` list of either:
      - ruby expressions for generating each parameter (i.e. functions that return lists of values)
      - plain string representing all parameter values
    - `self` values of `this` for methods; plain string
    - `setup` code to execute before running a test
    - `ret_type` type of returned expression
    - `mandatory` number of mandatory params (useful for optionals testing)
    - `rest` generator for rest parameters, `combinationRest` is most useful, as it yields all combinations of arguments
    - `special` special cases array: match tested ts expression and handle it in a special way

## Internal features
### Types
Types (`self_type`, `ret_type`) represent ArkTS types in ArkTS grammar:

```Haskell
Type
  = "Array<" Type ">"
  | "Iterable<" Type ">"
  | "[" Type ("," Type)+ "]"
  | TrivialType

TrivialType
  = Ident ("<" (TrivialType ",")* ">")?
  | TrivialType "|" TrivialType
  | "[" TrivialType ("," TrivialType)+ "]"
```

They are split into two categories, as `TrivialType` is interpreted as whole (string) and isn't used for destructuring result acquired from ts

### Test splitting
- By default tests are split into multiple files to prevent es2panda segfault
- Each file is split into multiple functions, so that jit/aot doesn't fail with message "bytecode is too long"
### Mangling and dumping
json file format can't represent all possible values, including `undefined`, `NaN`, `Infinity`, include exceptions info, ill formed strings. To do so mangling was introduced:
- strings that start with `#__` in json have special meaning
- objects that contain `__kind` property have special meaning
- numbers can be stored as bytes to prevent precision errors in parsers and so on

## Installation help
See test-u-runner [readme](../../readme.md#ets-es-checked-dependencies)

## Extending generator
- manglings reside in `ESChecker::UNMANGLE`, `ESChecker::ValueDumper` does dumping of expected values
- `ESChecker::Types` module parses types and detects (predicts) them based on expected values
- function `preparePersist` in ts makes a json and mangles most of the values
- generators for parameters reside in a module named `ScriptModule`

## Type replacement number/int for .ts/.ets in templates
- pattern %int% will be replaced with "number" type for .ts & "int" for .ets
- pattern %.toInt()% will be replaced with "" type for .ts & ".toInt()" for .ets

