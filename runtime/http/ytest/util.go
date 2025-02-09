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

package ytest

import (
	"encoding/json"
	"net/url"
	"strconv"

	"github.com/goplus/yap/test"
	"github.com/goplus/yap/ytest/auth"
	"github.com/goplus/yap/ytest/auth/bearer"
)

// -----------------------------------------------------------------------------

// Bearer creates a Bearer Authorization by specified token.
func Bearer(token string) auth.RTComposer {
	return bearer.New(token)
}

// -----------------------------------------------------------------------------

// JsonEncode encodes a value into string in json format.
func JsonEncode(v any) string {
	b, err := json.Marshal(v)
	if err != nil {
		test.Fatal("json.Marshal failed:", err)
	}
	return string(b)
}

// Form encodes a map value into string in form format.
func Form(m map[string]any) (ret url.Values) {
	ret = make(url.Values)
	for k, v := range m {
		ret[k] = formVal(v)
	}
	return ret
}

func formVal(val any) []string {
	switch v := val.(type) {
	case string:
		return []string{v}
	case []string:
		return v
	case int:
		return []string{strconv.Itoa(v)}
	case bool:
		if v {
			return []string{"true"}
		}
		return []string{"false"}
	case float64:
		return []string{strconv.FormatFloat(v, 'g', -1, 64)}
	}
	test.Fatalf("formVal unexpected type: %T\n", val)
	return nil
}

// -----------------------------------------------------------------------------
