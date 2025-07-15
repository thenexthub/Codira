//===--- MockPlugin.h ---------------------------------------------*- C -*-===//
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

#ifndef LANGUAGE_C_MOCK_PLUGIN_H
#define LANGUAGE_C_MOCK_PLUGIN_H

#ifdef __cplusplus
extern "C" {
#endif

int _mock_plugin_main(const char *);

#ifdef __cplusplus
}
#endif

/// Usage: MOCK_PLUGIN(JSON)
/// 'JSON' is a *bare* JSON value.
#define MOCK_PLUGIN(...)                                                       \
  int main() { return _mock_plugin_main(#__VA_ARGS__); }

#endif // LANGUAGE_C_MOCK_PLUGIN_H
