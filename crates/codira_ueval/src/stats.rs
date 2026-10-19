//! Copyright (c) 2026 Omnira CJSC
//! Author: Tunjay Akbarli
//! Date: October 15, 2026
//!
//! Functionality:
//! - Summary statistics for repeated measurements: minimum, median, spread
//!   (interquartile range) and tail percentiles by the nearest-rank method.

/// Summary of a sample of measurements.
#[derive(Clone, Debug, PartialEq)]
pub struct Summary {
    /// Number of observations.
    pub n: usize,
    /// Smallest observation.
    pub min: f64,
    /// First quartile.
    pub q1: f64,
    /// Median.
    pub median: f64,
    /// Third quartile.
    pub q3: f64,
    /// 90th percentile.
    pub p90: f64,
    /// 99th percentile.
    pub p99: f64,
    /// Largest observation.
    pub max: f64,
    /// Arithmetic mean.
    pub mean: f64,
}

/// Nearest-rank percentile of an ascending-sorted, non-empty slice.
pub fn percentile(sorted: &[f64], p: f64) -> f64 {
    assert!(!sorted.is_empty());
    let rank = ((p / 100.0) * sorted.len() as f64).ceil() as usize;
    sorted[rank.clamp(1, sorted.len()) - 1]
}

impl Summary {
    /// Summarises `samples` (not empty, no NaN).
    pub fn of(samples: &[f64]) -> Summary {
        let mut s = samples.to_vec();
        s.sort_by(|a, b| a.partial_cmp(b).expect("no NaN in a measurement"));
        Summary {
            n: s.len(),
            min: s[0],
            q1: percentile(&s, 25.0),
            median: percentile(&s, 50.0),
            q3: percentile(&s, 75.0),
            p90: percentile(&s, 90.0),
            p99: percentile(&s, 99.0),
            max: s[s.len() - 1],
            mean: s.iter().sum::<f64>() / s.len() as f64,
        }
    }

    /// Interquartile range.
    pub fn iqr(&self) -> f64 {
        self.q3 - self.q1
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn nearest_rank_percentiles() {
        let v: Vec<f64> = (1..=100).map(f64::from).collect();
        assert_eq!(percentile(&v, 50.0), 50.0);
        assert_eq!(percentile(&v, 99.0), 99.0);
        assert_eq!(percentile(&v, 100.0), 100.0);
        assert_eq!(percentile(&v, 0.0), 1.0);
    }

    #[test]
    fn summary_of_a_small_sample() {
        let s = Summary::of(&[5.0, 1.0, 3.0, 2.0, 4.0]);
        assert_eq!((s.n, s.min, s.median, s.max), (5, 1.0, 3.0, 5.0));
        assert_eq!(s.mean, 3.0);
        assert_eq!(s.iqr(), 2.0);
    }
}
