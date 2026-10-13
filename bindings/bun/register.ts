// Copyright (c) 2026 Omnira CJSC
// Author: Tunjay Akbarli
// Date: October 13, 2026
//
// Functionality:
// - Registers the Codira plugin with Bun's runtime. Preload it from
//   bunfig.toml (`preload = ["codira-bun/register"]`) or `bun --preload`.

import { plugin } from "bun";

import { codira } from "./plugin.ts";

plugin(codira());
