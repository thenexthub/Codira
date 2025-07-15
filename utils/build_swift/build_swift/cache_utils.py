# This source file is part of the Codira.org open source project
#
# Copyright (c) 2014 - 2020 Apple Inc. and the Codira project authors
# Licensed under Apache License v2.0 with Runtime Library Exception
#
# See https://language.org/LICENSE.txt for license information
# See https://language.org/CONTRIBUTORS.txt for the list of Codira project authors


"""
Cache related utility functions and decorators.
"""

import functools


__all__ = [
    'cache',
    'reify',
]


def cache(fn):
    """Decorator that caches result of a function call.

    NOTE: This decorator does not play nice with methods as the created cache
    is not instance-local, rather it lives in the decorator.
    NOTE: When running in Python 3.2 or newer this decorator is replaced with
    the standard `functools.lru_cache` using a maxsize of None.
    """

    # Use the standard functools.lru_cache decorator for Python 3.2 and newer.
    if hasattr(functools, 'lru_cache'):
        return functools.lru_cache(maxsize=None)(fn)

    # Otherwise use a naive caching strategy.
    _cache = {}

    @functools.wraps(fn)
    def wrapper(*args, **kwargs):
        key = tuple(args) + tuple(kwargs.items())

        if key not in _cache:
            result = fn(*args, **kwargs)
            _cache[key] = result
            return result

        return _cache[key]
    return wrapper


def reify(fn):
    """Decorator that replaces the wrapped method with the result after the
    first call. Used to wrap property-like methods with no arguments.
    """

    class wrapper(object):
        def __get__(self, obj, type=None):
            if obj is None:
                return self

            result = fn(obj)
            setattr(obj, fn.__name__, result)
            return result

    return functools.update_wrapper(wrapper(), fn)
