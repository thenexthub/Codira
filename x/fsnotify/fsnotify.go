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

package fsnotify

import (
	"errors"
	"io/fs"
	"log"
	"os"
	"path/filepath"

	"github.com/fsnotify/fsnotify"
)

var (
	debugEvent bool
)

const (
	DbgFlagEvent = 1 << iota
	DbgFlagAll   = DbgFlagEvent
)

func SetDebug(dbgFlags int) {
	debugEvent = (dbgFlags & DbgFlagEvent) != 0
}

// -----------------------------------------------------------------------------------------

type FSChanged interface {
	FileChanged(name string)
	DirAdded(name string)
	EntryDeleted(name string, isDir bool)
}

type Ignore = func(name string, isDir bool) bool

// -----------------------------------------------------------------------------------------

type Watcher struct {
	w *fsnotify.Watcher
}

func New() Watcher {
	w, err := fsnotify.NewWatcher()
	if err != nil {
		log.Panicln("[FATAL] fsnotify.NewWatcher:", err)
	}
	return Watcher{w}
}

func (p Watcher) Run(root string, fc FSChanged, ignore Ignore) error {
	go p.watchLoop(root, fc, ignore)
	return watchRecursive(p.w, root)
}

func (p Watcher) watchLoop(root string, fc FSChanged, ignore Ignore) {
	const (
		eventModify = fsnotify.Write | fsnotify.Create
	)
	for {
		select {
		case event, ok := <-p.w.Events:
			if !ok { // closed
				return
			}
			if debugEvent {
				log.Println("==> event:", event)
			}
			if (event.Op & fsnotify.Remove) != 0 {
				e := p.w.Remove(event.Name)
				name, err := filepath.Rel(root, event.Name)
				if err != nil {
					log.Println("[ERROR] fsnotify.EntryDeleted filepath.Rel:", err)
					continue
				}
				isDir := (e == nil || !errors.Is(e, fsnotify.ErrNonExistentWatch))
				name = filepath.ToSlash(name)
				if ignore != nil && ignore(name, isDir) {
					continue
				} else {
					fc.EntryDeleted(name, isDir)
				}
			} else if (event.Op & eventModify) != 0 {
				name, err := filepath.Rel(root, event.Name)
				if err != nil {
					log.Println("[ERROR] fsnotify.FileChanged filepath.Rel:", err)
					continue
				}
				isDir := isDir(event.Name)
				name = filepath.ToSlash(name)
				if ignore != nil && ignore(name, isDir) {
					continue
				} else if isDir {
					if (event.Op & fsnotify.Create) != 0 {
						watchRecursive(p.w, event.Name)
						fc.DirAdded(name)
					}
				} else {
					fc.FileChanged(name)
				}
			}
		case err, ok := <-p.w.Errors:
			if !ok {
				return
			}
			log.Println("[ERROR] fsnotify Errors:", err)
		}
	}
}

func (p *Watcher) Close() error {
	if w := p.w; w != nil {
		p.w = nil
		return w.Close()
	}
	return nil
}

func watchRecursive(w *fsnotify.Watcher, path string) error {
	err := filepath.WalkDir(path, func(walkPath string, d fs.DirEntry, err error) error {
		if err != nil {
			return err
		}
		if d.IsDir() {
			if err = w.Add(walkPath); err != nil {
				return err
			}
		}
		return nil
	})
	return err
}

func isDir(name string) bool {
	if fs, err := os.Lstat(name); err == nil {
		return fs.IsDir()
	}
	return false
}

// -----------------------------------------------------------------------------------------
