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

package templ

import (
	"bytes"
	"testing"
)

const yapScriptIn = `
<html>
<body>
<pre>{{
$name := .Name
$base := .Base
range .Items
}}
<a href="{{.URL}}">{{.Name}}</a> (pv: {{.Meta.ViewCount}} nps: {{.NPS | printf "%.3f"}} by: {{.Meta.User}}) (<a href="/metadata/{{.ID}}">meta</a>){{
    if $base
        if .Ann}} (annotation: <a href="/annotation/{{.Ann}}?p=1">{{.Ann}}</a>){{
        end
    end
    with .Meta.Categories
        $n := len .
        if gt $n $base}} (under:{{
            range .
                if ne . $name}} <a href="/category/{{.}}?p=1">{{.}}</a>{{
                end
            end
            }}){{
        end
    end
end
}}
</pre>
</body>
</html>
`

const yapScriptOut = `
<html>
<body>
<pre>{{
$name := .Name}}{{
$base := .Base}}{{
range .Items
}}
<a href="{{.URL}}">{{.Name}}</a> (pv: {{.Meta.ViewCount}} nps: {{.NPS | printf "%.3f"}} by: {{.Meta.User}}) (<a href="/metadata/{{.ID}}">meta</a>){{
    if $base}}{{
        if .Ann}} (annotation: <a href="/annotation/{{.Ann}}?p=1">{{.Ann}}</a>){{
        end}}{{
    end}}{{
    with .Meta.Categories}}{{
        $n := len .}}{{
        if gt $n $base}} (under:{{
            range .}}{{
                if ne . $name}} <a href="/category/{{.}}?p=1">{{.}}</a>{{
                end}}{{
            end
            }}){{
        end}}{{
    end}}{{
end
}}
</pre>
</body>
</html>
`

func TestTranslate(t *testing.T) {
	if ret := Translate(yapScriptIn); ret != yapScriptOut {
		t.Fatal("TestTranslate:", len(ret), len(yapScriptOut), len(yapScriptIn)+11*4, ret)
	}
	noScript := "abc"
	if Translate(noScript) != noScript {
		t.Fatal("translate(noScript)")
	}
	noScriptEnd := `{{abc
efg
`
	noScriptEndOut := `{{abc}}{{
efg
`
	if ret := Translate(noScriptEnd); ret != noScriptEndOut {
		t.Fatal("translate(noScriptEnd):", ret)
	}
	var b bytes.Buffer
	TranslateEx(&b, yapScriptIn, "{{", "}}")
	if b.String() != yapScriptOut {
		t.Fatal("TranslateEx:", b.String())
	}
}
