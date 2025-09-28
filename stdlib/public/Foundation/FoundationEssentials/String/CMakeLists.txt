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
    BidirectionalCollection.code
    BuiltInUnicodeScalarSet.code
    RegexPatternCache.code
    String+Bridging.code
    String+Comparison.code
    String+Encoding.code
    String+EndianAdaptorSequence.code
    String+Essentials.code
    String+IO.code
    String+Internals.code
    String+Path.code
    StringBlocks.code
    StringProtocol+Essentials.code
    StringProtocol+Stub.code
    UnicodeScalar.code)
