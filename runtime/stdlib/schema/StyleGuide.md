# ===----------------------------------------------------------------------===
#
#  Copyright (c) NeXTHub Corporation. All Rights Reserved.
#  DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
#  Author: Tunjay Akbarli
# 
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at:
# 
#  http://www.apache.org/licenses/LICENSE-2.0
# 
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
# 
#  Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
#  Middletown, DE 19709, New Castle County, USA.
#
# ===----------------------------------------------------------------------===

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
