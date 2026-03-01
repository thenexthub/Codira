# Style guide

Identifiers in a schema are meant to translate to many different programming languages, so using the style of your "main" language is generally a bad idea.

For this reason, below is a suggested style guide to adhere to, to keep schemas consistent for interoperation regardless of the target language.

Where possible, the code generators for specific languages will generate identifiers that adhere to the language style, based on the schema identifiers.

- Table, struct, enum and rpc names (types): UpperCamelCase.
- Table and struct field names: snake_case. This is translated to lowerCamelCase automatically for some languages, e.g. Java.
- Enum values: UpperCamelCase.
- namespaces: UpperCamelCase.

Formatting (this is less important, but still worth adhering to):

- Opening brace: on the same line as the start of the declaration.
- Spacing: Indent by 2 spaces. None around `:` for types, on both sides for `=`.
