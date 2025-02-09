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

package main

import (
	"fmt"
	"io"
	"log"
	"os"

	"github.com/goplus/llgo/xtool/ar"
)

func main() {
	if len(os.Args) != 2 {
		fmt.Fprintln(os.Stderr, "Usage: ardump xxx.a")
		return
	}

	f, err := os.Open(os.Args[1])
	check(err)
	defer f.Close()

	r, err := ar.NewReader(f)
	check(err)
	for {
		hdr, err := r.Next()
		if err != nil {
			if err != io.EOF {
				log.Println(err)
			}
			break
		}
		fmt.Println(hdr.Name)
	}
}

func check(err error) {
	if err != nil {
		panic(err)
	}
}
