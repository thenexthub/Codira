// Copyright (c) 2026 Omnira CJSC
// Author: Tunjay Akbarli
// Date: October 13, 2026
//
// Functionality:
// - A Bun plugin that makes a `.code` file importable:
//
//     import { add } from "./src/mod.code";
//     add(1, 2);
//
//   On import it builds the package the file belongs to with `codira build`,
//   asks `codira bindgen` for that module's `bun:ffi` bindings, and loads a
//   private copy of the built library.
//
// The copy is what makes this work with `bun --hot`: an open library is
// locked on Windows and cached by path everywhere else, so loading the
// compiler's output in place would stop the next build from replacing it,
// and a reload would get the old code back. Each build is copied to a file
// named after the hash of its contents, so an unchanged rebuild reuses the
// same copy and a changed one gets a fresh one.
//
// Calls go straight through `bun:ffi`'s JIT-compiled trampolines -- no
// marshalling layer of our own sits between JavaScript and the native code.

import type { BunPlugin } from "bun";
import { existsSync, mkdirSync, readFileSync, copyFileSync } from "node:fs";
import { dirname, join, resolve } from "node:path";
import { tmpdir } from "node:os";

export interface CodiraPluginOptions {
  /// The `codira` executable. Defaults to `$CODIRA`, then `codira` on PATH.
  codira?: string;
  /// Optimization level passed to `codira build -O`. Defaults to 2.
  optimization?: 0 | 1 | 2 | 3;
  /// Where the private copies of built libraries go.
  cacheDir?: string;
}

function findManifest(file: string): string {
  let dir = dirname(resolve(file));
  for (;;) {
    const candidate = join(dir, "codira.toml");
    if (existsSync(candidate)) return candidate;
    const parent = dirname(dir);
    if (parent === dir) {
      throw new Error(`codira-bun: no codira.toml above ${file}`);
    }
    dir = parent;
  }
}

function run(command: string[], what: string): string {
  const result = Bun.spawnSync(command, { stdout: "pipe", stderr: "pipe" });
  if (result.exitCode !== 0) {
    throw new Error(
      `codira-bun: ${what} failed (${command.join(" ")}):\n` +
        result.stderr.toString() +
        result.stdout.toString(),
    );
  }
  return result.stdout.toString();
}

export function codira(options: CodiraPluginOptions = {}): BunPlugin {
  const codiraExe = options.codira ?? process.env.CODIRA ?? "codira";
  const optimization = String(options.optimization ?? 2);
  const cacheDir = options.cacheDir ?? join(tmpdir(), "codira-bun");

  return {
    name: "codira",
    setup(build) {
      build.onLoad({ filter: /\.code$/ }, (args) => {
        const manifest = findManifest(args.path);
        run(
          [codiraExe, "build", "--manifest-path", manifest, "-O", optimization],
          "build",
        );

        const bindgen = [
          codiraExe,
          "bindgen",
          "--runtime",
          "bun",
          "--manifest-path",
          manifest,
          "--module",
          args.path,
        ];
        const built = run([...bindgen, "--print-library"], "locating the library").trim();

        const hash = Bun.hash(readFileSync(built)).toString(16);
        mkdirSync(cacheDir, { recursive: true });
        const extension = built.slice(built.lastIndexOf("."));
        const copy = join(cacheDir, `${hash}${extension}`);
        if (!existsSync(copy)) copyFileSync(built, copy);

        const contents = run(
          [...bindgen, "--library", copy, "--named-exports"],
          "generating bindings",
        );
        return { contents, loader: "ts" };
      });
    },
  };
}

export default codira;
