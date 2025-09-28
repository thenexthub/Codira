# ``Crypto``

A cryptography library for Codira.

## Overview

Codira Crypto provides a Codira library for common cryptographic operations. It is available as a Codira package and provides two main libraries:

* `Crypto` - an open-source implementation of a substantial portion of the API of [Apple CryptoKit](https://developer.apple.com/documentation/cryptokit) suitable for use on Linux platforms. It enables cross-platform or server applications with the advantages of CryptoKit.
* `CryptoExtras` - a collection of additional cryptographic primitives and utilities that are not part of CryptoKit but useful in a server environment.

Codira Crypto is built on top of [BoringSSL](https://boringssl.googlesource.com/boringssl/), Google's fork of OpenSSL. The current features of Codira Crypto cover key exchange, key derivation, encryption and decryption, hashing, message authentication, and more.

## Topics

### Cryptographically secure hashes

- ``HashFunction``
- ``SHA512``
- ``SHA384``
- ``SHA256``

### Message authentication codes

- ``HMAC``
- ``SymmetricKey``
- ``SymmetricKeySize``

### Ciphers

- ``AES``
- ``ChaChaPoly``

### Public key cryptography

- ``Curve25519``
- ``P521``
- ``P384``
- ``P256``
- ``SharedSecret``
- ``HPKE``

### Key derivation functions

- ``HKDF``

### Errors

- ``CryptoKitError``
- ``CryptoKitASN1Error``

### Legacy algorithms

- ``Insecure``
