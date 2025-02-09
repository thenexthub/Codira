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

package modcache

import (
	"errors"
	"path/filepath"
	"strings"

	"golang.org/x/mod/module"
)

// -----------------------------------------------------------------------------

var (
	GOMODCACHE = getGOMODCACHE()
)

// -----------------------------------------------------------------------------

var (
	ErrNoNeedToDownload = errors.New("no need to download")
)

// DownloadCachePath returns download cache path of a versioned module.
func DownloadCachePath(mod module.Version) (string, error) {
	if mod.Version == "" {
		return mod.Path, ErrNoNeedToDownload
	}
	encPath, err := module.EscapePath(mod.Path)
	if err != nil {
		return "", err
	}
	return filepath.Join(GOMODCACHE, "cache/download", encPath, "@v", mod.Version+".zip"), nil
}

// Path returns cache dir of a versioned module.
func Path(mod module.Version) (string, error) {
	if mod.Version == "" {
		return mod.Path, nil
	}
	encPath, err := module.EscapePath(mod.Path)
	if err != nil {
		return "", err
	}
	return filepath.Join(GOMODCACHE, encPath+"@"+mod.Version), nil
}

// InPath returns if a path is in GOMODCACHE or not.
func InPath(path string) bool {
	if strings.HasPrefix(path, GOMODCACHE) {
		name := path[len(GOMODCACHE):]
		return name == "" || name[0] == '/' || name[0] == '\\'
	}
	return false
}

// -----------------------------------------------------------------------------
