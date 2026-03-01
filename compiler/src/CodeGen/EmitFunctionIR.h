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

#ifndef CODIRA_EMITFUNCTIONIR_H
#define CODIRA_EMITFUNCTIONIR_H

#include <vector>

namespace Codira {
namespace CHIR {
class Func;
class ImportedFunc;
} // namespace CHIR

namespace CodeGen {
class CGModule;
void EmitFunctionIR(CGModule& cgMod, const std::vector<CHIR::Func*>& chirFuncs);
void EmitImportedCFuncIR(CGModule& cgMod, const std::vector<CHIR::ImportedFunc*>& importedCFuncs);
} // namespace CodeGen
} // namespace Codira

#endif // CODIRA_EMITFUNCTIONIR_H
