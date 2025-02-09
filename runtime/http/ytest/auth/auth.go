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

package auth

import (
	"net/http"
)

// RTComposer represents an abstract of http Authorization objects.
type RTComposer interface {
	Compose(base http.RoundTripper) http.RoundTripper
}

// -----------------------------------------------------------------------------

type tokenRT struct {
	rt    http.RoundTripper
	token string
}

func (p *tokenRT) RoundTrip(req *http.Request) (*http.Response, error) {
	req.Header.Set("Authorization", p.token)
	return p.rt.RoundTrip(req)
}

func WithToken(rt http.RoundTripper, token string) http.RoundTripper {
	return &tokenRT{
		rt:    rt,
		token: token,
	}
}

// -----------------------------------------------------------------------------
