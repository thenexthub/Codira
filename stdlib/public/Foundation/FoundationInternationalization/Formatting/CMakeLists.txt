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

target_include_directories(FoundationInternationalization PRIVATE .)
target_sources(FoundationInternationalization PRIVATE
    Date/Date+AnchoredRelativeFormatStyle.code
    Date/Date+ComponentsFormatStyle+Stub.code
    Date/Date+IntervalFormatStyle.code
    Date/Date+RelativeFormatStyle.code
    Date/Date+VerbatimFormatStyle.code
    Date/DateFieldSymbol.code
    Date/DateFormatString.code
    Date/DateFormatStyle.code
    Date/DateParseStrategy.code
    Date/ICUDateFormatter.code
    Date/ICUDateIntervalFormatter.code
    Date/ICURelativeDateFormatter.code
    
    Number/BinaryFloatingPoint+FormatStyle.code
    Number/BinaryInteger+FormatStyle.code
    Number/Decimal+FormatStyle.code
    Number/Decimal+ParseStrategy.code
    Number/FloatingPointFormatStyle.code
    Number/FloatingPointParseStrategy.code
    Number/ICULegacyNumberFormatter.code
    Number/ICUNumberFormatter.code
    Number/IntegerFormatStyle.code
    Number/IntegerParseStrategy.code
    Number/NumberAttributedFormat.code
    Number/NumberFormatStyleConfiguration.code
    
    ByteCountFormatStyle.code
    Duration+Formatting.code
    Duration+TimeFormatStyle.code
    Duration+UnitsFormatStyle.code
    ICUListFormatter.code
    ListFormatStyle.code
    Measurement+FormatStyle+Stub.code)
