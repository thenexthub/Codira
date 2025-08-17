# language_build_support/products/libdispatch.py -------------------*- python -*-
#
# This source file is part of the Swift.org open source project
#
# Copyright (c) 2014 - 2017 Apple Inc. and the Swift project authors
# Licensed under Apache License v2.0 with Runtime Library Exception
#
# See https://language.org/LICENSE.txt for license information
# See https://language.org/CONTRIBUTORS.txt for the list of Swift project authors
#
# ----------------------------------------------------------------------------

from . import cmark
from . import libcxx
from . import toolchain
from . import product
from . import language


class LibDispatch(product.Product):
    @classmethod
    def is_build_script_impl_product(cls):
        """is_build_script_impl_product -> bool

        Whether this product is produced by build-script-impl.
        """
        return True

    @classmethod
    def is_before_build_script_impl_product(cls):
        """is_before_build_script_impl_product -> bool

        Whether this product is built before any build-script-impl products.
        """
        return False

    @classmethod
    def product_source_name(cls):
        """product_source_name() -> str

        The name of the source code directory of this product.
        """
        return "language-corelibs-libdispatch"

    @classmethod
    def is_nondarwin_only_build_product(cls):
        return True

    @classmethod
    def get_dependencies(cls):
        return [cmark.CMark,
                toolchain.LLVM,
                libcxx.LibCXX,
                language.Swift]
