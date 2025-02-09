# Go bindings to system LLVM

[![Build Status](https://github.com/goplus/llvm/actions/workflows/go.yml/badge.svg)](https://github.com/goplus/llvm/actions/workflows/go.yml)
[![Go Report Card](https://goreportcard.com/badge/github.com/goplus/llvm)](https://goreportcard.com/report/github.com/goplus/llvm)
[![GoDoc](https://pkg.go.dev/badge/github.com/goplus/llvm.svg)](https://pkg.go.dev/github.com/goplus/llvm)
<!--
[![GitHub release](https://img.shields.io/github/v/tag/goplus/llvm.svg?label=release)](https://github.com/goplus/llvm/releases)
[![Coverage Status](https://codecov.io/gh/goplus/llvm/branch/main/graph/badge.svg)](https://codecov.io/gh/goplus/llvm)
-->

This library provides bindings to a system-installed LLVM.

Currently supported:

  * LLVM 17, 16, 15 and 14 from [apt.llvm.org](http://apt.llvm.org/) on Debian/Ubuntu.
  * LLVM 17, 16, 15 and 14 from Homebrew on macOS.
  * LLVM 15 with a manually built LLVM through the `byollvm` build tag. You
    need to set up `CFLAGS`/`LDFLAGS` etc yourself in this case.

You can select the LLVM version using a build tag, for example `-tags=llvm14` to use LLVM 14.

## Usage

If you have a supported LLVM installation, you should be able to do a simple `go get`:

    go get github.com/goplus/llvm

You can use build tags to select a LLVM version. For example, use `-tags=llvm15` to select LLVM 15. Setting a build tag for a LLVM version that is not supported will be ignored.

## License

These LLVM bindings for Go originally come from LLVM, but they have since been [removed](https://discourse.llvm.org/t/rfc-remove-the-go-bindings/65725). Still, they remain under the same license as they were originally, which is the [Apache License 2.0 (with LLVM exceptions)](http://releases.llvm.org/9.0.0/LICENSE.TXT). Check upstream LLVM for detailed copyright information.

This README, the backports\* files, and the Makefile are separate from LLVM but are licensed under the same license.
