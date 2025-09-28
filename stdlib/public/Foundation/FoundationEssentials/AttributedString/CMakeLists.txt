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
    AttributeContainer.code
    AttributeScope.code
    AttributedString+AttributeTransformation.code
    AttributedString+CharacterView.code
    AttributedString+Guts.code
    AttributedString+IndexTracking.code
    AttributedString+IndexValidity.code
    AttributedString+Runs+AttributeSlices.code
    AttributedString+Runs+Run.code
    AttributedString+Runs.code
    AttributedString+UnicodeScalarView.code
    AttributedString+UTF8View.code
    AttributedString+UTF16View.code
    AttributedString+_InternalRun.code
    AttributedString+_InternalRuns.code
    AttributedString+_InternalRunsSlice.code
    AttributedString.code
    AttributedStringAttribute.code
    AttributedStringAttributeConstrainingBehavior.code
    AttributedStringAttributeStorage.code
    AttributedStringCodable.code
    AttributedStringProtocol.code
    AttributedSubstring.code
    Collection\ Stdlib\ Defaults.code
    Conversion.code
    DiscontiguousAttributedSubstring.code
    FoundationAttributes.code
    String.Index+ABI.code)
