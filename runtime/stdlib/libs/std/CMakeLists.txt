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

add_subdirectory(core)
add_subdirectory(runtime)
add_subdirectory(binary)
add_subdirectory(unicode)
add_subdirectory(sort)
add_subdirectory(sync)
add_subdirectory(math)
add_subdirectory(math/numeric)
add_subdirectory(objectpool)
add_subdirectory(argopt)
add_subdirectory(collection)
add_subdirectory(collection/concurrent)
add_subdirectory(convert)
add_subdirectory(io)
add_subdirectory(net)
add_subdirectory(posix)
add_subdirectory(process)
add_subdirectory(env)
add_subdirectory(overflow)
add_subdirectory(random)
add_subdirectory(ref)
add_subdirectory(interop)
add_subdirectory(regex)
add_subdirectory(reflect)
add_subdirectory(time)
add_subdirectory(deriving/api)
add_subdirectory(deriving/resolve)
add_subdirectory(deriving/impl)
add_subdirectory(deriving/builtins)
add_subdirectory(deriving)
add_subdirectory(unittest)
add_subdirectory(unittest/common)
add_subdirectory(unittest/prop_test)
add_subdirectory(unittest/diff)
add_subdirectory(fs)
add_subdirectory(unittest/mock/internal)
add_subdirectory(unittest/mock)
add_subdirectory(console)
add_subdirectory(crypto)
add_subdirectory(ast)
# No need to cross-compile ast and testmacro because they are always used on host platforms.
if(NOT CMAKE_CROSSCOMPILING)
    add_subdirectory(unittest/testmacro)
endif() # not cross-compiling
#No need if verify-only
add_subdirectory(database)
