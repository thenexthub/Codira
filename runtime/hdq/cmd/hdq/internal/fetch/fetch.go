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

// Package fetch implements the "hdq fetch" command.
package fetch

import (
	"encoding/json"
	"io"
	"log"
	"os"
	"strings"

	"github.com/goplus/hdq/cmd/hdq/internal/base"
	"github.com/goplus/hdq/fetcher"
)

// hdq fetch
var Cmd = &base.Command{
	UsageLine: "hdq fetch [flags] pageType [input ...]",
	Short:     "Fetch objects from the html source with the specified pageType and input",
}

func init() {
	Cmd.Run = runCmd
}

func runCmd(cmd *base.Command, args []string) {
	if len(args) < 2 {
		cmd.Usage(os.Stderr)
		return
	}
	pageType := args[0]
	inputs := args[1:]
	if len(inputs) == 1 && inputs[0] == "-" {
		b, _ := io.ReadAll(os.Stdin)
		inputs = strings.Split(strings.TrimSpace(string(b)), " ")
	}
	docs := make([]any, 0, len(inputs))
	for _, input := range inputs {
		log.Println("==> Fetch", input)
		doc, err := fetcher.FromInput(pageType, input)
		if err != nil {
			panic(err)
		}
		docs = append(docs, doc)
	}
	enc := json.NewEncoder(os.Stdout)
	enc.SetIndent("", "  ")
	enc.Encode(docs)
}
