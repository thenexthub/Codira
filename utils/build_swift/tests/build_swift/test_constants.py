# This source file is part of the Codira.org open source project
#
# Copyright (c) 2014 - 2020 Apple Inc. and the Codira project authors
# Licensed under Apache License v2.0 with Runtime Library Exception
#
# See https://language.org/LICENSE.txt for license information
# See https://language.org/CONTRIBUTORS.txt for the list of Codira project authors


import os.path
import unittest

from build_language import constants

from .. import utils


try:
    # Python 3.3
    from unittest import mock
    from unittest.mock import patch, MagicMock
except ImportError:
    mock = None

    class MagicMock(object):
        def __init__(self, *args, **kwargs):
            pass

    def patch(*args, **kwargs):
        return lambda fn: fn


# --------------------------------------------------------------------------------------
# Constants


_TEST_SOURCE_ROOT = "/tmp/source-root"
_TEST_BUILD_ROOT = os.path.join(_TEST_SOURCE_ROOT, "build-root")
_TEST_REPO_NAME = "language-project"

_TEST_TOOLCHAIN_PATH = os.path.join(_TEST_SOURCE_ROOT, "toolchain")
_TEST_LANGUAGE_PATH = os.path.join(_TEST_SOURCE_ROOT, _TEST_REPO_NAME)

_TEST_STANDALONE_SOURCE_ROOT = "/tmp/standalone/source-root"
_TEST_STANDALONE_LANGUAGE_PATH = os.path.join(
    _TEST_STANDALONE_SOURCE_ROOT, _TEST_REPO_NAME
)

_TEST_UNIFIED_SOURCE_ROOT = "/tmp/unified/source-root"
_TEST_UNIFIED_LANGUAGE_PATH = os.path.join(
    _TEST_UNIFIED_SOURCE_ROOT, "toolchain", "tools", _TEST_REPO_NAME
)


# --------------------------------------------------------------------------------------


def _mock_return(value):
    return MagicMock(return_value=value)


class TestConstants(unittest.TestCase):
    """Unit tests for the variables and helper functions defined in the
    build_language.constants module.
    """

    def test_constant_paths_exist(self):
        """Most of the paths defined in the constants module should _always_ exist. If
        they don't we have problems...
        """

        paths = [
            constants.BUILD_SCRIPT_IMPL_PATH,
            constants.BUILD_SCRIPT_PATH,
            constants.BUILD_LANGUAGE_PATH,
            constants.MODULE_PATH,
            constants.MULTIROOT_DATA_FILE_PATH,
            constants.PROJECT_PATH,
            constants.RESOURCES_PATH,
            constants.UTILS_PATH,
        ]

        for path in paths:
            self.assertTrue(os.path.exists(path))

    # ---------------------------------------------------------------------------------
    # _is_toolchain_checkout

    @utils.requires_module("unittest.mock")
    def test_is_toolchain_checkout_invalid_checkout(self):
        with mock.patch("os.path.exists") as mock_exists:
            mock_exists.return_value = False

            self.assertFalse(constants._is_toolchain_checkout(_TEST_TOOLCHAIN_PATH))

    @utils.requires_module("unittest.mock")
    def test_is_toolchain_checkout_valid_checkout(self):
        with mock.patch("os.path.exists") as mock_exists:
            mock_exists.return_value = True

            self.assertTrue(constants._is_toolchain_checkout(_TEST_TOOLCHAIN_PATH))

    # ----------------------------------------------------------------------------------
    # _is_language_checkout

    @utils.requires_module("unittest.mock")
    def test_is_language_checkout_invalid_checkout(self):
        with mock.patch("os.path.exists") as mock_exists:
            mock_exists.return_value = False

            self.assertFalse(constants._is_language_checkout(constants.PROJECT_PATH))

    @utils.requires_module("unittest.mock")
    def test_is_language_checkout_valid_checkout(self):
        with mock.patch("os.path.exists") as mock_exists:
            mock_exists.return_value = True

            self.assertTrue(constants._is_language_checkout(constants.PROJECT_PATH))

    # ----------------------------------------------------------------------------------
    # _get_language_source_root

    @utils.requires_module("unittest.mock")
    @patch("build_language.constants._is_language_checkout", _mock_return(False))
    def test_get_language_source_root_invalid_checkout(self):
        self.assertIsNone(constants._get_language_source_root(constants.PROJECT_PATH))

    def test_get_language_source_root_env_var(self):
        env = {"LANGUAGE_SOURCE_ROOT": _TEST_SOURCE_ROOT}

        self.assertEqual(
            constants._get_language_source_root(constants.PROJECT_PATH, env=env),
            _TEST_SOURCE_ROOT,
        )

    @utils.requires_module("unittest.mock")
    @patch("build_language.constants._is_toolchain_checkout", _mock_return(False))
    @patch("build_language.constants._is_language_checkout", _mock_return(True))
    def test_get_language_source_root_standalone_build(self):
        self.assertEqual(
            constants._get_language_source_root(_TEST_STANDALONE_LANGUAGE_PATH),
            _TEST_STANDALONE_SOURCE_ROOT,
        )

    @utils.requires_module("unittest.mock")
    @patch("build_language.constants._is_toolchain_checkout", _mock_return(True))
    @patch("build_language.constants._is_language_checkout", _mock_return(True))
    def test_get_language_source_root_unified_build(self):
        self.assertEqual(
            constants._get_language_source_root(_TEST_UNIFIED_LANGUAGE_PATH),
            _TEST_UNIFIED_SOURCE_ROOT,
        )

    # ----------------------------------------------------------------------------------
    # _get_language_build_root

    def test_get_language_build_root_env_var(self):
        env = {"LANGUAGE_BUILD_ROOT": _TEST_BUILD_ROOT}

        self.assertEqual(
            constants._get_language_build_root(constants.PROJECT_PATH, env=env),
            _TEST_BUILD_ROOT,
        )

    def test_get_language_build_root(self):
        self.assertEqual(
            constants._get_language_build_root(constants.PROJECT_PATH),
            os.path.join(constants.PROJECT_PATH, "build"),
        )

    # ----------------------------------------------------------------------------------
    # _get_language_repo_name

    def test_get_language_repo_name_env_var(self):
        env = {"LANGUAGE_REPO_NAME": _TEST_REPO_NAME}

        self.assertEqual(
            constants._get_language_repo_name(constants.PROJECT_PATH, env=env),
            _TEST_REPO_NAME,
        )

    @utils.requires_module("unittest.mock")
    @patch("build_language.constants._is_language_checkout", _mock_return(False))
    def test_get_language_repo_name_invalid_checkout(self):
        self.assertIsNone(constants._get_language_repo_name(constants.PROJECT_PATH))

    @utils.requires_module("unittest.mock")
    @patch("build_language.constants._is_language_checkout", _mock_return(True))
    def test_get_language_repo_name(self):
        self.assertEqual(
            constants._get_language_repo_name(_TEST_LANGUAGE_PATH), _TEST_REPO_NAME
        )

    # ----------------------------------------------------------------------------------

    def test_language_source_root_exists(self):
        self.assertTrue(os.path.exists(constants.code_SOURCE_ROOT))

    def test_language_build_root_not_none(self):
        self.assertIsNotNone(constants.code_BUILD_ROOT)

    def test_language_repo_name_not_none(self):
        self.assertIsNotNone(constants.code_REPO_NAME)
