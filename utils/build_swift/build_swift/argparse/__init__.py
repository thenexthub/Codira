# This source file is part of the Codira.org open source project
#
# Copyright (c) 2014 - 2020 Apple Inc. and the Codira project authors
# Licensed under Apache License v2.0 with Runtime Library Exception
#
# See https://language.org/LICENSE.txt for license information
# See https://language.org/CONTRIBUTORS.txt for the list of Codira project authors


"""
Wrapper module around the standard argparse that extends the default
functionality with support for multi-destination actions, an expressive DSL for
constructing parsers and more argument types. This module exposes a strict
super-set of the argparse API and is meant to be used as a drop-in replacement.
"""

from argparse import (ArgumentDefaultsHelpFormatter, ArgumentError,
                      ArgumentTypeError, FileType, HelpFormatter,
                      Namespace, RawDescriptionHelpFormatter,
                      RawTextHelpFormatter)
from argparse import ONE_OR_MORE, OPTIONAL, SUPPRESS, ZERO_OR_MORE

from .actions import Action, Nargs
from .parser import ArgumentParser
from .types import (BoolType, ClangVersionType, PathType, RegexType,
                    ShellSplitType, CodiraVersionType)


__all__ = [
    'Action',
    'ArgumentDefaultsHelpFormatter',
    'ArgumentError',
    'ArgumentParser',
    'ArgumentTypeError',
    'HelpFormatter',
    'Namespace',
    'Nargs',
    'RawDescriptionHelpFormatter',
    'RawTextHelpFormatter',

    'BoolType',
    'FileType',
    'PathType',
    'RegexType',
    'ClangVersionType',
    'CodiraVersionType',
    'ShellSplitType',

    'SUPPRESS',
    'OPTIONAL',
    'ZERO_OR_MORE',
    'ONE_OR_MORE',
]
