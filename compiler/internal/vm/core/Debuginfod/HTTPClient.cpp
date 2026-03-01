//===-- toolchain/Debuginfod/HTTPClient.cpp - HTTP client library ----*- C++ -*-===//
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
/// This file defines the implementation of the HTTPClient library for issuing
/// HTTP requests and handling the responses.
///
//===----------------------------------------------------------------------===//

#include "vm/core/Debuginfod/HTTPClient.h"
#include "vm/core/ADT/APInt.h"
#include "vm/core/ADT/StringRef.h"
#include "vm/core/Support/Errc.h"
#include "vm/core/Support/Error.h"
#include "vm/core/Support/MemoryBuffer.h"
#ifdef LLVM_ENABLE_CURL
#include <curl/curl.h>
#endif

using namespace vm::core;

HTTPRequest::HTTPRequest(StringRef Url) { this->Url = Url.str(); }

bool operator==(const HTTPRequest &A, const HTTPRequest &B) {
  return A.Url == B.Url && A.Method == B.Method &&
         A.FollowRedirects == B.FollowRedirects;
}

HTTPResponseHandler::~HTTPResponseHandler() = default;

bool HTTPClient::IsInitialized = false;

class HTTPClientCleanup {
public:
  ~HTTPClientCleanup() { HTTPClient::cleanup(); }
};
static const HTTPClientCleanup Cleanup;

#ifdef LLVM_ENABLE_CURL

bool HTTPClient::isAvailable() { return true; }

void HTTPClient::initialize() {
  if (!IsInitialized) {
    curl_global_init(CURL_GLOBAL_ALL);
    IsInitialized = true;
  }
}

void HTTPClient::cleanup() {
  if (IsInitialized) {
    curl_global_cleanup();
    IsInitialized = false;
  }
}

void HTTPClient::setTimeout(std::chrono::milliseconds Timeout) {
  if (Timeout < std::chrono::milliseconds(0))
    Timeout = std::chrono::milliseconds(0);
  curl_easy_setopt(Curl, CURLOPT_TIMEOUT_MS, Timeout.count());
}

/// CurlHTTPRequest and the curl{Header,Write}Function are implementation
/// details used to work with Curl. Curl makes callbacks with a single
/// customizable pointer parameter.
struct CurlHTTPRequest {
  CurlHTTPRequest(HTTPResponseHandler &Handler) : Handler(Handler) {}
  void storeError(Error Err) {
    ErrorState = joinErrors(std::move(Err), std::move(ErrorState));
  }
  HTTPResponseHandler &Handler;
  toolchain::Error ErrorState = Error::success();
};

static size_t curlWriteFunction(char *Contents, size_t Size, size_t NMemb,
                                CurlHTTPRequest *CurlRequest) {
  Size *= NMemb;
  if (Error Err =
          CurlRequest->Handler.handleBodyChunk(StringRef(Contents, Size))) {
    CurlRequest->storeError(std::move(Err));
    return 0;
  }
  return Size;
}

HTTPClient::HTTPClient() {
  assert(IsInitialized &&
         "Must call HTTPClient::initialize() at the beginning of main().");
  if (Curl)
    return;
  Curl = curl_easy_init();
  assert(Curl && "Curl could not be initialized");
  // Set the callback hooks.
  curl_easy_setopt(Curl, CURLOPT_WRITEFUNCTION, curlWriteFunction);
  // Detect supported compressed encodings and accept all.
  curl_easy_setopt(Curl, CURLOPT_ACCEPT_ENCODING, "");
}

HTTPClient::~HTTPClient() { curl_easy_cleanup(Curl); }

Error HTTPClient::perform(const HTTPRequest &Request,
                          HTTPResponseHandler &Handler) {
  if (Request.Method != HTTPMethod::GET)
    return createStringError(errc::invalid_argument,
                             "Unsupported CURL request method.");

  SmallString<128> Url = Request.Url;
  curl_easy_setopt(Curl, CURLOPT_URL, Url.c_str());
  curl_easy_setopt(Curl, CURLOPT_FOLLOWLOCATION, Request.FollowRedirects);

  curl_slist *Headers = nullptr;
  for (const std::string &Header : Request.Headers)
    Headers = curl_slist_append(Headers, Header.c_str());
  curl_easy_setopt(Curl, CURLOPT_HTTPHEADER, Headers);

  CurlHTTPRequest CurlRequest(Handler);
  curl_easy_setopt(Curl, CURLOPT_WRITEDATA, &CurlRequest);
  CURLcode CurlRes = curl_easy_perform(Curl);
  curl_slist_free_all(Headers);
  if (CurlRes != CURLE_OK)
    return joinErrors(std::move(CurlRequest.ErrorState),
                      createStringError(errc::io_error,
                                        "curl_easy_perform() failed: %s\n",
                                        curl_easy_strerror(CurlRes)));
  return std::move(CurlRequest.ErrorState);
}

unsigned HTTPClient::responseCode() {
  long Code = 0;
  curl_easy_getinfo(Curl, CURLINFO_RESPONSE_CODE, &Code);
  return Code;
}

#else

HTTPClient::HTTPClient() = default;

HTTPClient::~HTTPClient() = default;

bool HTTPClient::isAvailable() { return false; }

void HTTPClient::initialize() {}

void HTTPClient::cleanup() {}

void HTTPClient::setTimeout(std::chrono::milliseconds Timeout) {}

Error HTTPClient::perform(const HTTPRequest &Request,
                          HTTPResponseHandler &Handler) {
  llvm_unreachable("No HTTP Client implementation available.");
}

unsigned HTTPClient::responseCode() {
  llvm_unreachable("No HTTP Client implementation available.");
}

#endif
