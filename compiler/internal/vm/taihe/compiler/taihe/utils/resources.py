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

"""Tools for tracking resources."""

import logging
import shutil
import subprocess
import tarfile
from abc import ABC, abstractmethod
from argparse import ArgumentParser, Namespace
from collections.abc import Sequence
from dataclasses import dataclass, field
from enum import Enum, auto
from pathlib import Path
from sys import exit
from typing import ClassVar, Final, cast
from urllib.parse import urljoin

from typing_extensions import Self, override


class DeploymentMode(Enum):
    DEV = auto()  # Inside the Git repository (repo_root)
    PKG = auto()  # pip install ... (site-packages/taihe)
    BUNDLE = auto()  # Bundled with a python executable (taihe-pkg)


def fetch_url(url: str, output: Path, curl_extra_args: Sequence[str] | None = None):
    """Downloads a file from a given URL to a specified output path using curl.

    Args:
        url: The URL of the file to download.
        output: The local path where the downloaded file will be saved.
        curl_extra_args: Optional sequence of additional arguments to pass to curl.

    Raises:
        RuntimeError: If the curl command fails.
        FileNotFoundError: If the curl command is not found.
    """
    output.unlink(missing_ok=True)
    output.parent.mkdir(exist_ok=True, parents=True)

    curl_args = [
        "curl",
        "--location",  # Follow redirects
        "--fail",  # Fail on HTTP errors
        "--progress-bar",
        "--output",
        str(output),
        *(curl_extra_args or []),
        url,
    ]
    ok = False
    try:
        logging.info("Downloading %s to %s", url, output)
        subprocess.check_call(curl_args)
        ok = output.exists()
        return output
    except subprocess.CalledProcessError as e:
        logging.error("curl failed")
        raise RuntimeError(f"Failed to download {url}") from e
    except FileNotFoundError as e:
        raise FileNotFoundError("curl command not found.") from e
    finally:
        if ok:
            logging.info("Downloaded %s successfully", output)
        else:
            output.unlink(missing_ok=True)


ResourceT = type["Resource"]


@dataclass
class ResourceContext:
    """Central configuration and state manager for all resources.

    - Determines the current DeploymentMode.
    - Handles command-line arguments to override resource paths.
    - Provides runtime cache for constructed Resource objects, ensuring that
      each resource is located and processed only once.
    """

    _singleton: ClassVar[Self | None] = None

    deployment_mode: DeploymentMode
    base_dir: Path = field(default_factory=Path)
    cache_dir: Path = field(default_factory=Path)

    resolved: dict[ResourceT, "Resource"] = field(init=False)

    def __post_init__(self):
        self.cache_dir = self._determine_cache_dir()
        self.resolved = {}
        self.cache_dir.mkdir(parents=True, exist_ok=True)

    def _determine_cache_dir(self) -> Path:
        match self.deployment_mode:
            case DeploymentMode.DEV:
                return self.base_dir / ".cache"
            case DeploymentMode.BUNDLE | DeploymentMode.PKG:
                return Path("~/.cache/taihe").expanduser()

    def set_path(self, t: ResourceT, p: Path):
        self.resolved[t] = t(p)

    @classmethod
    def from_path(cls, file: str) -> Self:
        # The directory looks like:
        #   7    6     5   4      3            2         1     0
        #                      repo_root/     compiler/taihe/utils/resources.py
        #           .venv/lib/python3.12/site-packages/taihe/utils/resources.py
        # taihe/lib/ pyrt/lib/python3.11/site-packages/taihe/utils/resources.py
        #            ^^^^                ^^^^^^^^^^^^^
        #     python_runtime_dir            repo_dir
        #
        # We use the heuristics based on the name of repository and python runtime.
        DEPTH_REPO = 2
        DEPTH_PYRT = 5
        DEPTH_PKG_ROOT = 7
        parents = Path(file).absolute().parents

        def parent_at(i: int) -> Path:
            if i < len(parents):
                return parents[i]
            return Path()

        repo_dir = parent_at(DEPTH_REPO)
        if repo_dir.name == "compiler":
            return cls(DeploymentMode.DEV, parent_at(DEPTH_REPO + 1))

        if repo_dir.name == "site-packages":
            if parent_at(DEPTH_PYRT).name == PythonBuild.BUNDLE_DIR_NAME:
                return cls(DeploymentMode.BUNDLE, parent_at(DEPTH_PKG_ROOT))
            else:
                return cls(DeploymentMode.PKG, parent_at(DEPTH_REPO - 1) / "data")

        raise RuntimeError(f"cannot determine deployment layout ({repo_dir=})")

    @classmethod
    def initialize(
        cls,
        cli_args: Namespace | None = None,
        resources: Sequence[ResourceT] | None = None,
    ) -> Self:
        if cls._singleton is not None:
            raise ValueError("already constructed")
        cls._singleton = cls.from_path(__file__)
        if cli_args:
            cls._singleton.apply_cli_args(
                ALL_RESOURCES if resources is None else resources,
                cli_args,
            )
        return cls._singleton

    @classmethod
    def instance(cls) -> Self:
        if cls._singleton is None:
            raise ValueError("must be initialized before")
        return cls._singleton

    @staticmethod
    def register_cli_options(
        parser: ArgumentParser,
        resources: Sequence[ResourceT] | None = None,
        use_print: bool = True,
        use_override: bool = True,
    ) -> None:
        """Register --print and --override CLI arguments to the parser."""
        resources = ALL_RESOURCES if resources is None else resources

        if use_print:
            # Add --print-paths argument
            parser.add_argument(
                "--print-paths",
                action="store_true",
                help="Print all resource paths and exit",
            )

            # Dynamically add --print-<resourcetype>-path arguments
            for r in resources:
                parser.add_argument(
                    f"--print-{r.CLI_NAME}-path",
                    action="store_true",
                    help=f"Print {r.CLI_NAME} path and exit",
                )

        if use_override:
            # Dynamically add --override-<resourcetype>-path arguments
            for r in resources:
                parser.add_argument(
                    f"--override-{r.CLI_NAME}-path",
                    type=str,
                    metavar="PATH",
                    help=f"Override {r.CLI_NAME} path",
                )

    def apply_cli_args(
        self,
        resources: Sequence[ResourceT],
        args: Namespace,
        auto_exit: bool = True,
    ) -> bool:
        """Process CLI arguments. Returns True if program should exit."""

        def key_of(res: ResourceT, prefix: str):
            return f"{prefix}_{res.CLI_NAME.replace('-', '_')}_path"

        # Apply overrides
        for r in resources:
            if path := getattr(args, key_of(r, "override"), None):
                self.set_path(r, Path(path))

        should_exit = False
        # Handle print requests
        if args.print_paths:
            print(f"<mode>: {self.deployment_mode.name}")
            print(f"<base>: {self.base_dir}")
            for r in resources:
                if issubclass(r, CachedResource):
                    prefix = "<pending> "
                    value = r.locate(self)
                else:
                    prefix = ""
                    value = r.resolve_path(self)
                print(f"{prefix}{r.CLI_NAME}: {value}")
            should_exit = True

        # Check individual print requests
        for r in resources:
            if getattr(args, key_of(r, "print"), False):
                print(r.resolve_path(self))
                should_exit = True

        if should_exit and auto_exit:
            exit(0)

        return should_exit


@dataclass
class Resource(ABC):
    """Abstract base class representing an external dependency or asset."""

    CLI_NAME: ClassVar[str]
    base_path: Path

    @classmethod
    @abstractmethod
    def construct(cls, ctx: ResourceContext) -> Self:
        """Returns a instance by auto detection."""

    @classmethod
    def resolve(cls, ctx: ResourceContext | None = None) -> Self:
        """Resolves the resource from cache, creating one if not exists."""
        ctx = ctx or ResourceContext.instance()
        if (res := ctx.resolved.get(cls, None)) is None:
            res = cls.construct(ctx)
            ctx.resolved[cls] = res
        return cast(Self, res)

    @classmethod
    def resolve_path(cls, ctx: ResourceContext | None = None) -> Path:
        """Resolves the path from cache, creating an instance if not exists."""
        return cls.resolve(ctx).base_path


@dataclass
class PathResource(Resource):
    """Resources bundled with the application distribution.

    The path to the resource is determined by the active `DeploymentMode` and
    is constructed by joining the `ResourceContext.base_dir` with one of the
    class-level path variables (`PATH_DEV`, `PATH_PKG`, `PATH_BUNDLE`). This
    allows the application to locate bundled assets correctly, regardless of
    whether it is running from a source checkout, an installed package, or a
    self-contained bundle.

    Attributes:
        PATH_PKG: The relative path to the resource in `PKG` mode.
        PATH_DEV: The relative path to the resource in `DEV` mode.
        PATH_BUNDLE: The relative path to the resource in `BUNDLE` mode.
    """

    PATH_PKG: ClassVar[str]
    PATH_DEV: ClassVar[str]
    PATH_BUNDLE: ClassVar[str]

    @override
    @classmethod
    def construct(cls, ctx: ResourceContext) -> Self:
        match ctx.deployment_mode:
            case DeploymentMode.DEV:
                return cls(ctx.base_dir / cls.PATH_DEV)
            case DeploymentMode.PKG:
                return cls(ctx.base_dir / cls.PATH_PKG)
            case DeploymentMode.BUNDLE:
                return cls(ctx.base_dir / cls.PATH_BUNDLE)


class CachedResource(Resource, ABC):
    """Resources that must be fetched from an external source and cached locally.

    This class is for dependencies that are not included in the application
    package, such as large binaries, toolchains, or source repositories.
    Normally, the resource is stored in a subdirectory of the
    `ResourceContext.cache_dir`, determined by the `PATH_CACHE` class variable.
    The `construct` method ensures the resource is available by checking
    `exists()` and calling `fetch()` if it is missing or outdated.

    Attributes:
        PATH_CACHE: The relative path within the cache directory where the
                    resource should be stored.
    """

    PATH_CACHE: ClassVar[str]

    @classmethod
    def locate(cls, ctx: ResourceContext) -> Path:
        """Returns the path of the resource."""
        return ctx.cache_dir / cls.PATH_CACHE

    def exists(self) -> bool:
        """Validates that the resource exists."""
        return self.base_path.exists()

    @abstractmethod
    def fetch(self):
        """Acquires the resource (e.g., downloading a file, cloning a git repository)."""

    @override
    @classmethod
    def construct(cls, ctx: ResourceContext) -> Self:
        self = cls(cls.locate(ctx))
        if not self.exists():
            self.fetch()
        return self


class RuntimeSource(PathResource):
    CLI_NAME = "runtime-source"
    PATH_PKG = PATH_DEV = "runtime/src"
    PATH_BUNDLE = "src/taihe/runtime"


class RuntimeHeader(PathResource):
    CLI_NAME = "runtime-header"
    PATH_PKG = PATH_DEV = "runtime/include"
    PATH_BUNDLE = "include"


class StandardLibrary(PathResource):
    CLI_NAME = "stdlib"
    PATH_PKG = PATH_DEV = "stdlib"
    PATH_BUNDLE = "lib/taihe/stdlib"


class CMakeModulesResource(PathResource):
    CLI_NAME = "cmake"
    PATH_PKG = PATH_DEV = "cmake"
    PATH_BUNDLE = "lib/cmake/taihe"


class _LegacyPandaVm(PathResource):
    PATH_DEV = ".panda_vm"
    PATH_PKG = "<no-panda-vm-in-pkg-mode>"
    PATH_BUNDLE = "var/lib/panda_vm"


@dataclass
class PandaVm(CachedResource):
    CLI_NAME = "panda-vm"
    PATH_CACHE = "panda-vm"
    VERSION: Final = "sdk-1.5.0-dev.54451"
    URL: Final = "https://gitcode.com/m0_52007851/panda_vm/releases/download/54451"

    # Computed attributes
    ani_header_dir: Path = field(init=False)
    stdlib_sources: dict[str, Path] = field(init=False)
    sdk_sources: dict[str, Path] = field(init=False)
    stdlib_lib: Path = field(init=False)
    sdk_lib: Path = field(init=False)
    host_tools_dir: Path = field(init=False)

    def __post_init__(self):
        self.ani_header_dir = (
            self.base_path / "ohos_arm64/include/plugins/ets/runtime/ani"
        )
        self.stdlib_sources = {
            "std": self.base_path / "ets/stdlib/std",
            "escompat": self.base_path / "ets/stdlib/escompat",
        }
        self.sdk_sources = {
            "@ohos": self.base_path / "ets/sdk/sdk/api/@ohos",
        }
        self.stdlib_lib = self.base_path / "ets" / "etsstdlib.abc"
        self.sdk_lib = self.base_path / "ets" / "etssdk.abc"
        self.host_tools_dir = self.base_path / "linux_host_tools"

    def tool(self, binary: str) -> Path:
        return self.host_tools_dir / "bin" / binary

    def _read_version(self) -> str:
        try:
            version_file = self.base_path / "version.txt"
            return version_file.read_text().strip()
        except (FileNotFoundError, OSError):
            return ""

    def _write_version(self, version: str):
        version_file = self.base_path / "version.txt"
        version_file.write_text(version)

    @override
    @classmethod
    def locate(cls, ctx: ResourceContext) -> Path:
        # Migrate from legacy location if needed
        base_dir = super().locate(ctx)
        old = _LegacyPandaVm.construct(ctx).base_path
        if old.exists():
            new = base_dir
            logging.info("Migrating %s -> %s", old, new)
            try:
                new.rmdir()  # Already created by `get_cache_dir`
                old.rename(new)
            except OSError:  # new has contents
                logging.warning("Cannot migrate: the new directory already exists")
                logging.warning("Purging the old directory to avoid futher migration")
                shutil.rmtree(old, ignore_errors=True)

        return base_dir / "package"

    @override
    def exists(self) -> bool:
        return self._read_version() == self.VERSION

    @override
    def fetch(self):
        tgz = self.base_path.parent / f"{self.VERSION}.tgz"
        url = f"{self.URL}/{self.VERSION}.tgz"
        if not tgz.exists():
            fetch_url(url, tgz)

        shutil.rmtree(self.base_path, ignore_errors=True)
        with tarfile.open(tgz, "r:gz") as tar:
            tar.extractall(self.base_path.parent, filter="tar")

        self._write_version(self.VERSION)


class PythonBuild(CachedResource):
    CLI_NAME = "python-packages"
    PATH_CACHE = "python-packages"

    # Constants for downloading prebuilt Python runtime bundles

    # HarmonyOS repository url for Python bundles
    REPO: Final = "https://repo.huaweicloud.com/harmonyos/compiler/python/3.11.4/"
    LINUX_REPO: Final = urljoin(REPO, "linux/")
    WINDOWS_REPO: Final = urljoin(REPO, "windows/")
    DARWIN_REPO: Final = urljoin(REPO, "darwin/")

    BUNDLE_DIR_NAME: Final = "pyrt"

    # Supported platforms
    LINUX_X86_64: Final = "linux-x86_64"
    WINDOWS_X86_64: Final = "windows-x86_64"
    DARWIN_ARM64: Final = "darwin-arm64"
    DARWIN_X86_64: Final = "darwin-x86_64"

    # Bundle tarball file names for each platform
    LINUX_X86_64_PY_BUNDLE: Final = "python-linux-x86-GLIBC2.27-3.11.4_20251107.tar.gz"
    WINDOWS_X86_64_PY_BUNDLE: Final = "python-mingw-x86-3.11.4_20251107.tar.gz"
    DARWIN_ARM64_PY_BUNDLE: Final = "python-darwin-arm64-3.11.4_20251107.tar.gz"
    DARWIN_X86_64_PY_BUNDLE: Final = "python-darwin-x86-3.11.4_20251107.tar.gz"

    @override
    def fetch(self):
        # Clear and rebuild the cache directory
        shutil.rmtree(self.base_path, ignore_errors=True)
        self.base_path.mkdir(parents=True, exist_ok=True)

        # System -> (repository url, file name)
        downloads = {
            self.LINUX_X86_64: (self.LINUX_REPO, self.LINUX_X86_64_PY_BUNDLE),
            self.WINDOWS_X86_64: (self.WINDOWS_REPO, self.WINDOWS_X86_64_PY_BUNDLE),
            self.DARWIN_ARM64: (self.DARWIN_REPO, self.DARWIN_ARM64_PY_BUNDLE),
            self.DARWIN_X86_64: (self.DARWIN_REPO, self.DARWIN_X86_64_PY_BUNDLE),
        }

        # Download all platform bundles using curl, save as <system>-python.tar.gz
        for system, (repo, filename) in downloads.items():
            url = urljoin(repo, filename)
            out_path = self.base_path / f"{system}-python.tar.gz"
            logging.info("Downloading Python packages from %s to %s", url, out_path)
            subprocess.run(["curl", "-fLsS", "-o", str(out_path), url], check=True)

        logging.info("All Python runtime bundles downloaded to %s", self.base_path)

    def extract_to(self, target_dir: Path, *, system: str):
        tgz = self.base_path / f"{system}-python.tar.gz"

        # Extract the tgz and get the directory list
        parent_dir = target_dir.parent
        with tarfile.open(tgz, "r:gz") as tar:
            tgz_dir_lists = tar.getnames()
            tar.extractall(parent_dir, filter="tar")

        # Find all top-level directories in the tgz
        tgz_top_dirs = {Path(n).parts[0] for n in tgz_dir_lists if Path(n).parts}

        # Find real root directory (including bin/lib/include)
        root = parent_dir
        while (
            root.is_dir()
            and not (root / "bin").exists()
            and len(list(root.iterdir())) == 1
        ):
            root = next(root.iterdir())

        # Next, rename root directory to "parent_dir/pyrt" (i.e. target_dir)
        if target_dir.exists():
            shutil.rmtree(target_dir, ignore_errors=True)
        root.rename(target_dir)

        # Remove unneeded directories extracted from the tgz
        for dir in tgz_top_dirs:
            dir_path = parent_dir / dir
            if dir_path != target_dir and dir_path.exists():
                shutil.rmtree(dir_path, ignore_errors=True)

    @override
    @classmethod
    def construct(cls, ctx: ResourceContext) -> Self:
        self = cls(cls.locate(ctx))
        self.fetch()
        return self


class Antlr(CachedResource):
    CLI_NAME = "antlr"

    VERSION: Final = "4.11.1"
    MAVEN_REMOTE: Final = "https://mirrors.huaweicloud.com/repository/maven"
    MAVEN_LOCAL: Final = "~/.m2/repository"
    MAVEN_PATH: Final = f"org/antlr/antlr4/{VERSION}/antlr4-{VERSION}-complete.jar"

    @override
    @classmethod
    def locate(cls, ctx: ResourceContext) -> Path:
        return Path(f"{cls.MAVEN_LOCAL}/{cls.MAVEN_PATH}").expanduser()

    @override
    def fetch(self):
        fetch_url(f"{self.MAVEN_REMOTE}/{self.MAVEN_PATH}", self.base_path)

    def run_tool(self, args: list[str]):
        subprocess.check_call(
            ["java", "-cp", str(self.base_path), "org.antlr.v4.Tool", *args], env={}
        )


BUILTIN_RESOURCES: Sequence[ResourceT] = [
    RuntimeSource,
    RuntimeHeader,
    StandardLibrary,
    CMakeModulesResource,
]
ALL_RESOURCES: Sequence[ResourceT] = [
    *BUILTIN_RESOURCES,
    PandaVm,
    PythonBuild,
    Antlr,
]