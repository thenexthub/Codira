#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Copyright (c) 2026 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

Description: Python Protocol Domain Interfaces
"""

import sys
from pathlib import Path

sys.path.append(str(Path(__file__).parent.parent))  # add aw path to sys.path

from customized_types import ProtocolType


class ProtocolImpl(object):

    def __init__(self, id_generator, websocket):
        self.id_generator = id_generator
        self.class_name = self.__class__.__name__
        self.domain = self.class_name[:-4]
        # dispatch_table maps protocol strings to a tuple of (handler, protocol_type)
        #   key: protocol name (str)
        #   value: (handler_function, ProtocolType)
        #   handler_function: method to handle the protocol
        #   ProtocolType: send or recv, indicating protocol direction
        self.dispatch_table = {}
        self.websocket = websocket

    async def send(self, protocol_name, connection, params=None, is_hybrid=False, counts=1, sessionId=""):
        protocol = self._check_and_parse_protocol(protocol_name)
        if self.dispatch_table.get(protocol) is not None:
            if self.dispatch_table.get(protocol)[1] != ProtocolType.send:
                raise AssertionError("{} send ProtocolType inconsistent: Protocol {}, calling {}, should be {}"
                                     .format(self.class_name, protocol_name, "send",
                                             self.dispatch_table.get(protocol)[1]))
            message_id = next(self.id_generator)
            if is_hybrid:
                return await self.dispatch_table.get(protocol)[0](message_id, connection, params, is_hybrid, counts, sessionId)
            else:
                return await self.dispatch_table.get(protocol)[0](message_id, connection, params)


    async def recv(self, protocol_name, connection, params=None, is_hybrid=False, counts=1):
        protocol = self._check_and_parse_protocol(protocol_name)
        if self.dispatch_table.get(protocol) is not None:
            if self.dispatch_table.get(protocol)[1] != ProtocolType.recv:
                raise AssertionError("{} recv ProtocolType inconsistent: Protocol {}, calling {}, should be {}"
                                     .format(self.class_name, protocol_name, "recv",
                                             self.dispatch_table.get(protocol)[1]))
            if is_hybrid:
                return await self.dispatch_table.get(protocol)[0](connection, params, is_hybrid, counts)
            else:
                return await self.dispatch_table.get(protocol)[0](connection, params)


    def _check_and_parse_protocol(self, protocol_name):
        res = protocol_name.split('.')
        if len(res) != 2:
            raise AssertionError("{} parse protocol name error: protocol_name {} is invalid"
                                 .format(self.class_name, protocol_name))
        domain, protocol = res[0], res[1]
        if domain != self.domain:
            raise AssertionError("{} parse protocol name error: protocol_name {} has the wrong domain"
                                 .format(self.class_name, protocol_name))
        if protocol not in self.dispatch_table:
            raise AssertionError("{} parse protocol name error: protocol_name {} has not been defined"
                                 .format(self.class_name, protocol_name))
        return protocol
