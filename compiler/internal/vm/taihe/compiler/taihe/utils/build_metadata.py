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

import sys
from dataclasses import dataclass
from functools import cache
from typing import TextIO


@dataclass(frozen=True)
class BuildMetadata:
    version: str
    git_commit: str
    git_message: str
    build_date: str
    build_time_utc: float
    is_unknown: bool

    @staticmethod
    def _unknown_metadata() -> "BuildMetadata":
        return BuildMetadata(
            version="0.0.0+unknown",
            git_commit="<unknown commit>",
            git_message="<unknown message>",
            build_date="<unknown date>",
            build_time_utc=-1.0,
            is_unknown=True,
        )

    @classmethod
    @cache
    def get(cls) -> "BuildMetadata":
        try:
            import taihe._version as ver

            return cls(
                version=ver.version,
                git_commit=ver.git_commit,
                git_message=ver.git_message,
                build_date=ver.build_date,
                build_time_utc=ver.build_time_utc,
                is_unknown=False,
            )
        except ModuleNotFoundError:
            return cls._unknown_metadata()

    def print_info(
        self,
        tool: str = "Taihe compiler (taihec)",
        auto_exit: bool = True,
        output: TextIO = sys.stderr,
    ) -> None:
        print(f"{tool} {self.version} {self.build_date}", file=output)
        print(f"Commit: {self.git_commit}", file=output)
        print(f"Message: {self.git_message}", file=output)

        if auto_exit:
            sys.exit(0)


if __name__ == "__main__":
    BuildMetadata.get().print_info()