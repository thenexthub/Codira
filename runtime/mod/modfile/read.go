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

package modfile

import (
	"golang.org/x/mod/modfile"
)

// A Position describes an arbitrary source position in a file, including the
// file, line, column, and byte offset.
type Position = modfile.Position

// An Expr represents an input element.
type Expr = modfile.Expr

// A Comment represents a single // comment.
type Comment = modfile.Comment

// Comments collects the comments associated with an expression.
type Comments = modfile.Comments

// A FileSyntax represents an entire gop.mod file.
type FileSyntax = modfile.FileSyntax

// A CommentBlock represents a top-level block of comments separate
// from any rule.
type CommentBlock = modfile.CommentBlock

// A Line is a single line of tokens.
type Line = modfile.Line

// A LineBlock is a factored block of lines, like
//
//	require (
//		"x"
//		"y"
//	)
type LineBlock = modfile.LineBlock

// An LParen represents the beginning of a parenthesized line block.
// It is a place to store suffix comments.
type LParen = modfile.LParen

// An RParen represents the end of a parenthesized line block.
// It is a place to store whole-line (before) comments.
type RParen = modfile.RParen
