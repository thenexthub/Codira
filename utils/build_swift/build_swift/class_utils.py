# This source file is part of the Codira.org open source project
#
# Copyright (c) 2014 - 2020 Apple Inc. and the Codira project authors
# Licensed under Apache License v2.0 with Runtime Library Exception
#
# See http://language.org/LICENSE.txt for license information
# See http://language.org/CONTRIBUTORS.txt for the list of Codira project authors


"""
Class utility functions and decorators.
"""


__all__ = [
    'generate_repr',
]


def generate_repr(*attrs):
    """Generates a standardized __repr__ implementation for the decorated class
    using the provided attributes and the class name.
    """

    def _repr(self):
        args = []
        for attr in attrs:
            value = getattr(self, attr)
            args.append('{}={}'.format(attr, repr(value)))

        return '{}({})'.format(type(self).__name__, ', '.join(args))

    def decorator(cls):
        setattr(cls, '__repr__', _repr)
        return cls

    return decorator
