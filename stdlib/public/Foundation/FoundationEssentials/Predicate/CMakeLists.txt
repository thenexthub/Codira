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
    Archiving/EncodingContainers+PredicateExpression.code
    Archiving/Predicate+Codable.code
    Archiving/PredicateCodableConfiguration.code
    Archiving/PredicateExpressionConstruction.code
    
    Expressions/Aggregate.code
    Expressions/Arithmetic.code
    Expressions/Collection.code
    Expressions/Comparison.code
    Expressions/Conditional.code
    Expressions/Conjunction.code
    Expressions/Dictionary.code
    Expressions/Disjunction.code
    Expressions/Division.code
    Expressions/Equality.code
    Expressions/ExpressionEvaluation.code
    Expressions/Filter.code
    Expressions/Inequality.code
    Expressions/Negation.code
    Expressions/Optional.code
    Expressions/PredicateEvaluation.code
    Expressions/Range.code
    Expressions/Regex.code
    Expressions/Sequence.code
    Expressions/StringComparison.code
    Expressions/Types.code
    Expressions/UnaryMinus.code
    
    Expression.code
    KeyPath+Inspection.code
    NSPredicateConversion.code
    Predicate+Description.code
    Predicate.code
    PredicateBindings.code
    PredicateExpression.code)
    