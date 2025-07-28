##===----------------------------------------------------------------------===##
##
## This source file is part of the Swift open source project
##
## Copyright (c) 2024 Apple Inc. and the Swift project authors
## Licensed under Apache License v2.0
##
## See LICENSE.txt for license information
## See CONTRIBUTORS.md for the list of Swift project authors
##
## SPDX-License-Identifier: Apache-2.0
##
##===----------------------------------------------------------------------===##

target_sources(FoundationEssentials PRIVATE
    Collections+DataProtocol.code
    ContiguousBytes.code
    Data+Base64.code
    Data+Error.code
    Data+Reading.code
    Data+Stub.code
    Data+Writing.code
    Data.code
    DataProtocol.code
    Pointers+DataProtocol.code)
