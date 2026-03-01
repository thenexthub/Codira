#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2025 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the 'License');
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an 'AS IS' BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import subprocess
from abc import ABC, abstractmethod
from typing import TextIO

from runner.logger import Log

_LOGGER = Log.get_logger(__file__)


class CmdExecutor(ABC):
    @abstractmethod
    def run_command(
        self,
        command: list[str],
        stdout: TextIO | int = subprocess.PIPE,
        stderror: TextIO | int = subprocess.PIPE
    ) -> subprocess.CompletedProcess | None:
        """
        Execute a shell command in a subprocess.

        Args:
            command (list[str]): Command and its arguments as a list of strings.
            stdout (TextIO | int): Standard output stream. Defaults to subprocess.PIPE.
            stderror (TextIO | int): Standard error stream. Defaults to subprocess.PIPE.

        Returns:
            CompletedProcess | None: Output of the command as a string or exit code.

        Raises:
            NotImplementedError: If the method is not implemented in a subclass.
        """
        raise NotImplementedError()

    @abstractmethod
    def get_binary(self, binary_name: str, version: str | None = None) -> str:
        """
        Get the binary name with optional version suffix.

        Args:
            binary_name (str): Base name of the binary.
            version (str | None): Optional version of the binary. Defaults to None.

        Returns:
            str: Binary name with version if specified.

        Raises:
            NotImplementedError: If the method is not implemented in a subclass.
        """
        raise NotImplementedError()


class LinuxCmdExecutor(CmdExecutor):
    def run_command(
        self,
        command: list[str],
        stdout: TextIO | int = subprocess.PIPE,
        stderror: TextIO | int = subprocess.PIPE
    ) -> subprocess.CompletedProcess | None:
        """
        Run a command on Linux using subprocess.run().

        Args:
            command (list[str]): Command and arguments to execute.
            stdout (TextIO | int): Standard output destination. Defaults to subprocess.PIPE.
            stderror (TextIO | int): Standard error stream. Defaults to subprocess.PIPE.

        Returns:
            CompletedProcess | None: Command output as a string or return code.

        Raises:
            subprocess.CalledProcessError: If the command returns a non-zero exit status.
            subprocess.TimeoutExpired: If the command times out.
        """
        try:
            return subprocess.run(command, check=True, text=True, stdout=stdout, stderr=stderror, timeout=5400)
        except subprocess.CalledProcessError as error:
            _LOGGER.default(f"There was a problem:\n{error.stderr}")
            return None

    def get_binary(self, binary_name: str, version: str | None = None) -> str:
        """
        Construct binary path with optional version.

        Args:
            binary_name (str): Base name of the binary.
            version (str | None): Optional version string. Defaults to None.

        Returns:
            str: Binary name with version suffix if provided.
        """
        return f"{binary_name}-{version}" if version else binary_name
