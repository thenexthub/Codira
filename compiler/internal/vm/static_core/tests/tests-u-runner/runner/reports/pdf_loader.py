#!/usr/bin/env python3
# -*- coding: utf-8 -*-
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
#

from __future__ import annotations
from pathlib import Path
from typing import List, Union, Optional, Dict
from datetime import datetime
from PyPDF2 import PdfReader, DocumentInformation
from PyPDF2.generic import Destination
from PyPDF2.types import OutlineType
from runner.reports.spec_node import SpecNode


class PdfLoader:
    def __init__(self, fpath: Path, config: Optional[Dict]):
        self.spec_file = fpath
        self.config: Optional[Dict] = config
        self.spec: SpecNode = SpecNode("SUMMARY", "", "", None)  # root node
        self.creation_date: Optional[datetime] = None

    @staticmethod
    def __is_destination(field: Union[List, Destination]) -> bool:
        return str(type(field)).find("Destination") > 0

    def parse(self) -> PdfLoader:
        with open(self.spec_file, "rb") as stream:
            reader: PdfReader = PdfReader(stream)
            metadata: Optional[DocumentInformation] = reader.metadata
            self.creation_date = metadata.creation_date if metadata else None
            fields: OutlineType = reader.outline
            self.__read_list(fields, self.spec)
            return self

    def get_root_node(self) -> SpecNode:
        return self.spec

    def get_creation_date(self) -> str:
        return str(self.creation_date.date()) if self.creation_date else "unknown"

    def __read_dest(self, item: Destination, counter: int, parent: SpecNode) -> SpecNode:
        new_prefix: str = str(counter) if parent.prefix == "" else f"{parent.prefix}.{counter}"
        title: str = str(item.title) if item and item.title else ""
        node_config = self.config.get(new_prefix) if self.config is not None else None
        status: str = node_config.get("status", "") if node_config is not None else ""
        node = SpecNode(title, new_prefix, status, parent)
        return node

    def __read_list(self, items: List, parent: SpecNode) -> None:
        counter = 0
        node = parent
        for item in items:
            if PdfLoader.__is_destination(item):
                counter += 1
                node = self.__read_dest(item, counter, parent)
            else:
                self.__read_list(item, node)
