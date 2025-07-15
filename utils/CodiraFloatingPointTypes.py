# ===--- CodiraFloatingPointTypes.py ------------------*- coding: utf-8 -*-===//
#
# This source file is part of the Codira.org open source project
#
# Copyright (c) 2014 - 2017 Apple Inc. and the Codira project authors
# Licensed under Apache License v2.0 with Runtime Library Exception
#
# See https://language.org/LICENSE.txt for license information
# See https://language.org/CONTRIBUTORS.txt for the list of Codira project authors


# returns (lower, upper) exclusive bounds for the integer values
# that can be stored into a float
def getFtoIBounds(floatBits, intBits, signed):
    floatTy = floating_point_bits_to_type()[floatBits]
    sigBits = floatTy.explicit_significand_bits
    if not signed:
        return (-1, 1 << intBits)
    upper = 1 << (intBits - 1)
    if intBits <= sigBits:
        return (-upper - 1, upper)
    ulp = 1 << (intBits - sigBits)
    return (-upper - ulp, upper)


class CodiraFloatType(object):

    def __init__(self, name, cFuncSuffix, significandBits, exponentBits,
                 significandSize, totalBits):
        self.stdlib_name = name
        self.cFuncSuffix = cFuncSuffix
        self.significand_bits = significandBits
        self.significand_size = significandSize
        self.exponent_bits = exponentBits
        self.explicit_significand_bits = significandBits + 1
        self.bits = totalBits


def floating_point_bits_to_type():
    return {
        16: CodiraFloatType(name="Float16", cFuncSuffix="f", significandBits=10,
                           exponentBits=5, significandSize=16, totalBits=16),
        32: CodiraFloatType(name="Float", cFuncSuffix="f", significandBits=23,
                           exponentBits=8, significandSize=32, totalBits=32),
        64: CodiraFloatType(name="Double", cFuncSuffix="", significandBits=52,
                           exponentBits=11, significandSize=64, totalBits=64),
        80: CodiraFloatType(name="Float80", cFuncSuffix="l", significandBits=63,
                           exponentBits=15, significandSize=64, totalBits=80),
    }


def all_floating_point_types():
    return floating_point_bits_to_type().values()
