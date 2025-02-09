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

package tool

import (
	"os"
	"os/exec"

	"github.com/goplus/mod/env"
	"github.com/goplus/mod/gopmod"
	"github.com/qiniu/x/errors"
)

func Tidy(dir string, gop *env.Gop) (err error) {
	modObj, err := gopmod.Load(dir)
	if err != nil {
		return errors.NewWith(err, `gopmod.Load(dir, mod.GopModOnly)`, -2, "gopmod.Load", dir)
	}

	modRoot := modObj.Root()
	/*
		depMods, err := GenDepMods(modObj, modRoot, true)
		if err != nil {
			return errors.NewWith(err, `GenDepMods(modObj, modRoot, true)`, -2, "tool.GenDepMods", modObj, modRoot, true)
		}

		old := modObj.DepMods()
		for modPath := range old {
			if _, ok := depMods[modPath]; !ok { // removed
				modObj.DropRequire(modPath)
			}
		}
		for modPath := range depMods {
			if _, ok := old[modPath]; !ok { // added
				if newMod, e := modfetch.Get(modPath); e != nil {
					return errors.NewWith(e, `modfetch.Get(modPath)`, -1, "modfetch.Get", modPath)
				} else {
					modObj.AddRequire(newMod.Path, newMod.Version)
				}
			}
		}

		modObj.Cleanup()
		err = modObj.Save()
		if err != nil {
			return errors.NewWith(err, `modObj.Save()`, -2, "(*gopmod.Module).Save")
		}
	*/
	conf := &Config{Gop: gop}
	err = genGoDir(modRoot, conf, true, true, 0)
	if err != nil {
		return errors.NewWith(err, `genGoDir(modRoot, conf, true, true)`, -2, "tool.genGoDir", modRoot, conf, true, true)
	}

	cmd := exec.Command("go", "mod", "tidy")
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr
	cmd.Dir = modRoot
	err = cmd.Run()
	if err != nil {
		err = errors.NewWith(err, `cmd.Run()`, -2, "(*exec.Cmd).Run")
	}
	return
}
