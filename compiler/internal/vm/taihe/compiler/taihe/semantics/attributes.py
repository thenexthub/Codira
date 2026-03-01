# coding=utf-8
#
#===----------------------------------------------------------------------===#
#   Copyright (c) NeXTHub Corporation. All Rights Reserved.
#   DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
#   Author: Tunjay Akbarli
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at:
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#
#   Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
#   Middletown, DE 19709, New Castle County, USA.
#===----------------------------------------------------------------------===#

"""Customizable metadata for declarations.

# Overview

The attribute subsystem provides a type-safe, hierarchical framework for
attaching metadata to declarations. It supports both validation and automatic
type checking through a plugin architecture.

## Architecture

The system follows a hierarchical design:

```
AnyAttribute
├── UncheckedAttribute: Raw attributes without validation
└── AbstractCheckedAttribute: Base class for validated attributes
    └── AutoCheckedAttribute: Automatic checking via configuration
        ├── TypedAttribute: Single-use attributes with type checking
        └── RepeatableAttribute: Multi-use attributes with type checking
```

## Lifecycle

1. **Backend Initialization**: Backends register attributes using
   `AbstractCheckedAttribute.register_to(registry)`
2. **IR Construction**: The IR converter processes unchecked attributes:
   - `AttributeRegistry.attach()` dispatches to appropriate handlers
   - `AbstractCheckedAttribute.try_construct()` validates arguments and constructs instances
"""

from abc import ABC, abstractmethod
from collections.abc import Iterable
from dataclasses import MISSING, dataclass, fields
from difflib import get_close_matches
from itertools import chain
from types import UnionType
from typing import Any, ClassVar, Generic, TypeVar, cast

from typing_extensions import Self, override

from taihe.semantics.declarations import Decl
from taihe.semantics.format import PrettyFormatter
from taihe.utils.diagnostics import DiagnosticsManager
from taihe.utils.exceptions import (
    AttrArgMissingError,
    AttrArgOrderError,
    AttrArgRedefError,
    AttrArgTypeError,
    AttrArgUnrequiredError,
    AttrConflictError,
    AttrNotExistError,
    AttrTargetError,
)
from taihe.utils.sources import SourceLocation


@dataclass
class Argument:
    """Represents a single argument within an attribute invocation.

    Attributes store both the source location (for error reporting) and the
    evaluated value of each argument.
    """

    loc: SourceLocation | None
    """Source location of this argument for error reporting.

    For positional arguments, this covers the entire argument expression.
    For keyword arguments, this covers both the key and value.

    Example:
    ```
    # For positional arguments
    @foo(2 + 3)
         ^^^^^

    # For keyword arguments (in the future?)
    @foo(bar="baz")
         ^^^^^^^^^
    ```
    """

    key: str | None
    """The name of the argument if it is a keyword argument, or None for positional arguments."""

    value: float | bool | int | str
    """The evaluated constant value of the argument."""


@dataclass
class AnyAttribute(ABC):
    """Base class for all attributes, both checked and unchecked.

    This serves as a common interface for both raw attributes (UncheckedAttribute)
    and validated attributes (AbstractCheckedAttribute). It provides a unified
    way to retrieve the name and arguments of an attribute for diagnostics.
    """

    loc: SourceLocation | None
    """Source location of the attribute name for error reporting."""

    @property
    def description(self) -> str:
        """Provides a human-readable description of the attribute.

        This can be overridden by subclasses to provide more context.
        """
        return f"attribute {PrettyFormatter().get_format_attr(self)}"

    @abstractmethod
    def get_name(self) -> str:
        """Returns the name of the attribute without the '@' prefix.

        This is used for diagnostics and debugging purposes.
        """

    @abstractmethod
    def get_args(self) -> Iterable[Argument]:
        """Returns the list of arguments passed to this attribute.

        This is used for diagnostics and debugging purposes.
        """

    @abstractmethod
    def check_context(self, parent: Decl, dm: DiagnosticsManager) -> None:
        """Checks if this attribute can be attached to the given declaration.

        This method should be implemented by subclasses to enforce specific
        attachment rules. It should not raise exceptions, but instead use
        the diagnostics manager to report any issues.

        Args:
            parent: The declaration to check against
            dm: Diagnostics manager for error reporting
        """


@dataclass
class UncheckedAttribute(AnyAttribute):
    """Raw attribute data before type checking and validation.

    This represents the syntactic form of an attribute as parsed from source code,
    before any semantic analysis or type checking has been performed.
    """

    name: str
    """The attribute name as it appears in source code (without @ prefix)."""

    args: list[Argument]
    """Positional arguments passed to the attribute."""

    @classmethod
    def consume(cls, decl: Decl) -> Iterable[Self]:
        """Yields all unchecked attributes from a declaration.

        This method iterates through the declaration's attributes and yields
        each unchecked attribute of the specified class, removing it from the
        declaration's attribute list.
        """
        unchecked_attrs = decl.find_attributes(cls)
        while unchecked_attrs:
            yield unchecked_attrs.pop(0)

    @override
    def get_name(self) -> str:
        return self.name

    @override
    def get_args(self) -> Iterable[Argument]:
        return self.args

    @override
    def check_context(self, parent: Decl, dm: DiagnosticsManager) -> None:
        pass


class AbstractCheckedAttribute(AnyAttribute, ABC):
    """Base class for validated attributes with pluggable checking logic.

    This provides the low-level framework for implementing custom attributes.
    Most backend developers should use `TypedAttribute` or `RepeatableAttribute`
    instead, which provide automatic type checking.

    ## Inheritance Constraints

    While you can subclass, registering or querying the base class is not
    supported. For example:
    ```
    class Base(CheckedAttribute)
    class DerivedA(Base)
    class DerivedB(Base)

    Base.register_to(registry)      # Error
    DerivedA.register_to(registry)  # OK
    DerivedB.register_to(registry)  # OK
    Base.get(d)                     # Error
    DerivedA.get(d)                 # OK
    DerivedB.get(d)                 # OK
    ```
    """

    @classmethod
    @abstractmethod
    def register_to(cls, registry: "AttributeRegistry") -> None:
        """Registers this attribute type with the given registry.

        Args:
            registry: The registry to register with

        This method defines how the attribute should be looked up during
        the attachment phase.
        """

    @classmethod
    @abstractmethod
    def try_construct(
        cls,
        raw: UncheckedAttribute,
        dm: DiagnosticsManager,
    ) -> Self | None:
        """Process the validated arguments and construct the attribute instance.

        This method should be implemented by subclasses to handle specific
        argument processing logic.

        Args:
            raw: The unchecked attribute data from parsing
            dm: Diagnostics manager for error reporting

        Returns:
            An instance of the attribute on success, None on failure
        """


class AttributeGroupTag:
    pass


_D = TypeVar("_D", bound=Decl)


class AutoCheckedAttribute(AbstractCheckedAttribute, Generic[_D]):
    """Base class providing automatic name inference and target checking.

    This class implements common patterns for attribute registration and
    target validation, reducing boilerplate in concrete attribute implementations.
    """

    NAME: ClassVar[str]
    """Explicit attribute name."""

    TARGETS: ClassVar[tuple[type[_D], ...]]  # type: ignore
    """Declaration types this attribute can be attached to.

    The system uses isinstance() checking, so inheritance hierarchies are properly
    supported.

    Use `(Decl,)` to indicate the attribute can be attached to any declaration.
    """

    ATTRIBUTE_GROUP_TAGS: ClassVar[frozenset[AttributeGroupTag]] = frozenset()
    """Set of tags indicating mutually exclusive attribute groups.

    If this is non-empty, the attribute cannot coexist with any other
    attribute that has any of these tags.
    """

    @override
    @classmethod
    def register_to(cls, registry: "AttributeRegistry") -> None:
        registry.register_one(cls.NAME, cls)

    @override
    @classmethod
    def try_construct(
        cls,
        raw: UncheckedAttribute,
        dm: DiagnosticsManager,
    ) -> Self | None:
        args: list[Argument] = []
        kwargs: dict[str, Argument] = {}

        for arg in raw.args:
            if arg.key is None:
                if kwargs:
                    dm.emit(AttrArgOrderError(arg))
                    return None
                args.append(arg)
            else:
                if (prev := kwargs.get(arg.key)) is not None:
                    dm.emit(AttrArgRedefError(prev, arg))
                    return None
                kwargs[arg.key] = arg

        return cls.try_construct_from_parsed_args(raw.loc, args, kwargs, dm)

    @classmethod
    def try_construct_from_parsed_args(
        cls,
        loc: SourceLocation | None,
        args: list[Argument],
        kwargs: dict[str, Argument],
        dm: DiagnosticsManager,
    ) -> Self | None:
        """Constructs the attribute instance from parsed arguments.

        This method processes the positional and keyword arguments, validates
        them against the dataclass fields, and constructs the attribute instance.

        Args:
            loc: Source location of the attribute for error reporting
            name: The name of the attribute (without @ prefix)
            args: Positional arguments passed to the attribute
            kwargs: Keyword arguments passed to the attribute
            dm: Diagnostics manager for error reporting

        Returns:
            An instance of the attribute on success, None on failure
        """
        dataclass_arguments: dict[str, Any] = {}

        for field in fields(cls):
            if field.name == "loc":
                # Special handling for the 'loc' field
                dataclass_arguments["loc"] = loc
                continue
            if field.init is False:
                # Skip fields that are not intended for initialization
                continue
            if field.name in kwargs:
                # If the field is in keyword arguments, use that value
                arg = kwargs.pop(field.name)
            elif field.kw_only:
                # If no value is provided and no default, emit an error
                if field.default is not MISSING or field.default_factory is not MISSING:
                    continue
                dm.emit(AttrArgMissingError(cls.NAME, field.name, False, loc=loc))
                return None
            elif args:
                # If the field is positional, pop the first positional argument
                arg = args.pop(0)
            else:
                # If no value is provided and no default, emit an error
                if field.default is not MISSING or field.default_factory is not MISSING:
                    continue
                dm.emit(AttrArgMissingError(cls.NAME, field.name, False, loc=loc))
                return None

            # Validate the type of the provided value
            field_type = cast(type | UnionType, field.type)
            if not isinstance(arg.value, field_type):
                dm.emit(AttrArgTypeError(cls.NAME, field.name, field_type, arg))
                return None

            # Store the validated value in the dataclass kwargs
            dataclass_arguments[field.name] = arg.value

        # If there are any remaining keyword arguments, emit an error
        for attr_arg in chain(args, kwargs.values()):
            dm.emit(AttrArgUnrequiredError(cls.NAME, attr_arg))
            return None

        return cls(**dataclass_arguments)

    @override
    def check_context(self, parent: Decl, dm: DiagnosticsManager) -> None:
        if not isinstance(parent, self.TARGETS):
            dm.emit(AttrTargetError(parent, self))
            return

        self.check_typed_context(parent, dm)

    def check_typed_context(self, parent: _D, dm: DiagnosticsManager) -> None:
        """Checks if this attribute can be attached to the given declaration.

        Notice that parent type is already checked outside.

        Args:
            parent: The declaration to check against
            dm: Diagnostics manager for error reporting

        Returns:
            True if the attribute can be attached, False otherwise
        """
        for prev in chain(*parent.attributes.values()):
            if type(prev) is type(self):
                continue
            if not isinstance(prev, AutoCheckedAttribute):
                continue
            if self.ATTRIBUTE_GROUP_TAGS & prev.ATTRIBUTE_GROUP_TAGS:
                dm.emit(AttrConflictError(prev, self))  # type: ignore

    @override
    def get_name(self) -> str:
        return self.NAME

    @override
    def get_args(self) -> Iterable[Argument]:
        for field in fields(self):
            if field.name == "loc":
                # Skip the 'loc' field in the argument list
                continue
            if field.init is False:
                # Skip fields that are not intended for initialization
                continue
            value = getattr(self, field.name, None)
            if value is not None:
                yield Argument(key=field.name, value=value, loc=None)


class TypedAttribute(AutoCheckedAttribute[_D]):
    """Type-checked attribute that can be attached at most once per declaration."""

    @classmethod
    def get(cls, decl: _D) -> Self | None:
        """Retrieves the single instance of this attribute from a declaration.

        Args:
            decl: The declaration to search

        Returns:
            The attribute instance if present, None otherwise
        """
        if attrs := decl.find_attributes(cls):
            return attrs[0]
        return None

    @override
    def check_typed_context(self, parent: _D, dm: DiagnosticsManager) -> None:
        prev = self.get(parent)
        if prev is not None and prev is not self:
            dm.emit(AttrConflictError(prev, self))

        super().check_typed_context(parent, dm)


class RepeatableAttribute(AutoCheckedAttribute[_D]):
    """Type-checked attribute that can be attached multiple times per declaration."""

    @classmethod
    def get_all(cls, decl: _D) -> list[Self]:
        """Retrieves all instances of this attribute from a declaration.

        Args:
            decl: The declaration to search

        Returns:
            List of attribute instances (empty if none present)
        """
        return decl.find_attributes(cls)


# Type aliases for clarity
CheckedAttrT = type[AbstractCheckedAttribute]


class AttributeRegistry:
    """Registry for mapping attribute names to their implementation classes.

    This registry serves as the central dispatch mechanism during IR construction,
    allowing the system to convert unchecked attributes to their typed equivalents.
    """

    def __init__(self) -> None:
        self._name_to_attr: dict[str, CheckedAttrT] = {}

    def register_one(self, name: str, attr_type: CheckedAttrT) -> None:
        """Registers a single attribute type with the given name.

        Args:
            name: The attribute name (without @ prefix)
            attr_type: The attribute implementation class

        Raises:
            ValueError: If an attribute with this name is already registered
        """
        if name in self._name_to_attr:
            existing = self._name_to_attr[name]
            raise ValueError(
                f"Attribute '{name}' already registered to {existing.__qualname__}, "
                f"cannot register {attr_type.__qualname__}"
            )
        self._name_to_attr[name] = attr_type

    def register(self, *attr_types: CheckedAttrT) -> None:
        """Registers multiple attribute types using their inferred names.

        Args:
            *attr_types: Attribute classes to register

        This is a convenience method that calls register_to() on each class.
        """
        for attr_type in attr_types:
            attr_type.register_to(self)

    def attach(
        self,
        raw: UncheckedAttribute,
        dm: DiagnosticsManager,
    ) -> AbstractCheckedAttribute | None:
        """Validates and constructs a typed attribute from unchecked attribute data.

        This method orchestrates the entire attachment process:
        1. Looks up the attribute type by name
        2. Constructs the typed attribute instance

        Args:
            raw: Unchecked attribute data from parsing
            dm: Diagnostics manager for error reporting
        """
        attr_type = self._name_to_attr.get(raw.name)
        if attr_type is None:
            suggestions = get_close_matches(raw.name, self._name_to_attr.keys())
            dm.emit(AttrNotExistError(raw.name, suggestions, loc=raw.loc))
            return None

        return attr_type.try_construct(raw, dm)