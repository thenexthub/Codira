//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 15, 2026
//!
//! Functionality:
//! - A small deterministic random number generator (`SplitMix64`) so request
//!   streams and mutation choices are reproducible from a seed, with no
//!   dependency on an external crate.

/// `SplitMix64`: fast, passes the usual statistical tests for this purpose, and
/// every output is a pure function of the seed.
#[derive(Clone, Debug)]
pub struct Rng(u64);

impl Rng {
    /// A generator whose whole sequence is determined by `seed`.
    pub fn new(seed: u64) -> Self {
        Rng(seed ^ 0x9E37_79B9_7F4A_7C15)
    }

    /// The next 64 random bits.
    pub fn next_u64(&mut self) -> u64 {
        self.0 = self.0.wrapping_add(0x9E37_79B9_7F4A_7C15);
        let mut z = self.0;
        z = (z ^ (z >> 30)).wrapping_mul(0xBF58_476D_1CE4_E5B9);
        z = (z ^ (z >> 27)).wrapping_mul(0x94D0_49BB_1331_11EB);
        z ^ (z >> 31)
    }

    /// A value in `0..n` (`n > 0`). The modulo bias is below 2^-40 for the
    /// small ranges used here.
    pub fn below(&mut self, n: u64) -> u64 {
        assert!(n > 0);
        self.next_u64() % n
    }

    /// A value in `lo..=hi`.
    pub fn range(&mut self, lo: i64, hi: i64) -> i64 {
        assert!(lo <= hi);
        lo + self.below((hi - lo) as u64 + 1) as i64
    }

    /// True with probability `percent / 100`.
    pub fn chance(&mut self, percent: u64) -> bool {
        self.below(100) < percent
    }

    /// Fisher-Yates shuffle.
    pub fn shuffle<T>(&mut self, items: &mut [T]) {
        for i in (1..items.len()).rev() {
            let j = self.below(i as u64 + 1) as usize;
            items.swap(i, j);
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn the_sequence_depends_only_on_the_seed() {
        let a: Vec<u64> = {
            let mut r = Rng::new(7);
            (0..5).map(|_| r.next_u64()).collect()
        };
        let b: Vec<u64> = {
            let mut r = Rng::new(7);
            (0..5).map(|_| r.next_u64()).collect()
        };
        let c: Vec<u64> = {
            let mut r = Rng::new(8);
            (0..5).map(|_| r.next_u64()).collect()
        };
        assert_eq!(a, b);
        assert_ne!(a, c);
    }

    #[test]
    fn range_is_inclusive_and_bounded() {
        let mut r = Rng::new(1);
        let mut seen = [false; 4];
        for _ in 0..200 {
            let v = r.range(-1, 2);
            assert!((-1..=2).contains(&v));
            seen[(v + 1) as usize] = true;
        }
        assert!(seen.iter().all(|s| *s));
    }
}
