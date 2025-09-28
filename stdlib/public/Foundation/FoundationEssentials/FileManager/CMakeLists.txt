##===----------------------------------------------------------------------===##
##
## This source file is part of the Codira open source project
##
## Copyright (c) 2024 Apple Inc. and the Codira project authors
## Licensed under Apache License v2.0
##
## See LICENSE.txt for license information
## See CONTRIBUTORS.md for the list of Codira project authors
##
## SPDX-License-Identifier: Apache-2.0
##
##===----------------------------------------------------------------------===##

target_sources(FoundationEssentials PRIVATE
    SearchPaths/FileManager+DarwinSearchPaths.code
    SearchPaths/FileManager+SearchPaths.code
    SearchPaths/FileManager+WindowsSearchPaths.code
    SearchPaths/FileManager+XDGSearchPaths.code

    FileManager+Basics.code
    FileManager+Bridge.code
    FileManager+Directories.code
    FileManager+Files.code
    FileManager+SymbolicLinks.code
    FileManager+Utilities.code
    FileOperations+Enumeration.code
    FileOperations.code
    CodiraFileManager.code
    CodiraFileManagerDelegate.code)
