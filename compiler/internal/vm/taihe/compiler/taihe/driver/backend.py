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

from abc import ABC, abstractmethod
from collections.abc import Iterable
from typing import TYPE_CHECKING, ClassVar

if TYPE_CHECKING:
    from taihe.driver.contexts import CompilerInstance


class Backend(ABC):
    @abstractmethod
    def __init__(self, instance: "CompilerInstance"):
        """Initialize the backend."""

    def inject(self):
        """Add backend-specific sources after the collection phase.

        For example, the standard library sources can be added in this stage.
        """
        return

    def post_process(self):
        """Post-processes the IR just after parsing.

        Language backend may transform the IR in-place in this stage.
        """
        return

    def validate(self):
        """Validate the IR after the post-process stage.

        Language backend MUST NOT transform the IR in this stage.
        """
        return

    def generate(self):
        """Generate the output files.

        Language backend MUST NOT transform the IR or report error in this stage:
        - The transformation should be completed in the `post_process` stage.
        - The error reporting should be completed in the `validate` stage.
        """
        return


class BackendConfig(ABC):
    NAME: ClassVar[str]
    "The name of the backend."

    DEPS: ClassVar[list[str]] = []
    "List of backends that the current backend depends on."

    @abstractmethod
    def __init__(self):
        ...

    @abstractmethod
    def construct(self, instance: "CompilerInstance") -> Backend:
        ...


BackendConfigT = type[BackendConfig]


class BackendRegistry:
    def __init__(self):
        self._factories: dict[str, BackendConfigT] = {}

    def register(self, factory: BackendConfigT):
        name = factory.NAME
        if (setted := self._factories.setdefault(name, factory)) is not factory:
            raise ValueError(
                f"backend {name!r} cannot be registered as {factory.__name__} "
                f"because it is already registered as {setted.__name__}"
            )

    def get_backend_names(self) -> list[str]:
        return list(self._factories.keys())

    def clear(self):
        self._factories.clear()

    def collect_required_backends(self, names: Iterable[str]) -> list[BackendConfigT]:
        factories: list[BackendConfigT] = []
        visited: set[str] = set()

        def add(name: str):
            if name in visited:
                return False
            factory = self._factories.get(name)
            if not factory:
                raise KeyError(f"unknown backend {name!r}")

            visited.add(name)
            for dep in factory.DEPS:
                add(dep)
            factories.append(factory)
            return True

        for name in names:
            add(name)

        return factories

    def register_all(self):
        from taihe.codegen.abi import (
            AbiHeaderBackendConfig,
            AbiSourcesBackendConfig,
            CAuthorBackendConfig,
        )
        from taihe.codegen.ani import AniBridgeBackendConfig
        from taihe.codegen.cpp import (
            CppAuthorBackendConfig,
            CppCommonHeadersBackendConfig,
            CppUserHeadersBackendConfig,
        )
        from taihe.semantics import PrettyPrintBackendConfig

        backends = [
            # abi
            AbiHeaderBackendConfig,
            AbiSourcesBackendConfig,
            CAuthorBackendConfig,
            # cpp
            CppCommonHeadersBackendConfig,
            CppAuthorBackendConfig,
            CppUserHeadersBackendConfig,
            # ani
            AniBridgeBackendConfig,
            # pretty print
            PrettyPrintBackendConfig,
        ]

        for b in backends:
            self.register(b)