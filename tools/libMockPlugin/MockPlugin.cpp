//===--- MockPlugin.cpp ---------------------------------------------------===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//

#include "language-c/MockPlugin/MockPlugin.h"
#include "toolchain/ADT/StringExtras.h"
#include "toolchain/Support/Endian.h"
#include "toolchain/Support/JSON.h"

#include <stdio.h>

namespace {
struct TestItem {
  toolchain::json::Value expect;
  toolchain::json::Value response;
  bool handled = false;

  TestItem(const toolchain::json::Value expect, const toolchain::json::Value response)
      : expect(expect), response(response) {}
};

struct TestRunner {
  std::vector<TestItem> items;

  TestRunner() {}
  static std::unique_ptr<TestRunner> create(const char *testSpecStr);

  toolchain::json::Value createResponse(const TestItem &item,
                                   const toolchain::json::Value &req);

  TestItem *findMatchItem(const toolchain::json::Value &req);

  int run();
};
} // namespace

std::unique_ptr<TestRunner> TestRunner::create(const char *testSpecStr) {
  // Read test spec.
  auto testSpec = toolchain::json::parse(testSpecStr);
  if (!testSpec) {
    toolchain::errs() << "failed to parse test spec JSON\n";
    toolchain::handleAllErrors(testSpec.takeError(),
                          [](const toolchain::ErrorInfoBase &err) {
                            toolchain::errs() << err.message() << "\n";
                          });
    return nullptr;
  }

  auto *items = testSpec->getAsArray();
  if (!items) {
    toolchain::errs() << "test spec list must be an array\n";
    return nullptr;
  }

  std::unique_ptr<TestRunner> runner(new TestRunner());

  for (auto item : *items) {
    auto *obj = item.getAsObject();
    if (!obj) {
      toolchain::errs() << "test spec must be an object\n";
      return nullptr;
    }
    auto *expect = obj->get("expect");
    auto *response = obj->get("response");
    if (!expect || !response) {
      toolchain::errs() << "test item must have 'expect' and 'response'\n";
      return nullptr;
    }
    runner->items.emplace_back(*expect, *response);
  }

  return runner;
}

/// Replace string values starting with '=req' (e.g. '"=req.foo.bar[2].baz"') in
/// \p value with the corresponding values from \p req .
static const toolchain::json::Value substitute(const toolchain::json::Value &value,
                                          const toolchain::json::Value &req) {

  // Recursively substitute objects and arrays.
  if (auto *obj = value.getAsObject()) {
    toolchain::json::Object copy;
    for (auto &kv : *obj) {
      copy[kv.first] = substitute(kv.second, req);
    }
    return copy;
  }
  if (auto *arr = value.getAsArray()) {
    toolchain::json::Array copy;
    for (auto &elem : *arr) {
      copy.push_back(substitute(elem, req));
    }
    return copy;
  }

  auto str = value.getAsString();
  if (!str || !str->starts_with("=req")) {
    // Not a substitution.
    return value;
  }

  auto path = str->substr(4);
  const toolchain::json::Value *subst = &req;
  while (!path.empty()) {
    // '.' <alphanum> -> object key.
    if (path.starts_with(".")) {
      if (subst->kind() != toolchain::json::Value::Object)
        return "<substitution error: not an object>";
      path = path.substr(1);
      auto keyLength =
          path.find_if([](char c) { return !toolchain::isAlnum(c) && c != '_'; });
      auto key = path.slice(0, keyLength);
      subst = subst->getAsObject()->get(key);
      path = path.substr(keyLength);
      continue;
    }
    // '[' <digit>+ ']' -> array index.
    if (path.starts_with("[")) {
      if (subst->kind() != toolchain::json::Value::Array)
        return "<substitution error: not an array>";
      path = path.substr(1);
      auto idxlength = path.find_if([](char c) { return !toolchain::isDigit(c); });
      size_t idx;
      (void)path.slice(0, idxlength).getAsInteger(10, idx);
      subst = &(*subst->getAsArray())[idx];
      path = path.substr(idxlength);
      if (!path.starts_with("]"))
        return "<substitution error: missing ']' after digits>";
      path = path.substr(1);
      continue;
    }
    // Malformed.
    return "<substitution error: malformed path>";
  }
  return *subst;
}

toolchain::json::Value TestRunner::createResponse(const TestItem &item,
                                             const toolchain::json::Value &req) {
  auto response = item.response;
  return substitute(response, req);
}

/// Check if \p expect matches \p val .
static bool match(const toolchain::json::Value &expect,
                  const toolchain::json::Value &val) {
  if (expect.kind() != val.kind())
    return false;

  switch (expect.kind()) {
  case toolchain::json::Value::Object: {
    auto *exp = expect.getAsObject();
    auto *obj = val.getAsObject();
    return toolchain::all_of(*exp, [obj](const auto &kv) {
      auto *val = obj->get(kv.first.str());
      return val && match(kv.second, *val);
    });
    return true;
  }

  case toolchain::json::Value::Array: {
    auto *exp = expect.getAsArray();
    auto *arr = val.getAsArray();
    if (exp->size() != arr->size())
      return false;
    return toolchain::all_of_zip(
        *exp, *arr,
        [](const auto &expect, const auto &val) { return match(expect, val); });
  }

  default:
    return expect == val;
  }
}

TestItem *TestRunner::findMatchItem(const toolchain::json::Value &req) {
  for (auto &item : items) {
    if (match(item.expect, req)) {
      return &item;
    }
  }
  return nullptr;
}

int TestRunner::run() {
  size_t ioSize;
  while (true) {
    // Read request header.
    uint64_t request_header;
    ioSize = fread(&request_header, sizeof(request_header), 1, stdin);
    if (!ioSize) {
      // STDIN is closed.
      return 0;
    }

    // Read request data.
    auto request_size = toolchain::support::endian::byte_swap(
        request_header, toolchain::endianness::little);
    toolchain::SmallVector<char, 0> request_data;
    request_data.assign(request_size, 0);
    ioSize = fread(request_data.data(), request_size, 1, stdin);
    if (!ioSize) {
      toolchain::errs() << "failed to read request data\n";
      return 1;
    }

    // Parse request object.
    auto request_obj =
        toolchain::json::parse({request_data.data(), request_data.size()});
    if (!request_obj) {
      toolchain::handleAllErrors(
          request_obj.takeError(),
          [](toolchain::ErrorInfoBase &err) { toolchain::errs() << err.message(); });
      return 1;
    }

    // Handle
    auto *item = findMatchItem(*request_obj);
    if (!item) {
      toolchain::errs() << "couldn't find matching item for request: " << *request_obj
                   << "\n";
      return 1;
    }
    if (item->handled) {
      toolchain::errs() << "request is already handled\n";
      return 1;
    }
    item->handled = true;
    auto response = createResponse(*item, *request_obj);

    // Write response data.
    toolchain::SmallVector<char, 0> response_data;
    toolchain::raw_svector_ostream(response_data) << response;
    auto response_size = response_data.size();
    auto response_header =
        toolchain::support::endian::byte_swap<uint64_t, toolchain::endianness::little>(
            response_size);
    ioSize = fwrite(&response_header, sizeof(response_header), 1, stdout);
    if (!ioSize) {
      toolchain::errs() << "failed to write response header\n";
      return 1;
    }
    ioSize = fwrite(response_data.data(), response_size, 1, stdout);
    if (!ioSize) {
      toolchain::errs() << "failed to write response data\n";
      return 1;
    }
    fflush(stdout);
  }
}

int _mock_plugin_main(const char *testSpectStr) {
  auto runner = TestRunner::create(testSpectStr);
  if (!runner) {
    return 1;
  }
  return runner->run();
}
