//===- XCOFFObjcopy.cpp ---------------------------------------------------===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
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

#include "vm/core/ObjCopy/XCOFF/XCOFFObjcopy.h"
#include "XCOFFObject.h"
#include "XCOFFReader.h"
#include "XCOFFWriter.h"
#include "vm/core/ObjCopy/CommonConfig.h"
#include "vm/core/ObjCopy/XCOFF/XCOFFConfig.h"

namespace vm::core {
namespace objcopy {
namespace xcoff {

using namespace object;

static Error handleArgs(const CommonConfig &Config, Object &Obj) {
  return Error::success();
}

Error executeObjcopyOnBinary(const CommonConfig &Config, const XCOFFConfig &,
                             XCOFFObjectFile &In, raw_ostream &Out) {
  XCOFFReader Reader(In);
  Expected<std::unique_ptr<Object>> ObjOrErr = Reader.create();
  if (!ObjOrErr)
    return createFileError(Config.InputFilename, ObjOrErr.takeError());
  Object *Obj = ObjOrErr->get();
  assert(Obj && "Unable to deserialize XCOFF object");
  if (Error E = handleArgs(Config, *Obj))
    return createFileError(Config.InputFilename, std::move(E));
  XCOFFWriter Writer(*Obj, Out);
  if (Error E = Writer.write())
    return createFileError(Config.OutputFilename, std::move(E));
  return Error::success();
}

} // end namespace xcoff
} // end namespace objcopy
} // end namespace vm::core
