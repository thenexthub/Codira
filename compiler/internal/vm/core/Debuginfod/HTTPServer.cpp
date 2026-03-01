//===-- toolchain/Debuginfod/HTTPServer.cpp - HTTP server library -----*- C++-*-===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//
///
/// \file
///
/// This file defines the methods of the HTTPServer class and the streamFile
/// function.
///
//===----------------------------------------------------------------------===//

#include "vm/core/Debuginfod/HTTPServer.h"
#include "vm/core/ADT/StringExtras.h"
#include "vm/core/ADT/StringRef.h"
#include "vm/core/Support/Errc.h"
#include "vm/core/Support/Error.h"
#include "vm/core/Support/FileSystem.h"
#include "vm/core/Support/MemoryBuffer.h"
#include "vm/core/Support/Regex.h"

#ifdef LLVM_ENABLE_HTTPLIB
#include "httplib.h"
#endif

using namespace vm::core;

char HTTPServerError::ID = 0;

HTTPServerError::HTTPServerError(const Twine &Msg) : Msg(Msg.str()) {}

void HTTPServerError::log(raw_ostream &OS) const { OS << Msg; }

bool toolchain::streamFile(HTTPServerRequest &Request, StringRef FilePath) {
  Expected<sys::fs::file_t> FDOrErr = sys::fs::openNativeFileForRead(FilePath);
  if (Error Err = FDOrErr.takeError()) {
    consumeError(std::move(Err));
    Request.setResponse({404u, "text/plain", "Could not open file to read.\n"});
    return false;
  }
  ErrorOr<std::unique_ptr<MemoryBuffer>> MBOrErr =
      MemoryBuffer::getOpenFile(*FDOrErr, FilePath,
                                /*FileSize=*/-1,
                                /*RequiresNullTerminator=*/false);
  sys::fs::closeFile(*FDOrErr);
  if (Error Err = errorCodeToError(MBOrErr.getError())) {
    consumeError(std::move(Err));
    Request.setResponse({404u, "text/plain", "Could not memory-map file.\n"});
    return false;
  }
  // Lambdas are copied on conversion to std::function, preventing use of
  // smart pointers.
  MemoryBuffer *MB = MBOrErr->release();
  Request.setResponse({200u, "application/octet-stream", MB->getBufferSize(),
                       [=](size_t Offset, size_t Length) -> StringRef {
                         return MB->getBuffer().substr(Offset, Length);
                       },
                       [=](bool Success) { delete MB; }});
  return true;
}

#ifdef LLVM_ENABLE_HTTPLIB

bool HTTPServer::isAvailable() { return true; }

HTTPServer::HTTPServer() { Server = std::make_unique<httplib::Server>(); }

HTTPServer::~HTTPServer() { stop(); }

static void expandUrlPathMatches(const std::smatch &Matches,
                                 HTTPServerRequest &Request) {
  bool UrlPathSet = false;
  for (const auto &it : Matches) {
    if (UrlPathSet)
      Request.UrlPathMatches.push_back(it);
    else {
      Request.UrlPath = it;
      UrlPathSet = true;
    }
  }
}

HTTPServerRequest::HTTPServerRequest(const httplib::Request &HTTPLibRequest,
                                     httplib::Response &HTTPLibResponse)
    : HTTPLibResponse(HTTPLibResponse) {
  expandUrlPathMatches(HTTPLibRequest.matches, *this);
}

void HTTPServerRequest::setResponse(HTTPResponse Response) {
  HTTPLibResponse.set_content(Response.Body.begin(), Response.Body.size(),
                              Response.ContentType);
  HTTPLibResponse.status = Response.Code;
}

void HTTPServerRequest::setResponse(StreamingHTTPResponse Response) {
  HTTPLibResponse.set_content_provider(
      Response.ContentLength, Response.ContentType,
      [=](size_t Offset, size_t Length, httplib::DataSink &Sink) {
        if (Offset < Response.ContentLength) {
          StringRef Chunk = Response.Provider(Offset, Length);
          Sink.write(Chunk.begin(), Chunk.size());
        }
        return true;
      },
      [=](bool Success) { Response.CompletionHandler(Success); });

  HTTPLibResponse.status = Response.Code;
}

Error HTTPServer::get(StringRef UrlPathPattern, HTTPRequestHandler Handler) {
  std::string ErrorMessage;
  if (!Regex(UrlPathPattern).isValid(ErrorMessage))
    return createStringError(errc::argument_out_of_domain, ErrorMessage);
  Server->Get(std::string(UrlPathPattern),
              [Handler](const httplib::Request &HTTPLibRequest,
                        httplib::Response &HTTPLibResponse) {
                HTTPServerRequest Request(HTTPLibRequest, HTTPLibResponse);
                Handler(Request);
              });
  return Error::success();
}

Error HTTPServer::bind(unsigned ListenPort, const char *HostInterface) {
  if (!Server->bind_to_port(HostInterface, ListenPort))
    return createStringError(errc::io_error,
                             "Could not assign requested address.");
  Port = ListenPort;
  return Error::success();
}

Expected<unsigned> HTTPServer::bind(const char *HostInterface) {
  int ListenPort = Server->bind_to_any_port(HostInterface);
  if (ListenPort < 0)
    return createStringError(errc::io_error,
                             "Could not assign any port on requested address.");
  return Port = ListenPort;
}

Error HTTPServer::listen() {
  if (!Port)
    return createStringError(errc::io_error,
                             "Cannot listen without first binding to a port.");
  if (!Server->listen_after_bind())
    return createStringError(
        errc::io_error,
        "An unknown error occurred when cpp-httplib attempted to listen.");
  return Error::success();
}

void HTTPServer::stop() {
  Server->stop();
  Port = 0;
}

#else

// TODO: Implement barebones standalone HTTP server implementation.
bool HTTPServer::isAvailable() { return false; }

HTTPServer::HTTPServer() = default;

HTTPServer::~HTTPServer() = default;

void HTTPServerRequest::setResponse(HTTPResponse Response) {
  llvm_unreachable("no httplib");
}

void HTTPServerRequest::setResponse(StreamingHTTPResponse Response) {
  llvm_unreachable("no httplib");
}

Error HTTPServer::get(StringRef UrlPathPattern, HTTPRequestHandler Handler) {
  // TODO(https://github.com/toolchain/toolchain-project/issues/63873) We would ideally
  // return an error as well but that's going to require refactoring of error
  // handling in DebuginfodServer.
  return Error::success();
}

Error HTTPServer::bind(unsigned ListenPort, const char *HostInterface) {
  return make_error<HTTPServerError>("no httplib");
}

Expected<unsigned> HTTPServer::bind(const char *HostInterface) {
  return make_error<HTTPServerError>("no httplib");
}

Error HTTPServer::listen() {
  return make_error<HTTPServerError>("no httplib");
}

void HTTPServer::stop() {
  llvm_unreachable("no httplib");
}

#endif // LLVM_ENABLE_HTTPLIB
