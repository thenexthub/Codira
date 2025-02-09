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

type T struct {
	*T
	A int `json:"a"`
}

func bar(v chan bool) (int, <-chan error) {
	v <- true
	<-v
	return 0, (<-chan error)(nil)
}

func foo(f func([]byte, *string, ...T) chan<- int) (v int, err error) {
	return
}
