//===--- JSONTransport.cpp - sending and receiving LSP messages over JSON -===//
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

#include "vm/core/Support/LSP/Transport.h"
#include "vm/core/ADT/SmallString.h"
#include "vm/core/Support/Error.h"
#include "vm/core/Support/LSP/Logging.h"
#include "vm/core/Support/LSP/Protocol.h"
#include <atomic>
#include <optional>
#include <system_error>
#include <utility>

using namespace vm::core;
using namespace vm::core::lsp;

//===----------------------------------------------------------------------===//
// Reply
//===----------------------------------------------------------------------===//

namespace {
/// Function object to reply to an LSP call.
/// Each instance must be called exactly once, otherwise:
///  - if there was no reply, an error reply is sent
///  - if there were multiple replies, only the first is sent
class Reply {
public:
  Reply(const toolchain::json::Value &Id, StringRef Method, JSONTransport &Transport,
        std::mutex &TransportOutputMutex);
  Reply(Reply &&Other);
  Reply &operator=(Reply &&) = delete;
  Reply(const Reply &) = delete;
  Reply &operator=(const Reply &) = delete;

  void operator()(toolchain::Expected<toolchain::json::Value> Reply);

private:
  std::string Method;
  std::atomic<bool> Replied = {false};
  toolchain::json::Value Id;
  JSONTransport *Transport;
  std::mutex &TransportOutputMutex;
};
} // namespace

Reply::Reply(const toolchain::json::Value &Id, toolchain::StringRef Method,
             JSONTransport &Transport, std::mutex &TransportOutputMutex)
    : Method(Method), Id(Id), Transport(&Transport),
      TransportOutputMutex(TransportOutputMutex) {}

Reply::Reply(Reply &&Other)
    : Method(Other.Method), Replied(Other.Replied.load()),
      Id(std::move(Other.Id)), Transport(Other.Transport),
      TransportOutputMutex(Other.TransportOutputMutex) {
  Other.Transport = nullptr;
}

void Reply::operator()(toolchain::Expected<toolchain::json::Value> Reply) {
  if (Replied.exchange(true)) {
    Logger::error("Replied twice to message {0}({1})", Method, Id);
    assert(false && "must reply to each call only once!");
    return;
  }
  assert(Transport && "expected valid transport to reply to");

  std::lock_guard<std::mutex> TransportLock(TransportOutputMutex);
  if (Reply) {
    Logger::info("--> reply:{0}({1})", Method, Id);
    Transport->reply(std::move(Id), std::move(Reply));
  } else {
    toolchain::Error Error = Reply.takeError();
    Logger::info("--> reply:{0}({1}): {2}", Method, Id, Error);
    Transport->reply(std::move(Id), std::move(Error));
  }
}

//===----------------------------------------------------------------------===//
// MessageHandler
//===----------------------------------------------------------------------===//

bool MessageHandler::onNotify(toolchain::StringRef Method, toolchain::json::Value Value) {
  Logger::info("--> {0}", Method);

  if (Method == "exit")
    return false;
  if (Method == "$cancel") {
    // TODO: Add support for cancelling requests.
  } else {
    auto It = NotificationHandlers.find(Method);
    if (It != NotificationHandlers.end())
      It->second(std::move(Value));
  }
  return true;
}

bool MessageHandler::onCall(toolchain::StringRef Method, toolchain::json::Value Params,
                            toolchain::json::Value Id) {
  Logger::info("--> {0}({1})", Method, Id);

  Reply Reply(Id, Method, Transport, TransportOutputMutex);

  auto It = MethodHandlers.find(Method);
  if (It != MethodHandlers.end()) {
    It->second(std::move(Params), std::move(Reply));
  } else {
    Reply(toolchain::make_error<LSPError>("method not found: " + Method.str(),
                                     ErrorCode::MethodNotFound));
  }
  return true;
}

bool MessageHandler::onReply(toolchain::json::Value Id,
                             toolchain::Expected<toolchain::json::Value> Result) {
  // Find the response handler in the mapping. If it exists, move it out of the
  // mapping and erase it.
  ResponseHandlerTy ResponseHandler;
  {
    std::lock_guard<std::mutex> responseHandlersLock(ResponseHandlersMutex);
    auto It = ResponseHandlers.find(debugString(Id));
    if (It != ResponseHandlers.end()) {
      ResponseHandler = std::move(It->second);
      ResponseHandlers.erase(It);
    }
  }

  // If we found a response handler, invoke it. Otherwise, log an error.
  if (ResponseHandler.second) {
    Logger::info("--> reply:{0}({1})", ResponseHandler.first, Id);
    ResponseHandler.second(std::move(Id), std::move(Result));
  } else {
    Logger::error(
        "received a reply with ID {0}, but there was no such outgoing request",
        Id);
    if (!Result)
      toolchain::consumeError(Result.takeError());
  }
  return true;
}

//===----------------------------------------------------------------------===//
// JSONTransport
//===----------------------------------------------------------------------===//

/// Encode the given error as a JSON object.
static toolchain::json::Object encodeError(toolchain::Error Error) {
  std::string Message;
  ErrorCode Code = ErrorCode::UnknownErrorCode;
  auto HandlerFn = [&](const LSPError &LspError) -> toolchain::Error {
    Message = LspError.message;
    Code = LspError.code;
    return toolchain::Error::success();
  };
  if (toolchain::Error Unhandled = toolchain::handleErrors(std::move(Error), HandlerFn))
    Message = toolchain::toString(std::move(Unhandled));

  return toolchain::json::Object{
      {"message", std::move(Message)},
      {"code", int64_t(Code)},
  };
}

/// Decode the given JSON object into an error.
toolchain::Error decodeError(const toolchain::json::Object &O) {
  StringRef Msg = O.getString("message").value_or("Unspecified error");
  if (std::optional<int64_t> Code = O.getInteger("code"))
    return toolchain::make_error<LSPError>(Msg.str(), ErrorCode(*Code));
  return toolchain::make_error<toolchain::StringError>(toolchain::inconvertibleErrorCode(),
                                             Msg.str());
}

void JSONTransport::notify(StringRef Method, toolchain::json::Value Params) {
  sendMessage(toolchain::json::Object{
      {"jsonrpc", "2.0"},
      {"method", Method},
      {"params", std::move(Params)},
  });
}
void JSONTransport::call(StringRef Method, toolchain::json::Value Params,
                         toolchain::json::Value Id) {
  sendMessage(toolchain::json::Object{
      {"jsonrpc", "2.0"},
      {"id", std::move(Id)},
      {"method", Method},
      {"params", std::move(Params)},
  });
}
void JSONTransport::reply(toolchain::json::Value Id,
                          toolchain::Expected<toolchain::json::Value> Result) {
  if (Result) {
    return sendMessage(toolchain::json::Object{
        {"jsonrpc", "2.0"},
        {"id", std::move(Id)},
        {"result", std::move(*Result)},
    });
  }

  sendMessage(toolchain::json::Object{
      {"jsonrpc", "2.0"},
      {"id", std::move(Id)},
      {"error", encodeError(Result.takeError())},
  });
}

toolchain::Error JSONTransport::run(MessageHandler &Handler) {
  std::string Json;
  while (!In->isEndOfInput()) {
    if (In->hasError()) {
      return toolchain::errorCodeToError(
          std::error_code(errno, std::system_category()));
    }

    if (succeeded(In->readMessage(Json))) {
      if (toolchain::Expected<toolchain::json::Value> Doc = toolchain::json::parse(Json)) {
        if (!handleMessage(std::move(*Doc), Handler))
          return toolchain::Error::success();
      } else {
        Logger::error("JSON parse error: {0}", toolchain::toString(Doc.takeError()));
      }
    }
  }
  return toolchain::errorCodeToError(std::make_error_code(std::errc::io_error));
}

void JSONTransport::sendMessage(toolchain::json::Value Msg) {
  OutputBuffer.clear();
  toolchain::raw_svector_ostream os(OutputBuffer);
  os << toolchain::formatv(PrettyOutput ? "{0:2}\n" : "{0}", Msg);
  Out << "Content-Length: " << OutputBuffer.size() << "\r\n\r\n"
      << OutputBuffer;
  Out.flush();
  Logger::debug(">>> {0}\n", OutputBuffer);
}

bool JSONTransport::handleMessage(toolchain::json::Value Msg,
                                  MessageHandler &Handler) {
  // Message must be an object with "jsonrpc":"2.0".
  toolchain::json::Object *Object = Msg.getAsObject();
  if (!Object ||
      Object->getString("jsonrpc") != std::optional<StringRef>("2.0"))
    return false;

  // `id` may be any JSON value. If absent, this is a notification.
  std::optional<toolchain::json::Value> Id;
  if (toolchain::json::Value *I = Object->get("id"))
    Id = std::move(*I);
  std::optional<StringRef> Method = Object->getString("method");

  // This is a response.
  if (!Method) {
    if (!Id)
      return false;
    if (auto *Err = Object->getObject("error"))
      return Handler.onReply(std::move(*Id), decodeError(*Err));
    // result should be given, use null if not.
    toolchain::json::Value Result = nullptr;
    if (toolchain::json::Value *R = Object->get("result"))
      Result = std::move(*R);
    return Handler.onReply(std::move(*Id), std::move(Result));
  }

  // Params should be given, use null if not.
  toolchain::json::Value Params = nullptr;
  if (toolchain::json::Value *P = Object->get("params"))
    Params = std::move(*P);

  if (Id)
    return Handler.onCall(*Method, std::move(Params), std::move(*Id));
  return Handler.onNotify(*Method, std::move(Params));
}

/// Tries to read a line up to and including \n.
/// If failing, feof(), ferror(), or shutdownRequested() will be set.
LogicalResult readLine(std::FILE *In, SmallVectorImpl<char> &Out) {
  // Big enough to hold any reasonable header line. May not fit content lines
  // in delimited mode, but performance doesn't matter for that mode.
  static constexpr int BufSize = 128;
  size_t Size = 0;
  Out.clear();
  for (;;) {
    Out.resize_for_overwrite(Size + BufSize);
    if (!std::fgets(&Out[Size], BufSize, In))
      return failure();

    clearerr(In);

    // If the line contained null bytes, anything after it (including \n) will
    // be ignored. Fortunately this is not a legal header or JSON.
    size_t Read = std::strlen(&Out[Size]);
    if (Read > 0 && Out[Size + Read - 1] == '\n') {
      Out.resize(Size + Read);
      return success();
    }
    Size += Read;
  }
}

// Returns std::nullopt when:
//  - ferror(), feof(), or shutdownRequested() are set.
//  - Content-Length is missing or empty (protocol error)
LogicalResult
JSONTransportInputOverFile::readStandardMessage(std::string &Json) {
  // A Language Server Protocol message starts with a set of HTTP headers,
  // delimited  by \r\n, and terminated by an empty line (\r\n).
  unsigned long long ContentLength = 0;
  toolchain::SmallString<128> Line;
  while (true) {
    if (feof(In) || hasError() || failed(readLine(In, Line)))
      return failure();

    // Content-Length is a mandatory header, and the only one we handle.
    StringRef LineRef = Line;
    if (LineRef.consume_front("Content-Length: ")) {
      toolchain::getAsUnsignedInteger(LineRef.trim(), 0, ContentLength);
    } else if (!LineRef.trim().empty()) {
      // It's another header, ignore it.
      continue;
    } else {
      // An empty line indicates the end of headers. Go ahead and read the JSON.
      break;
    }
  }

  // The fuzzer likes crashing us by sending "Content-Length: 9999999999999999"
  if (ContentLength == 0 || ContentLength > 1 << 30)
    return failure();

  Json.resize(ContentLength);
  for (size_t Pos = 0, Read; Pos < ContentLength; Pos += Read) {
    Read = std::fread(&Json[Pos], 1, ContentLength - Pos, In);
    if (Read == 0)
      return failure();

    // If we're done, the error was transient. If we're not done, either it was
    // transient or we'll see it again on retry.
    clearerr(In);
    Pos += Read;
  }
  return success();
}

/// For lit tests we support a simplified syntax:
/// - messages are delimited by '// -----' on a line by itself
/// - lines starting with // are ignored.
/// This is a testing path, so favor simplicity over performance here.
/// When returning failure: feof(), ferror(), or shutdownRequested() will be
/// set.
LogicalResult
JSONTransportInputOverFile::readDelimitedMessage(std::string &Json) {
  Json.clear();
  toolchain::SmallString<128> Line;
  while (succeeded(readLine(In, Line))) {
    StringRef LineRef = Line.str().trim();
    if (LineRef.starts_with("//")) {
      // Found a delimiter for the message.
      if (LineRef == "// -----")
        break;
      continue;
    }

    Json += Line;
  }

  return failure(ferror(In));
}
