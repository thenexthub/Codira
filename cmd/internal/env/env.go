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

package env

import (
	"bytes"
	"encoding/json"
	"fmt"
	"log"
	"os"
	"os/exec"
	"sort"

	"github.com/goplus/gop/cmd/internal/base"
	"github.com/goplus/gop/env"
	"github.com/goplus/gop/x/gocmd"
	"github.com/goplus/mod"
	"github.com/goplus/mod/modcache"
)

// Cmd - gop env
var Cmd = &base.Command{
	UsageLine: "gop env [-json] [var ...]",
	Short:     "Prints Go+ environment information",
}

var (
	flag    = &Cmd.Flag
	envJson = flag.Bool("json", false, "prints Go environment information.")
)

func init() {
	Cmd.Run = runCmd
}

func runCmd(_ *base.Command, args []string) {
	err := flag.Parse(args)
	if err != nil {
		log.Fatalln("parse input arguments failed:", err)
	}

	var stdout bytes.Buffer

	cmd := exec.Command("go", "env", "-json")
	cmd.Env = os.Environ()
	cmd.Stdout = &stdout

	err = cmd.Run()
	if err != nil {
		log.Fatalln("run go env failed:", err)
	}

	var gopEnv map[string]interface{}
	if err := json.Unmarshal(stdout.Bytes(), &gopEnv); err != nil {
		log.Fatal("decode json of go env failed:", err)
	}

	gopEnv["BUILDDATE"] = env.BuildDate()
	gopEnv["GOPVERSION"] = env.Version()
	gopEnv["GOPROOT"] = env.GOPROOT()
	gopEnv["GOP_GOCMD"] = gocmd.Name()
	gopEnv["GOMODCACHE"] = modcache.GOMODCACHE
	gopEnv["GOPMOD"], _ = mod.GOPMOD("")
	gopEnv["HOME"] = env.HOME()

	vars := flag.Args()

	outputEnvVars(gopEnv, vars, *envJson)
}

func outputEnvVars(gopEnv map[string]interface{}, vars []string, outputJson bool) {
	onlyValues := true

	if len(vars) == 0 {
		onlyValues = false

		vars = make([]string, 0, len(gopEnv))
		for k := range gopEnv {
			vars = append(vars, k)
		}
		sort.Strings(vars)
	} else {
		newEnv := make(map[string]interface{})
		for _, v := range vars {
			if value, ok := gopEnv[v]; ok {
				newEnv[v] = value
			} else {
				newEnv[v] = ""
			}
		}
		gopEnv = newEnv
	}

	if outputJson {
		b, err := json.Marshal(gopEnv)
		if err != nil {
			log.Fatal("encode json of go env failed:", err)
		}

		var out bytes.Buffer
		json.Indent(&out, b, "", "\t")
		fmt.Println(out.String())
	} else {
		for _, k := range vars {
			v := gopEnv[k]
			if onlyValues {
				fmt.Printf("%v\n", v)
			} else {
				fmt.Printf("%s=\"%v\"\n", k, v)
			}
		}
	}
}
