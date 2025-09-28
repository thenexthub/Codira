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
    Locale+Components.code
    Locale+Language.code
    Locale.code
    Locale_Autoupdating.code
    Locale_Cache.code
    Locale_Notifications.code
    Locale_Preferences.code
    Locale_Protocol.code
    Locale_Unlocalized.code)
