/*
 * Copyright (c) NeXTHub Corporation. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 */

package raylib

import (
	_ "unsafe"

	"github.com/goplus/llgo/c"
)

// -----------------------------------------------------------------------------

// Show trace log messages (LOG_DEBUG, LOG_INFO, LOG_WARNING, LOG_ERROR...)
//
//go:linkname TraceLog C.TraceLog
func TraceLog(logLevel int, text *c.Char, __llgo_va_list ...any)

// Set the current threshold (minimum) log level
//
//go:linkname SetTraceLogLevel C.SetTraceLogLevel
func SetTraceLogLevel(logLevel int)

// -----------------------------------------------------------------------------
// Set custom callbacks

// -----------------------------------------------------------------------------
// Files management functions

// -----------------------------------------------------------------------------
// File system functions

// Check if file exists
//
//go:linkname FileExists C.FileExists
func FileExists(fileName *c.Char) bool

// Check if a directory path exists
//
//go:linkname DirectoryExists C.DirectoryExists
func DirectoryExists(dirPath *c.Char) bool

// Check file extension (including point: .png, .wav)
//
//go:linkname IsFileExtension C.IsFileExtension
func IsFileExtension(fileName *c.Char, ext *c.Char) bool

// -----------------------------------------------------------------------------
// Compression/Encoding functionality

// -----------------------------------------------------------------------------
// Automation events functionality

// -----------------------------------------------------------------------------
