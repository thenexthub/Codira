/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

/**
 * @file
 *
 * This file declares the symbol table printing function.
 */

#ifndef CODIRA_AST_PRINTSYMBOLTABLE_H
#define CODIRA_AST_PRINTSYMBOLTABLE_H

#include "Codira/Frontend/CompilerInstance.h"

namespace Codira {
/**
 * Print the symbol tables of the compiler instance.
 *
 * The output is formatted as JSON and the following schema is required:
 *
 * {
 *   "packages": [
 *     {
 *       "name": <name of the package>,
 *       "files": [
 *         <path of the file>
 *       ]
 *     }
 *   ],
 *   "files": [
 *     {
 *       "path": <path of the file>,
 *       "symbols": [
 *         {
 *           "astKind": <AST kind of the symbol>,
 *           "name": <name of the symbol>,
 *           if astKind = package_spec:
 *           "packageName": <name of package>,
 *           "packagePos": {
 *             "line": <integer>,
 *             "column": <integer>
 *           },
 *           "packageNamePos": {
 *             "line": <integer>,
 *             "column": <integer>
 *           },
 *           endif
 *           if astKind = import_spec:
 *           if has from keyword:
 *           "fromPos": {
 *             "line": <integer>,
 *             "column": <integer>
 *           },
 *           "moduleName": <name of module>,
 *           "modulePos": {
 *             "line": <integer>,
 *             "column": <integer>
 *           },
 *           endif
 *           "importPos": {
 *             "line": <integer>,
 *             "column": <integer>
 *           },
 *           "packageName": <name of package>,
 *           "PackageNamePos": {
 *             "line": <integer>,
 *             "column": <integer>
 *           },
 *           "importedItemName": <name of imported item>,
 *           "importedItemNamePos": {
 *             "line": <integer>,
 *             "column": <integer>
 *           },
 *           if has as keyword:
 *           "asPos": {
 *             "line": <integer>,
 *             "column": <integer>
 *           },
 *           "asIdentifier": <name of asIdentifier>,
 *           "asIdentifierPos": {
 *             "line": <integer>,
 *             "column": <integer>
 *           },
 *           endif
 *           endif
 *           if astKind = *decl
 *           "identifier": <name of identifier>,
 *           "identifierPos": {
 *             "line": <integer>,
 *             "column": <integer>
 *           },
 *           endif
 *           "begin": {
 *             "line": <integer>,
 *             "column": <integer>
 *           },
 *           "end": {
 *             "line": <integer>,
 *             "column": <integer>
 *           }
 *         }
 *       ]
 *     }
 *   ]
 * }
 */
void PrintSymbolTable(CompilerInstance& ci);
} // namespace Codira

#endif
