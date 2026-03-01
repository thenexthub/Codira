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
class AbiHeaderBackendConfig(BackendConfig):
    NAME = "abi-header"

    def construct(self, instance: "CompilerInstance") -> Backend:
        from taihe.codegen.abi.gen_abi import AbiHeadersGenerator

        class AbiHeaderBackendImpl(Backend):
            def __init__(self, ci: "CompilerInstance"):
                super().__init__(ci)
                self._ci = ci

            def generate(self):
                om = self._ci.output_manager
                am = self._ci.analysis_manager
                pg = self._ci.package_group
                AbiHeadersGenerator(om, am).generate(pg)

        return AbiHeaderBackendImpl(instance)


@dataclass
class AbiSourcesBackendConfig(BackendConfig):
    NAME = "abi-source"
    DEPS: ClassVar = ["abi-header"]

    def construct(self, instance: "CompilerInstance") -> Backend:
        from taihe.codegen.abi.gen_abi import AbiSourcesGenerator

        class AbiSourcesBackendImpl(Backend):
            def __init__(self, ci: "CompilerInstance"):
                super().__init__(ci)
                self._ci = ci

            def generate(self):
                om = self._ci.output_manager
                am = self._ci.analysis_manager
                pg = self._ci.package_group
                AbiSourcesGenerator(om, am).generate(pg)

        return AbiSourcesBackendImpl(instance)


@dataclass
class CAuthorBackendConfig(BackendConfig):
    NAME = "c-author"
    DEPS: ClassVar = ["abi-source"]

    def construct(self, instance: "CompilerInstance") -> Backend:
        from taihe.codegen.abi.gen_impl import (
            CImplHeadersGenerator,
            CImplSourcesGenerator,
        )

        # TODO: unify CImpl{Headers,Sources}Generator
        class CImplBackendImpl(Backend):
            def __init__(self, ci: "CompilerInstance"):
                super().__init__(ci)
                self._ci = ci

            def generate(self):
                om = self._ci.output_manager
                am = self._ci.analysis_manager
                pg = self._ci.package_group
                CImplSourcesGenerator(om, am).generate(pg)
                CImplHeadersGenerator(om, am).generate(pg)

        return CImplBackendImpl(instance)