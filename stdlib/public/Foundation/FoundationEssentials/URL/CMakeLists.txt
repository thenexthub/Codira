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
    URL.code
    URL_Bridge.code
    URL_ObjC.code
    URL_Protocol.code
    URL_Swift.code
    URLComponents.code
    URLComponents_ObjC.code
    URLParser.code
    URLTemplate_Expression.code
    URLTemplate_PercentEncoding.code
    URLTemplate_Substitution.code
    URLTemplate_Value.code
    URLTemplate_VariableName.code
    URLTemplate.code)
