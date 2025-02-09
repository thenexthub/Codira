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

package bearer

import (
	"net/http"

	"github.com/goplus/yap/ytest/auth"
)

// -----------------------------------------------------------------------------

type tokenAuth struct {
	token string
}

func (p *tokenAuth) Compose(rt http.RoundTripper) http.RoundTripper {
	return auth.WithToken(rt, "Bearer "+p.token)
}

// New creates an Authorization by specified token.
func New(token string) auth.RTComposer {
	return &tokenAuth{
		token: token,
	}
}

// -----------------------------------------------------------------------------
