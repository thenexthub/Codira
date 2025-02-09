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

package safesplit

import "strings"

// SplitPkgConfigFlags splits a pkg-config outputs string into parts.
// Each part starts with "-" followed by a single character flag.
// Spaces after the flag character are ignored.
// Content is read until the next space, unless escaped with "\".
func SplitPkgConfigFlags(s string) []string {
	var result []string
	var current strings.Builder
	i := 0

	// Skip leading whitespace
	for i < len(s) && (s[i] == ' ' || s[i] == '\t') {
		i++
	}

	for i < len(s) {
		// Start a new part
		if current.Len() > 0 {
			result = append(result, strings.TrimSpace(current.String()))
			current.Reset()
		}
		// Write "-" and the flag character
		current.WriteByte('-')
		i++
		if i < len(s) {
			current.WriteByte(s[i])
			i++
		}
		// Skip spaces after flag character
		for i < len(s) && (s[i] == ' ' || s[i] == '\t') {
			i++
		}
		// Read content until next space
		for i < len(s) {
			if s[i] == '\\' && i+1 < len(s) && (s[i+1] == ' ' || s[i+1] == '\t') {
				// Skip backslash and write the escaped space
				i++
				current.WriteByte(s[i])
				i++
				continue
			}
			if s[i] == ' ' || s[i] == '\t' {
				// Skip consecutive spaces
				j := i
				for j < len(s) && (s[j] == ' ' || s[j] == '\t') {
					j++
				}
				// If we've seen content, check for new flag
				if j < len(s) && s[j] == '-' {
					i = j
					break
				}
				// Otherwise, include one space and continue
				current.WriteByte(' ')
				i = j
			} else {
				current.WriteByte(s[i])
				i++
			}
		}
	}
	// Add the last part
	if current.Len() > 0 {
		result = append(result, strings.TrimSpace(current.String()))
	}
	return result
}
