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

"""Format the IDL files."""

from collections.abc import Callable
from itertools import chain
from json import dumps
from typing import TYPE_CHECKING

from typing_extensions import override

from taihe.semantics.visitor import ExplicitTypeRefVisitor, RecursiveDeclVisitor
from taihe.utils.diagnostics import AnsiStyle
from taihe.utils.outputs import BaseWriter

if TYPE_CHECKING:
    from taihe.semantics.attributes import AnyAttribute
    from taihe.semantics.declarations import (
        CallbackTypeRefDecl,
        Decl,
        DeclarationImportDecl,
        DeclarationRefDecl,
        EnumDecl,
        EnumItemDecl,
        ExplicitTypeRefDecl,
        GenericArgDecl,
        GenericTypeRefDecl,
        GlobFuncDecl,
        IfaceDecl,
        IfaceExtendDecl,
        IfaceMethodDecl,
        LongTypeRefDecl,
        PackageDecl,
        PackageGroup,
        PackageImportDecl,
        PackageRefDecl,
        ParamDecl,
        ShortTypeRefDecl,
        StructDecl,
        StructFieldDecl,
        UnionDecl,
        UnionFieldDecl,
    )

WrapF = Callable[[str], str]


class PrettyFormatter(ExplicitTypeRefVisitor[str]):
    as_keyword: WrapF
    as_attr: WrapF
    as_comment: WrapF

    def __init__(self, show_resolved: bool = False, colorize: bool = False):
        self.show_resolved = show_resolved
        if colorize:
            self.as_keyword = lambda s: f"{AnsiStyle.CYAN}{s}{AnsiStyle.RESET}"
            self.as_attr = lambda s: f"{AnsiStyle.MAGENTA}{s}{AnsiStyle.RESET}"
            self.as_comment = lambda s: f"{AnsiStyle.GREEN}{s}{AnsiStyle.RESET}"
        else:
            self.as_keyword = lambda s: s
            self.as_attr = lambda s: s
            self.as_comment = lambda s: s

    def with_attr(self, d: "Decl", s: str, bracket: bool = False) -> str:
        attrs: list[str] = []
        for item in chain(*d.attributes.values()):
            attr = self.as_attr(f"@{self.get_format_attr(item)}")
            attrs.append(attr)
        if not attrs:
            return s
        attrs_fmt = " ".join(attrs)
        if bracket:
            attrs_fmt = f"[{attrs_fmt}]"
        return f"{attrs_fmt} {s}"

    def get_type_ref(self, d: "ExplicitTypeRefDecl") -> str:
        type_ref_repr = d.accept(self)
        if not d.is_resolved or not self.show_resolved:
            return type_ref_repr
        if (ty := d.resolved_ty_or_none) is None:
            ty_sig = "<ERROR>"
        else:
            ty_sig = ty.signature
        comment = self.as_comment(f"/* {ty_sig} */")
        return f"{type_ref_repr} {comment}"

    @override
    def visit_long_type_ref(self, d: "LongTypeRefDecl") -> str:
        return self.with_attr(d, f"{d.pkname}.{d.symbol}")

    @override
    def visit_short_type_ref(self, d: "ShortTypeRefDecl") -> str:
        return self.with_attr(d, d.symbol)

    @override
    def visit_generic_type_ref(self, d: "GenericTypeRefDecl") -> str:
        args_fmt = ", ".join(map(self.get_generic_arg_decl, d.args))
        return self.with_attr(d, f"{d.symbol}<{args_fmt}>")

    @override
    def visit_callback_type_ref(self, d: "CallbackTypeRefDecl") -> str:
        params_fmt = ", ".join(map(self.get_param_decl, d.params))
        ret = d.return_ty_ref.format(self)
        return self.with_attr(d, f"({params_fmt}) => {ret}")

    def get_package_ref(self, d: "PackageRefDecl") -> str:
        package_ref_repr = d.symbol
        if not d.is_resolved or not self.show_resolved:
            return package_ref_repr
        if (pkg := d.resolved_pkg_or_none) is None:
            pkg_desc = "<ERROR>"
        else:
            pkg_desc = pkg.description
        comment = self.as_comment(f"/* {pkg_desc} */")
        return f"{package_ref_repr} {comment}"

    def get_declaration_ref(self, d: "DeclarationRefDecl") -> str:
        decl_ref_repr = d.symbol
        if not d.is_resolved or not self.show_resolved:
            return decl_ref_repr
        if (decl := d.resolved_decl_or_none) is None:
            desc_desc = "<ERROR>"
        else:
            desc_desc = decl.description
        comment = self.as_comment(f"/* {desc_desc} */")
        return f"{decl_ref_repr} {comment}"

    def get_generic_arg_decl(self, d: "GenericArgDecl") -> str:
        res = d.ty_ref.format(self)
        return self.with_attr(d, res, bracket=True)

    def get_extend_decl(self, d: "IfaceExtendDecl") -> str:
        res = d.ty_ref.format(self)
        return self.with_attr(d, res, bracket=True)

    def get_param_decl(self, d: "ParamDecl") -> str:
        res = f"{d.name}: {d.ty_ref.format(self)}"
        return self.with_attr(d, res)

    def get_format_attr(self, item: "AnyAttribute") -> str:
        name = item.get_name()
        args = item.get_args()
        args_str: list[str] = []
        for arg in args:
            value = dumps(arg.value)
            if arg.key:
                arg_str = f"{arg.key}={value}"
            else:
                arg_str = value
            args_str.append(arg_str)
        if not args_str:
            return name
        args_fmt = ", ".join(args_str)
        return f"{name}({args_fmt})"


class PrettyPrinter(RecursiveDeclVisitor):
    def __init__(
        self,
        out: BaseWriter,
        show_resolved: bool = False,
        colorize: bool = False,
    ):
        self.out = out
        self.fmt = PrettyFormatter(show_resolved, colorize)

    def write_pkg_attr(self, d: "PackageDecl"):
        for item in chain(*d.attributes.values()):
            attr = self.fmt.as_attr(f"@!{self.fmt.get_format_attr(item)}")
            self.out.writeln(f"{attr}")

    def write_attr(self, d: "Decl"):
        for item in chain(*d.attributes.values()):
            attr = self.fmt.as_attr(f"@{self.fmt.get_format_attr(item)}")
            self.out.writeln(f"{attr}")

    @override
    def visit_package_import(self, d: "PackageImportDecl"):
        self.write_attr(d)

        use_kw = self.fmt.as_keyword("use")
        as_kw = self.fmt.as_keyword("as")

        alias_pair = (
            f"{self.fmt.get_package_ref(d.pkg_ref)} {as_kw} {d.name}"
            if d.is_alias()
            else self.fmt.get_package_ref(d.pkg_ref)
        )

        self.out.writeln(f"{use_kw} {alias_pair};")

    @override
    def visit_declaration_import(self, d: "DeclarationImportDecl"):
        self.write_attr(d)

        from_kw = self.fmt.as_keyword("from")
        use_kw = self.fmt.as_keyword("use")
        as_kw = self.fmt.as_keyword("as")

        alias_pair = (
            f"{self.fmt.get_declaration_ref(d.decl_ref)} {as_kw} {d.name}"
            if d.is_alias()
            else self.fmt.get_declaration_ref(d.decl_ref)
        )

        self.out.writeln(
            f"{from_kw} {self.fmt.get_package_ref(d.decl_ref.pkg_ref)} {use_kw} {alias_pair};"
        )

    @override
    def visit_glob_func(self, d: "GlobFuncDecl"):
        self.write_attr(d)

        func_kw = self.fmt.as_keyword("function")

        params_fmt = ", ".join(map(self.fmt.get_param_decl, d.params))

        if (ret := d.return_ty_ref.format(self.fmt)) is None:
            self.out.writeln(f"{func_kw} {d.name}({params_fmt});")
        else:
            self.out.writeln(f"{func_kw} {d.name}({params_fmt}): {ret};")

    @override
    def visit_enum_item(self, d: "EnumItemDecl") -> None:
        self.write_attr(d)

        if d.value is None:
            self.out.writeln(f"{d.name},")
        else:
            self.out.writeln(f"{d.name} = {dumps(d.value)},")

    @override
    def visit_enum_decl(self, d: "EnumDecl") -> None:
        self.write_attr(d)

        enum_kw = self.fmt.as_keyword("enum")

        full_decl = f"{d.name}: {d.ty_ref.format(self.fmt)}"
        prologue = f"{enum_kw} {full_decl} {{"
        epilogue = f"}}"

        if d.items:
            with self.out.indented(prologue, epilogue):
                for i in d.items:
                    i.accept(self)
        else:
            self.out.writeln(prologue + epilogue)

    @override
    def visit_union_field(self, d: "UnionFieldDecl"):
        self.write_attr(d)

        if (ret := d.ty_ref.format(self.fmt)) is None:
            self.out.writeln(f"{d.name};")
        else:
            self.out.writeln(f"{d.name}: {ret};")

    @override
    def visit_union_decl(self, d: "UnionDecl"):
        self.write_attr(d)

        union_kw = self.fmt.as_keyword("union")
        prologue = f"{union_kw} {d.name} {{"
        epilogue = f"}}"

        if d.fields:
            with self.out.indented(prologue, epilogue):
                for f in d.fields:
                    f.accept(self)
        else:
            self.out.writeln(prologue + epilogue)

    @override
    def visit_struct_field(self, d: "StructFieldDecl"):
        self.write_attr(d)

        self.out.writeln(f"{d.name}: {d.ty_ref.format(self.fmt)};")

    @override
    def visit_struct_decl(self, d: "StructDecl"):
        self.write_attr(d)

        struct_kw = self.fmt.as_keyword("struct")
        prologue = f"{struct_kw} {d.name} {{"
        epilogue = f"}}"

        if d.fields:
            with self.out.indented(prologue, epilogue):
                for f in d.fields:
                    f.accept(self)
        else:
            self.out.writeln(prologue + epilogue)

    @override
    def visit_iface_method(self, d: "IfaceMethodDecl"):
        self.write_attr(d)

        params_fmt = ", ".join(map(self.fmt.get_param_decl, d.params))

        if (ret := d.return_ty_ref.format(self.fmt)) is None:
            self.out.writeln(f"{d.name}({params_fmt});")
        else:
            self.out.writeln(f"{d.name}({params_fmt}): {ret};")

    @override
    def visit_iface_decl(self, d: "IfaceDecl"):
        self.write_attr(d)

        iface_kw = self.fmt.as_keyword("interface")

        full_decl = (
            f"{d.name}: " + ", ".join(map(self.fmt.get_extend_decl, d.extends))
            if d.extends
            else d.name
        )
        prologue = f"{iface_kw} {full_decl} {{"
        epilogue = f"}}"

        if d.methods:
            with self.out.indented(prologue, epilogue):
                for f in d.methods:
                    f.accept(self)
        else:
            self.out.writeln(prologue + epilogue)

    @override
    def visit_package(self, p: "PackageDecl"):
        self.out.writeln(f"// {p.name}")
        self.write_pkg_attr(p)
        for d in p.pkg_imports:
            d.accept(self)
        for d in p.decl_imports:
            d.accept(self)
        for d in p.declarations:
            d.accept(self)

    @override
    def visit_package_group(self, g: "PackageGroup"):
        for i, p in enumerate(g.all_packages):
            if i != 0:
                self.out.newline()
            p.accept(self)