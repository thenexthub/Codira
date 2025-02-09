//go:build nogc
// +build nogc

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

package pthread

import (
	_ "unsafe"

	c "github.com/goplus/llgo/runtime/internal/clite"
)

const (
	LLGoPackage = "decl"
)

// The pthread_create() function starts a new thread in the calling
// process.  The new thread starts execution by invoking
// start_routine(); arg is passed as the sole argument of
// start_routine().
//
// The new thread terminates in one of the following ways:
//
//   - It calls pthread_exit(3), specifying an exit status value that
//     is available to another thread in the same process that calls
//     pthread_join(3).
//
//   - It returns from start_routine().  This is equivalent to
//     calling pthread_exit(3) with the value supplied in the return
//     statement.
//
//   - It is canceled (see pthread_cancel(3)).
//
//   - Any of the threads in the process calls exit(3), or the main
//     thread performs a return from main().  This causes the
//     termination of all threads in the process.
//
// On success, pthread_create() returns 0; on error, it returns an
// error number, and the contents of *thread are undefined.
//
// See https://man7.org/linux/man-pages/man3/pthread_create.3.html
//
//go:linkname Create C.pthread_create
func Create(pthread *Thread, attr *Attr, routine RoutineFunc, arg c.Pointer) c.Int

// The pthread_join() function waits for the thread specified by
// thread to terminate.  If that thread has already terminated, then
// pthread_join() returns immediately.  The thread specified by
// thread must be joinable.
//
// If retval is not NULL, then pthread_join() copies the exit status
// of the target thread (i.e., the value that the target thread
// supplied to pthread_exit(3)) into the location pointed to by
// retval.  If the target thread was canceled, then PTHREAD_CANCELED
// is placed in the location pointed to by retval.
//
// If multiple threads simultaneously try to join with the same
// thread, the results are undefined.  If the thread calling
// pthread_join() is canceled, then the target thread will remain
// joinable (i.e., it will not be detached).
//
// See https://man7.org/linux/man-pages/man3/pthread_join.3.html
//
//go:linkname Join C.pthread_join
func Join(thread Thread, retval *c.Pointer) c.Int
