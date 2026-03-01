#===----------------------------------------------------------------------===#
#   Copyright (c) NeXTHub Corporation. All Rights Reserved.
#   DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
#   Author: Tunjay Akbarli
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at:
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#
#   Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
#   Middletown, DE 19709, New Castle County, USA.
#===----------------------------------------------------------------------===#

set(BASIC_SRC
    StringConvertor.cpp
    DiagnosticEngine.cpp
    DiagnosticEngineImpl.cpp
    Position.cpp
    SourceManager.cpp
    Display.cpp
    DiagnosticEmitter.cpp
    DiagnosticJsonFormatter.cpp
    Utils.cpp
    Print.cpp)

set(BASIC_SRC ${BASIC_SRC} Version.cpp)

add_library(CodiraBasic OBJECT ${BASIC_SRC})
target_compile_options(CodiraBasic PRIVATE -Wno-deprecated-declarations)
target_compile_options(CodiraBasic PRIVATE ${CODEC_EXTRA_WARNINGS})
