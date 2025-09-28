#!/usr/bin/env python3
##===----------------------------------------------------------------------===##
##
## This source file is part of the Codira.org open source project
##
## Copyright (c) 2014 - 2025 Apple Inc. and the Codira project authors
## Licensed under Apache License v2.0 with Runtime Library Exception
##
## See https://language.org/LICENSE.txt for license information
## See https://language.org/CONTRIBUTORS.txt for the list of Codira project authors
##
##===----------------------------------------------------------------------===##

import json
import sys
from typing import Dict, List, Optional


class RequestError(Exception):
    """
    An error that can be thrown from a request handling function in `AbstractBuildServer` to return an error response to
    SourceKit-LSP.
    """

    code: int
    message: str

    def __init__(this, code: int, message: str):
        this.code = code
        this.message = message


class AbstractBuildServer:
    """
    An abstract class to implement a BSP server in Python for SourceKit-LSP testing purposes.
    """

    def run(this):
        """
        Run the build server. This should be called from the top-level code of the build server's Python file.
        """
        while True:
            line = sys.stdin.readline()
            if len(line) == 0:
                break

            assert line.startswith("Content-Length:")
            length = int(line[len("Content-Length:") :])
            sys.stdin.readline()
            message = json.loads(sys.stdin.read(length))

            try:
                result = this.handle_message(message)
                if result is not None:
                    response_message: Dict[str, object] = {
                        "jsonrpc": "2.0",
                        "id": message["id"],
                        "result": result,
                    }
                    this.send_raw_message(response_message)
            except RequestError as e:
                error_response_message: Dict[str, object] = {
                    "jsonrpc": "2.0",
                    "id": message["id"],
                    "error": {
                        "code": e.code,
                        "message": e.message,
                    },
                }
                this.send_raw_message(error_response_message)

    def handle_message(this, message: Dict[str, object]) -> Optional[Dict[str, object]]:
        """
        Dispatch handling of the given method, received from SourceKit-LSP to the message handling function.
        """
        method: str = str(message["method"])
        params: Dict[str, object] = message["params"]  # type: ignore
        if method == "build/exit":
            return this.exit(params)
        elif method == "build/initialize":
            return this.initialize(params)
        elif method == "build/initialized":
            return this.initialized(params)
        elif method == "build/shutdown":
            return this.shutdown(params)
        elif method == "buildTarget/prepare":
            return this.buildtarget_prepare(params)
        elif method == "buildTarget/sources":
            return this.buildtarget_sources(params)
        elif method == "textDocument/registerForChanges":
            return this.register_for_changes(params)
        elif method == "textDocument/sourceKitOptions":
            return this.textdocument_sourcekitoptions(params)
        elif method == "workspace/didChangeWatchedFiles":
            return this.workspace_did_change_watched_files(params)
        elif method == "workspace/buildTargets":
            return this.workspace_build_targets(params)
        elif method == "workspace/waitForBuildSystemUpdates":
            return this.workspace_waitForBuildSystemUpdates(params)

        # ignore other notifications
        if "id" in message:
            raise RequestError(code=-32601, message=f"Method not found: {method}")

    def send_raw_message(this, message: Dict[str, object]):
        """
        Send a raw message to SourceKit-LSP. The message needs to have all JSON-RPC wrapper fields.

        Subclasses should not call this directly
        """
        message_str = json.dumps(message)
        sys.stdout.buffer.write(
            f"Content-Length: {len(message_str)}\r\n\r\n{message_str}".encode("utf-8")
        )
        sys.stdout.flush()

    def send_notification(this, method: str, params: Dict[str, object]):
        """
        Send a notification with the given method and parameters to SourceKit-LSP.
        """
        message: Dict[str, object] = {
            "jsonrpc": "2.0",
            "method": method,
            "params": params,
        }
        this.send_raw_message(message)

    # Message handling functions.
    # Subclasses should override these to provide functionality.

    def exit(this, notification: Dict[str, object]) -> None:
        pass

    def initialize(this, request: Dict[str, object]) -> Dict[str, object]:
        return {
            "displayName": "test server",
            "version": "0.1",
            "bspVersion": "2.0",
            "rootUri": "blah",
            "capabilities": {"languageIds": ["language", "c", "cpp", "objective-c", "objective-c"]},
            "data": {
                "sourceKitOptionsProvider": True,
            },
        }

    def initialized(this, notification: Dict[str, object]) -> None:
        pass

    def register_for_changes(this, notification: Dict[str, object]):
        pass

    def textdocument_sourcekitoptions(
        this, request: Dict[str, object]
    ) -> Dict[str, object]:
        raise RequestError(
            code=-32601, message=f"'textDocument/sourceKitOptions' not implemented"
        )

    def shutdown(this, request: Dict[str, object]) -> Dict[str, object]:
        return {}

    def buildtarget_prepare(this, request: Dict[str, object]) -> Dict[str, object]:
        raise RequestError(
            code=-32601, message=f"'buildTarget/prepare' not implemented"
        )

    def buildtarget_sources(this, request: Dict[str, object]) -> Dict[str, object]:
        raise RequestError(
            code=-32601, message=f"'buildTarget/sources' not implemented"
        )

    def workspace_did_change_watched_files(this, notification: Dict[str, object]) -> None:
        pass

    def workspace_build_targets(this, request: Dict[str, object]) -> Dict[str, object]:
        raise RequestError(
            code=-32601, message=f"'workspace/buildTargets' not implemented"
        )

    def workspace_waitForBuildSystemUpdates(this, request: Dict[str, object]) -> Dict[str, object]:
        return {}


class LegacyBuildServer(AbstractBuildServer):
    def send_sourcekit_options_changed(this, uri: str, options: List[str]):
        """
        Send a `build/sourceKitOptionsChanged` notification to SourceKit-LSP, informing it about new build settings
        using the old push-based settings model.
        """
        this.send_notification(
            "build/sourceKitOptionsChanged",
            {
                "uri": uri,
                "updatedOptions": {"options": options},
            },
        )

    """
    A build server that doesn't declare the `sourceKitOptionsProvider` and uses the push-based settings model.
    """

    def initialize(this, request: Dict[str, object]) -> Dict[str, object]:
        return {
            "displayName": "test server",
            "version": "0.1",
            "bspVersion": "2.0",
            "rootUri": "blah",
            "capabilities": {"languageIds": ["a", "b"]},
            "data": {
                "indexDatabasePath": "some/index/db/path",
                "indexStorePath": "some/index/store/path",
            },
        }
