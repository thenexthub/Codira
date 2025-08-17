//==--- DiagnosticRefactoringKinds.td - refactoring diagnostics -----------===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// 
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Refactoring Diagnostics
//===----------------------------------------------------------------------===//

let Component = "Refactoring" in {

let CategoryName = "Refactoring Invocation Issue" in {

def err_refactor_no_selection : Error<"refactoring action cannot be initiated "
  "without a selection">;
def err_refactor_selection_no_symbol : Error<"there is no symbol at the given "
  "location">;
def err_refactor_selection_invalid_ast : Error<"the provided selection does "
  "not overlap with the AST nodes of interest">;

def err_refactor_code_outside_of_function : Error<"the selected code is not a "
  "part of a function's / method's body">;
def err_refactor_extract_simple_expression : Error<"the selected expression "
  "is too simple to extract">;
def err_refactor_extract_prohibited_expression : Error<"the selected "
  "expression cannot be extracted">;

}

} // end of Refactoring diagnostics
