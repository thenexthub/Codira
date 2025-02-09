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

package sync

// llgo:skipall
import (
	gosync "sync"
	"unsafe"

	c "github.com/goplus/llgo/runtime/internal/clite"
	"github.com/goplus/llgo/runtime/internal/clite/pthread/sync"
)

// -----------------------------------------------------------------------------

type Mutex sync.Mutex

func (m *Mutex) Lock() {
	if *(*c.Long)(unsafe.Pointer(m)) == 0 {
		(*sync.Mutex)(m).Init(nil) // TODO(xsw): finalize
	}
	(*sync.Mutex)(m).Lock()
}

func (m *Mutex) TryLock() bool {
	if *(*c.Long)(unsafe.Pointer(m)) == 0 {
		(*sync.Mutex)(m).Init(nil)
	}
	return (*sync.Mutex)(m).TryLock() == 0
}

// llgo:link (*Mutex).Unlock C.pthread_mutex_unlock
func (m *Mutex) Unlock() {}

// -----------------------------------------------------------------------------

type RWMutex sync.RWLock

func (rw *RWMutex) RLock() {
	if *(*c.Long)(unsafe.Pointer(rw)) == 0 {
		(*sync.RWLock)(rw).Init(nil)
	}
	(*sync.RWLock)(rw).RLock()
}

func (rw *RWMutex) TryRLock() bool {
	if *(*c.Long)(unsafe.Pointer(rw)) == 0 {
		(*sync.RWLock)(rw).Init(nil)
	}
	return (*sync.RWLock)(rw).TryRLock() == 0
}

// llgo:link (*RWMutex).RUnlock C.pthread_rwlock_unlock
func (rw *RWMutex) RUnlock() {}

func (rw *RWMutex) Lock() {
	if *(*c.Long)(unsafe.Pointer(rw)) == 0 {
		(*sync.RWLock)(rw).Init(nil)
	}
	(*sync.RWLock)(rw).Lock()
}

func (rw *RWMutex) TryLock() bool {
	if *(*c.Long)(unsafe.Pointer(rw)) == 0 {
		(*sync.RWLock)(rw).Init(nil)
	}
	return (*sync.RWLock)(rw).TryLock() == 0
}

// llgo:link (*RWMutex).Unlock C.pthread_rwlock_unlock
func (rw *RWMutex) Unlock() {}

// -----------------------------------------------------------------------------

type Once struct {
	m    Mutex
	done bool
}

func (o *Once) Do(f func()) {
	if !o.done {
		o.m.Lock()
		if !o.done {
			o.done = true
			f()
		}
		o.m.Unlock()
	}
}

// -----------------------------------------------------------------------------

type Cond struct {
	cond sync.Cond
	m    *sync.Mutex
}

func NewCond(l gosync.Locker) *Cond {
	ret := &Cond{m: l.(*sync.Mutex)}
	ret.cond.Init(nil) // TODO(xsw): finalize
	return ret
}

// llgo:link (*Cond).Signal C.pthread_cond_signal
func (c *Cond) Signal() {}

// llgo:link (*Cond).Broadcast C.pthread_cond_broadcast
func (c *Cond) Broadcast() {}

func (c *Cond) Wait() {
	c.cond.Wait(c.m)
}

// -----------------------------------------------------------------------------

type WaitGroup struct {
	mutex sync.Mutex
	cond  sync.Cond
	count int
}

func (wg *WaitGroup) doInit() {
	wg.mutex.Init(nil)
	wg.cond.Init(nil) // TODO(xsw): finalize
}

func (wg *WaitGroup) Add(delta int) {
	if *(*c.Long)(unsafe.Pointer(wg)) == 0 {
		wg.doInit()
	}
	wg.mutex.Lock()
	wg.count += delta
	if wg.count <= 0 {
		wg.cond.Broadcast()
	}
	wg.mutex.Unlock()
}

func (wg *WaitGroup) Done() {
	wg.Add(-1)
}

func (wg *WaitGroup) Wait() {
	if *(*c.Long)(unsafe.Pointer(wg)) == 0 {
		wg.doInit()
	}
	wg.mutex.Lock()
	for wg.count > 0 {
		wg.cond.Wait(&wg.mutex)
	}
	wg.mutex.Unlock()
}

// -----------------------------------------------------------------------------
