//==--- DiagnosticCrossTUKinds.td - Cross Translation Unit diagnostics ----===//
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

let Component = "CrossTU" in {

def err_ctu_error_opening : Error<
  "error opening '%0': required by the CrossTU functionality">;

def err_extdefmap_parsing : Error<
  "error parsing index file: '%0' line: %1 '<USR-Length>:<USR> <File-Path>' "
  "format expected">;

def err_multiple_def_index : Error<
  "multiple definitions are found for the same key in index ">;

def warn_ctu_incompat_triple : Warning<
  "imported AST from '%0' had been generated for a different target, "
  "current: %1, imported: %2">, InGroup<CrossTU>;
}
