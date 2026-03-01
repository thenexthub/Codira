#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright (c) 2024 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

from typing import List, Tuple, Union

from h11._events import Request, _ResponseBase

_response_ctor = _ResponseBase.__init__
_request_ctor = Request.__init__


def overwrite() -> None:
    """
    The commit https://github.com/python-hyper/wsproto/commit/c5e53b67bbb43b410976d070eeb195fa0ef56076
    has not been released yet.
    """

    def create_init(ctor):
        def init(self, **kwargs) -> None:
            def fix_websocket(
                item: Union[Tuple[bytes, bytes], Tuple[str, str]],
            ) -> Union[Tuple[bytes, bytes], Tuple[str, str]]:
                match item:
                    case (b"Upgrade", b"WebSocket"):
                        return (b"Upgrade", b"websocket")
                return item

            headers: Union[List[Tuple[bytes, bytes]], List[Tuple[str, str]]] = kwargs.get("headers", None)
            if headers is not None:
                kwargs["headers"] = [fix_websocket(h) for h in headers]
            ctor(self, **kwargs)

        return init

    setattr(_ResponseBase, "__init__", create_init(_response_ctor))
    setattr(Request, "__init__", create_init(_request_ctor))
