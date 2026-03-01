# coding=utf-8
#
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

from dataclasses import dataclass
from typing import TYPE_CHECKING, ClassVar

from taihe.driver.backend import Backend, BackendConfig

if TYPE_CHECKING:
    from taihe.driver.contexts import CompilerInstance


@dataclass
class CppCommonHeadersBackendConfig(BackendConfig):
    NAME = "cpp-common"
    DEPS: ClassVar = ["abi-header"]

    def construct(self, instance: "CompilerInstance") -> Backend:
        from taihe.codegen.cpp.gen_common import CppHeadersGenerator

        class CppCommonHeadersBackendImpl(Backend):
            def __init__(self, ci: "CompilerInstance"):
                super().__init__(ci)
                self._ci = ci

            def generate(self):
                om = self._ci.output_manager
                am = self._ci.analysis_manager
                pg = self._ci.package_group
                CppHeadersGenerator(om, am).generate(pg)

        return CppCommonHeadersBackendImpl(instance)


@dataclass
class CppAuthorBackendConfig(BackendConfig):
    NAME = "cpp-author"
    DEPS: ClassVar = ["cpp-common", "abi-source"]

    def construct(self, instance: "CompilerInstance") -> Backend:
        from taihe.codegen.cpp.gen_impl import (
            CppImplHeadersGenerator,
            CppImplSourcesGenerator,
        )

        # TODO: unify CppImpl{Headers,Sources}Generator
        class CppImplBackendImpl(Backend):
            def __init__(self, ci: "CompilerInstance"):
                super().__init__(ci)
                self._ci = ci

            def generate(self):
                om = self._ci.output_manager
                am = self._ci.analysis_manager
                pg = self._ci.package_group
                CppImplSourcesGenerator(om, am).generate(pg)
                CppImplHeadersGenerator(om, am).generate(pg)

        return CppImplBackendImpl(instance)


@dataclass
class CppUserHeadersBackendConfig(BackendConfig):
    NAME = "cpp-user"
    DEPS: ClassVar = ["cpp-common"]

    def construct(self, instance: "CompilerInstance") -> Backend:
        from taihe.codegen.cpp.gen_user import CppUserHeadersGenerator

        class CppUserHeadersBackendImpl(Backend):
            def __init__(self, ci: "CompilerInstance"):
                super().__init__(ci)
                self._ci = ci

            def generate(self):
                om = self._ci.output_manager
                am = self._ci.analysis_manager
                pg = self._ci.package_group
                CppUserHeadersGenerator(om, am).generate(pg)

        return CppUserHeadersBackendImpl(instance)