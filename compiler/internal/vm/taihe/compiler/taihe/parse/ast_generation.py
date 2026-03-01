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

from contextlib import suppress
from typing import Any, cast

from antlr4.CommonTokenStream import CommonTokenStream
from antlr4.error.ErrorListener import ErrorListener
from antlr4.InputStream import InputStream
from antlr4.ParserRuleContext import ParserRuleContext
from antlr4.Token import Token
from antlr4.tree.Tree import ErrorNodeImpl, TerminalNodeImpl, Tree

from taihe.parse.antlr.TaiheAST import TaiheAST
from taihe.parse.antlr.TaiheLexer import TaiheLexer
from taihe.parse.antlr.TaiheParser import TaiheParser
from taihe.utils.diagnostics import DiagnosticsManager
from taihe.utils.exceptions import IDLSyntaxError
from taihe.utils.sources import (
    SourceBase,
    SourceLocation,
    TextPosition,
    TextSpan,
)


class SourceCodeLocator:
    def __init__(self, source: SourceBase) -> None:
        self.source = source
        self._cache: dict[Tree | Token, TextSpan | None] = {}

    def get_span(self, ctx: Tree | Token) -> TextSpan | None:
        if ctx in self._cache:
            return self._cache[ctx]
        if isinstance(ctx, Token):
            text = cast(str, ctx.text).splitlines()  # type: ignore
            row = cast(int, ctx.line)
            col = cast(int, ctx.column)
            row_offset = len(text) - 1
            col_offset = len(text[-1])
            beg = TextPosition(
                row,
                col + 1,
            )
            end = TextPosition(
                row + row_offset,
                col + col_offset if row_offset == 0 else col_offset,
            )
            span = TextSpan(beg, end)
        elif isinstance(ctx, TerminalNodeImpl | ErrorNodeImpl):
            span = self.get_span(ctx.symbol)
        elif isinstance(ctx, ParserRuleContext):
            span = None
            children = cast(list[Tree | Token], ctx.children) or []
            for child in children:
                part = self.get_span(child)
                if part is None:
                    continue
                if span is None:
                    span = part
                else:
                    span = span | part
        else:
            raise TypeError(f"Unsupported type {type(ctx)} for ASTRecorder.get_pos()")
        return self._cache.setdefault(ctx, span)

    def get_loc(self, ctx: Tree | Token) -> SourceLocation:
        return SourceLocation(self.source, self.get_span(ctx))


class TaiheErrorListener(ErrorListener):
    def __init__(
        self,
        locator: SourceCodeLocator,
        dm: DiagnosticsManager,
    ) -> None:
        super().__init__()
        self.locator = locator
        self.dm = dm

    def syntaxError(
        self,
        recognizer: Any,
        offendingSymbol: Token,
        line: int,
        column: int,
        msg: str,
        e: Any,
    ):
        self.dm.emit(
            IDLSyntaxError(
                cast(str, offendingSymbol.text),  # type: ignore
                loc=self.locator.get_loc(offendingSymbol),
            )
        )


def is_qualified(real_kind: str, node_kind: str):
    ctx_kind = node_kind + "Context"
    ctx_type = getattr(TaiheParser, ctx_kind)
    sub_kind = real_kind + "Context"
    sub_type = getattr(TaiheParser, sub_kind)
    subclasses = ctx_type.__subclasses__()
    if subclasses:
        return sub_type in subclasses
    else:
        return real_kind == node_kind


class TaiheASTConverter:
    def __init__(self, locator: SourceCodeLocator):
        self.locator = locator

    def visit(self, node_kind: str, ctx: Any) -> Any:
        if node_kind.endswith("Lst"):
            lst: list[Any] = []
            for sub in ctx:
                with suppress(Exception):
                    lst.append(self.visit(node_kind[:-3], sub))
            return lst
        if node_kind.endswith("Opt"):
            opt_node: Any | None = None
            if ctx is not None:
                with suppress(Exception):
                    opt_node = self.visit(node_kind[:-3], ctx)
            return opt_node
        loc = self.locator.get_loc(ctx)
        if node_kind == "TOKEN":
            return TaiheAST.TOKEN(loc=loc, text=ctx.text)
        kwargs = {}
        for attr_full_name, attr_ctx in ctx.__dict__.items():
            if attr_full_name[0].isupper():
                attr_kind_name, attr_name = attr_full_name.split("_", 1)
                kwargs[attr_name] = self.visit(attr_kind_name, attr_ctx)
        real_kind = ctx.__class__.__name__[:-7]  # Remove the trailing "Context"
        pass
        return getattr(TaiheAST, real_kind)(loc=loc, **kwargs)


def generate_ast(source: SourceBase, dm: DiagnosticsManager) -> TaiheAST.Spec:
    input_stream = InputStream(source.read())

    lexer = TaiheLexer(input_stream)
    token_stream = CommonTokenStream(lexer)

    locator = SourceCodeLocator(source)
    error_listener = TaiheErrorListener(locator, dm)
    converter = TaiheASTConverter(locator)

    parser = TaiheParser(token_stream)
    parser.removeErrorListeners()
    parser.addErrorListener(error_listener)  # type: ignore
    tree = parser.spec()
    return converter.visit("Spec", tree)