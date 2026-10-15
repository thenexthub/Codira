//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: August 6, 2026
//!
//! Functionality:
//! - Part of the Codira compiler and runtime toolchain.
use crate::SyntaxKind;

/// A bit-set of `SyntaxKind`s. Two words: the tokens (punctuation,
/// keywords, contextual keywords, literals) number more than 128.
#[derive(Clone, Copy)]
pub(crate) struct TokenSet([u128; 2]);

impl TokenSet {
    pub(crate) const fn empty() -> TokenSet {
        TokenSet([0; 2])
    }

    pub(crate) const fn new(kinds: &[SyntaxKind]) -> TokenSet {
        let mut res = [0u128; 2];
        let mut i = 0;
        while i < kinds.len() {
            let (word, bit) = mask(kinds[i]);
            res[word] |= bit;
            i += 1;
        }
        TokenSet(res)
    }

    pub(crate) const fn union(self, other: TokenSet) -> TokenSet {
        TokenSet([self.0[0] | other.0[0], self.0[1] | other.0[1]])
    }

    pub(crate) fn contains(&self, kind: SyntaxKind) -> bool {
        let (word, bit) = mask(kind);
        self.0.get(word).is_some_and(|w| w & bit != 0)
    }
}

/// The word and bit of `kind`. Kinds past 255 are nodes, never tokens a
/// set is built from (building one fails at compile time).
const fn mask(kind: SyntaxKind) -> (usize, u128) {
    let index = kind as usize;
    (index / 128, 1u128 << (index % 128))
}

#[test]
fn token_set_works_for_tokens() {
    use crate::SyntaxKind::*;
    let ts = TokenSet::new(&[EOF, EQ]);
    assert!(ts.contains(EOF));
    assert!(ts.contains(T![=]));
    assert!(!ts.contains(PLUS));
    // Past the first word.
    let ts = TokenSet::new(&[IDENT]);
    assert!(ts.contains(IDENT));
    assert!(!ts.contains(EOF));
}
