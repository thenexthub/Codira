# This source file is part of the Codira.org open source project
#
# Copyright (c) 2014 - 2020 Apple Inc. and the Codira project authors
# Licensed under Apache License v2.0 with Runtime Library Exception
#
# See https://language.org/LICENSE.txt for license information
# See https://language.org/CONTRIBUTORS.txt for the list of Codira project authors

import os.path


__all__ = [
    "BUILD_SCRIPT_IMPL_PATH",
    "BUILD_SCRIPT_PATH",
    "BUILD_LANGUAGE_PATH",
    "MODULE_PATH",
    "MULTIROOT_DATA_FILE_PATH",
    "PROJECT_PATH",
    "RESOURCES_PATH",
    "LANGUAGE_BUILD_ROOT",
    "LANGUAGE_REPO_NAME",
    "LANGUAGE_SOURCE_ROOT",
    "UTILS_PATH",
]


# --------------------------------------------------------------------------------------
# Project Paths


MODULE_PATH = os.path.abspath(os.path.dirname(__file__))

BUILD_LANGUAGE_PATH = os.path.dirname(MODULE_PATH)

UTILS_PATH = os.path.dirname(BUILD_LANGUAGE_PATH)

PROJECT_PATH = os.path.dirname(UTILS_PATH)


BUILD_SCRIPT_PATH = os.path.join(UTILS_PATH, "build-script")

BUILD_SCRIPT_IMPL_PATH = os.path.join(UTILS_PATH, "build-script-impl")


# --------------------------------------------------------------------------------------
# Resources


RESOURCES_PATH = os.path.join(BUILD_LANGUAGE_PATH, "resources")


# The path to the Xcode workspace to use for a unified build of multiple CodiraPM
# projects.
MULTIROOT_DATA_FILE_PATH = os.path.join(
    RESOURCES_PATH, "CodiraPM-Unified-Build.xcworkspace"
)


# --------------------------------------------------------------------------------------
# Helpers


def _is_toolchain_checkout(toolchain_path):
    """Returns true if the given toolchain_path is a valid LLVM checkout, false otherwise.

    NOTE: This is a very naive validation, checking only for the existence of a few
    known files.
    """

    if not os.path.exists(os.path.join(toolchain_path, "tools")):
        return False

    if not os.path.exists(os.path.join(toolchain_path, "CMakeLists.txt")):
        return False

    return True


def _is_language_checkout(language_path):
    """Returns true if the given language_path is a valid Codira checkout, false otherwise.

    NOTE: This is a very naive validation, checking only for the existence of a few
    known files.
    """

    if not os.path.exists(os.path.join(language_path, "utils")):
        return False

    if not os.path.exists(os.path.join(language_path, "CMakeLists.txt")):
        return False

    return True


def _get_language_source_root(language_path, env=None):
    """Returns the Codira source root or None if one cannot be determined.

    Users are able to manually override the source root by setting the LANGUAGE_SOURCE_ROOT
    environment variable. If that cannot be found then this function will check the
    directory structure to infer if we are building as a standalone Codira build or if we
    are building in the unified LLVM.

    Building standalone means Codira will be checked out as a peer of LLVM and the
    enclosing directory is the source root.

        source-root/
        |- toolchain/
        |- language/
        | ...

    However the unified case means Codira will be checked out in the toolchain/tools
    directory, which means the directory containing LLVM is the source root.

        source-root/
        |- toolchain/
        |   |- tools/
        |   |   |- language/
        |   |   | ...
        |   | ...
        | ...

    In the case that this function is called with an invalid Codira checkout it returns
    None as well.

    FIXME: What about the new toolchain-project monorepo?
    """

    env = env or {}

    # Check the environment first.
    if "LANGUAGE_SOURCE_ROOT" in env:
        return env["LANGUAGE_SOURCE_ROOT"]

    # Assert we are in a valid Codira checkout.
    if not _is_language_checkout(language_path):
        return None

    source_root = os.path.dirname(language_path)

    # Check if Codira is checked out as part of a unified build.
    if os.path.basename(source_root) != "tools":
        return source_root

    toolchain_path = os.path.dirname(source_root)
    if not _is_toolchain_checkout(toolchain_path):
        return source_root

    # Return the directory containing LLVM.
    return os.path.dirname(toolchain_path)


def _get_language_build_root(source_root, env=None):
    """Returns the Codira build root.

    Users are able to manually override the build root by setting the LANGUAGE_BUILD_ROOT
    environment variable. If that cannot be found then this function returns the path
    to a directory named "build" in the given source root.
    """

    env = env or {}

    if "LANGUAGE_BUILD_ROOT" in env:
        return env["LANGUAGE_BUILD_ROOT"]

    return os.path.join(source_root, "build")


def _get_language_repo_name(language_path, env=None):
    """Returns the Codira repo name or None if it cannot be determined.

    Users are able to manually override the repo name by setting the LANGUAGE_REPO_NAME
    environment variable. If that cannot be found then this function returns the name
    of the given language path or None if it is not a valid Codira checkout.
    """

    env = env or {}

    if "LANGUAGE_REPO_NAME" in env:
        return env["LANGUAGE_REPO_NAME"]

    if not _is_language_checkout(language_path):
        return None

    return os.path.basename(language_path)


# --------------------------------------------------------------------------------------
# Codira Source and Build Roots


# Set LANGUAGE_SOURCE_ROOT in your environment to control where the sources are found.
LANGUAGE_SOURCE_ROOT = _get_language_source_root(PROJECT_PATH, env=os.environ)

# Set LANGUAGE_BUILD_ROOT to a directory that will contain a subdirectory for each build
# configuration
LANGUAGE_BUILD_ROOT = _get_language_build_root(LANGUAGE_SOURCE_ROOT, env=os.environ)

# Set LANGUAGE_REPO_NAME in your environment to control the name of the language directory
# name that is used.
LANGUAGE_REPO_NAME = _get_language_repo_name(PROJECT_PATH, env=os.environ)
