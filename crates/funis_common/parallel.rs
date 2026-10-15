use rayon::prelude::*;

/// The number of threads worth starting for `bytes` of input: about
/// 1.5 * sqrt(MiB), at least one and at most `limit`.
///
/// Starting the thread pool costs more than a small link takes, and a link of
/// a few megabytes slows down again once many threads compete for cores
/// (measured on a 32-thread machine under load: 40 MiB of objects linked in
/// 74 ms with 4 threads and 169 ms with 32). The square root keeps the count
/// growing with the input, so a multi-gigabyte link still uses every core.
/// The output is byte-identical for every thread count.
pub fn adaptive_thread_count(bytes: u64, limit: usize) -> usize {
    let mib = bytes as f64 / (1u64 << 20) as f64;
    ((1.5 * mib.sqrt()).round() as usize).clamp(1, limit.max(1))
}

/// An owned Rayon job whose result is needed by a later linker pass.
pub struct Background<T> {
    receiver: std::sync::mpsc::Receiver<std::thread::Result<T>>,
    name: &'static str,
}

impl<T: Send + 'static> Background<T> {
    pub fn spawn(name: &'static str, run: impl FnOnce() -> T + Send + 'static) -> Self {
        let (sender, receiver) = std::sync::mpsc::sync_channel(1);
        // A job that panics would abort the process if rayon caught it, so
        // the panic is carried to `join` and resumed there. That is how a
        // link in a library session ends from inside a background job.
        rayon::spawn(move || {
            let _ = sender.send(std::panic::catch_unwind(std::panic::AssertUnwindSafe(run)));
        });
        Self { receiver, name }
    }

    pub fn join(self) -> T {
        use std::sync::mpsc::TryRecvError;
        loop {
            match self.receiver.try_recv() {
                Ok(Ok(value)) => return value,
                Ok(Err(payload)) => std::panic::resume_unwind(payload),
                Err(TryRecvError::Disconnected) => panic!("{} task failed", self.name),
                Err(TryRecvError::Empty) => {
                    // A blocking receive would deadlock a one-worker pool.
                    if !matches!(rayon::yield_now(), Some(rayon::Yield::Executed)) {
                        std::thread::yield_now();
                    }
                }
            }
        }
    }
}

/// Stably partitions a slice, returning the number of matching elements.
/// Each block scatters into disjoint ranges computed from its match count.
pub fn stable_partition<T: Copy + Send + Sync>(
    values: &mut [T],
    pred: impl Fn(&T) -> bool + Sync,
) -> usize {
    const BLOCK: usize = 16384;
    let matches: Vec<bool> = if values.len() <= BLOCK {
        values.iter().map(&pred).collect()
    } else {
        values.par_iter().map(&pred).collect()
    };
    let counts: Vec<usize> =
        matches.par_chunks(BLOCK).map(|chunk| chunk.iter().filter(|&&v| v).count()).collect();
    let num_matches = counts.iter().sum();
    if num_matches == 0 || num_matches == values.len() {
        return num_matches;
    }

    let mut output = values.to_vec();
    let (mut yes, mut no) = output.split_at_mut(num_matches);
    let parts: Vec<_> = counts
        .iter()
        .zip(matches.chunks(BLOCK))
        .map(|(&count, flags)| {
            let matched = yes.split_off_mut(..count).unwrap();

            let unmatched = no.split_off_mut(..flags.len() - count).unwrap();

            (matched, unmatched)
        })
        .collect();
    parts.into_par_iter().zip(values.par_chunks(BLOCK)).zip(matches.par_chunks(BLOCK)).for_each(
        |(((yes, no), input), flags)| {
            let (mut y, mut n) = (0, 0);
            for (&value, &matched) in input.iter().zip(flags) {
                if matched {
                    yes[y] = value;
                    y += 1;
                } else {
                    no[n] = value;
                    n += 1;
                }
            }
        },
    );
    values
        .par_chunks_mut(BLOCK)
        .zip(output.par_chunks(BLOCK))
        .for_each(|(dst, src)| dst.copy_from_slice(src));
    num_matches
}

/// An `UnsafeCell` that may be shared between threads, like std's unstable
/// type of the same name. A parallel pass views a slice as cells so that
/// each task can write the elements it owns through a shared reference,
/// with the slice's bounds checks still applied to every access.
#[repr(transparent)]
pub struct SyncUnsafeCell<T>(std::cell::UnsafeCell<T>);

// SAFETY: the unsafe methods below require callers to keep concurrent
// accesses to one cell disjoint, which is what `Sync` promises.
unsafe impl<T: Send + Sync> Sync for SyncUnsafeCell<T> {}

impl<T> SyncUnsafeCell<T> {
    pub fn new(value: T) -> Self {
        Self(std::cell::UnsafeCell::new(value))
    }

    /// Views an exclusively borrowed slice as shared cells.
    pub fn from_mut(slice: &mut [T]) -> &[Self] {
        // SAFETY: a cell has the same layout as its content, and the
        // exclusive borrow of the slice is given up for the shared view.
        unsafe { &*(std::ptr::from_mut(slice) as *const [Self]) }
    }

    /// A raw pointer to the content.
    pub fn get(&self) -> *mut T {
        self.0.get()
    }

    pub fn get_mut(&mut self) -> &mut T {
        self.0.get_mut()
    }

    /// Views cells as their contents.
    ///
    /// # Safety
    ///
    /// No cell in `cells` may be written while the returned slice is alive.
    pub(crate) unsafe fn as_slice(cells: &[Self]) -> &[T] {
        // SAFETY: a cell has the same layout as its content; the caller
        // guarantees that the contents do not change.
        unsafe { &*(std::ptr::from_ref(cells) as *const [T]) }
    }

    /// Views cells as their exclusively borrowed contents.
    ///
    /// # Safety
    ///
    /// No cell in `cells` may be accessed otherwise while the returned slice
    /// is alive.
    #[allow(clippy::mut_from_ref)]
    pub unsafe fn as_mut_slice(cells: &[Self]) -> &mut [T] {
        let ptr = std::cell::UnsafeCell::raw_get(cells.as_ptr().cast());
        // SAFETY: a cell has the same layout as its content; the caller
        // guarantees exclusive access.
        unsafe { std::slice::from_raw_parts_mut(ptr, cells.len()) }
    }
}

impl<T> std::fmt::Debug for SyncUnsafeCell<T> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("SyncUnsafeCell").finish_non_exhaustive()
    }
}

/// Sets flag bits without taking exclusive ownership of a cache line when
/// all requested bits are already set. Callers needing the previous value
/// must use `fetch_or` directly.
#[inline]
pub fn atomic_or(atomic: &std::sync::atomic::AtomicU8, bits: u8) {
    use std::sync::atomic::Ordering::Relaxed;
    if atomic.load(Relaxed) & bits != bits {
        atomic.fetch_or(bits, Relaxed);
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn background_job_joins_on_one_worker() {
        rayon::ThreadPoolBuilder::new().num_threads(1).build().unwrap().install(|| {
            let job = Background::spawn("test", || (0..100usize).into_par_iter().sum::<usize>());
            assert_eq!(job.join(), 4950);
        });
    }

    #[test]
    fn preserves_both_groups_across_blocks() {
        for len in [0, 1, 16383, 16384, 16385, 65539] {
            for divisor in [1, 2, 7, 100000] {
                let mut values: Vec<_> = (0..len).collect();
                let (yes, no): (Vec<_>, Vec<_>) =
                    values.iter().copied().partition(|v| v % divisor == 0);
                let count = stable_partition(&mut values, |v| v % divisor == 0);
                assert_eq!(count, yes.len());
                assert_eq!(&values[..count], yes);
                assert_eq!(&values[count..], no);
                assert_eq!(stable_partition(&mut values, |_| false), 0);
            }
        }
    }
}
