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
from typing import ClassVar

from taihe.driver.backend import Backend, BackendConfig
from taihe.driver.contexts import CompilerInstance
from taihe.utils.resources import StandardLibrary
from taihe.utils.sources import SourceFile


@dataclass
class AniBridgeBackendConfig(BackendConfig):
    NAME = "ani-bridge"
    DEPS: ClassVar = ["cpp-user"]

    keep_name: bool = False
    """Use the original function name (instead of "camelCase") in exported ArkTS sources."""

    def construct(self, instance: CompilerInstance) -> Backend:
        from taihe.codegen.ani.attributes import all_attr_types
        from taihe.codegen.ani.gen_ani import AniCodeGenerator
        from taihe.codegen.ani.gen_sts import StsCodeGenerator

        # TODO: unify {Ani,Sts}CodeGenerator
        class AniBridgeBackendImpl(Backend):
            def __init__(self, ci: "CompilerInstance"):
                super().__init__(ci)
                self._ci = ci
                self._ci.attribute_registry.register(*all_attr_types)

            def inject(self):
                self._ci.source_manager.add_source(
                    SourceFile(
                        StandardLibrary.resolve_path() / "taihe.platform.ani.taihe",
                        is_stdlib=True,
                    )
                )

            def generate(self):
                om = self._ci.output_manager
                am = self._ci.analysis_manager
                pg = self._ci.package_group
                AniCodeGenerator(om, am).generate(pg)
                StsCodeGenerator(om, am).generate(pg)

        return AniBridgeBackendImpl(instance)