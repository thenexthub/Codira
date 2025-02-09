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

package jwt

import (
	"log"
	"net/http"
	"time"

	"github.com/golang-jwt/jwt/v5"
	"github.com/goplus/yap/ytest/auth"
)

// -----------------------------------------------------------------------------

type Signaturer struct {
	method jwt.SigningMethod
	claims jwt.MapClaims
	secret any
}

func (p *Signaturer) Set(k string, v any) *Signaturer {
	p.claims[k] = v
	return p
}

func (p *Signaturer) Audience(aud ...string) *Signaturer {
	p.claims["aud"] = jwt.ClaimStrings(aud)
	return p
}

func (p *Signaturer) Expiration(exp time.Time) *Signaturer {
	p.claims["exp"] = jwt.NewNumericDate(exp)
	return p
}

func (p *Signaturer) NotBefore(nbf time.Time) *Signaturer {
	p.claims["nbf"] = jwt.NewNumericDate(nbf)
	return p
}

func (p *Signaturer) IssuedAt(iat time.Time) *Signaturer {
	p.claims["iat"] = jwt.NewNumericDate(iat)
	return p
}

func (p *Signaturer) Compose(rt http.RoundTripper) http.RoundTripper {
	token := jwt.NewWithClaims(p.method, p.claims)
	raw, err := token.SignedString(p.secret)
	if err != nil {
		log.Panicln("jwt token.SignedString:", err)
	}
	return auth.WithToken(rt, "Bearer "+raw)
}

func newSign(method jwt.SigningMethod, secret any) *Signaturer {
	return &Signaturer{
		method: method,
		claims: make(jwt.MapClaims),
		secret: secret,
	}
}

// HS256 creates a signing methods by using the HMAC-SHA256.
func HS256(key string) *Signaturer {
	return newSign(jwt.SigningMethodHS256, []byte(key))
}

// HS384 creates a signing methods by using the HMAC-SHA384.
func HS384(key string) *Signaturer {
	return newSign(jwt.SigningMethodHS384, []byte(key))
}

// HS512 creates a signing methods by using the HMAC-SHA512.
func HS512(key string) *Signaturer {
	return newSign(jwt.SigningMethodHS512, []byte(key))
}

// -----------------------------------------------------------------------------
