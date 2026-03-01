## Thread pool

TLA+/PlusCal model for thread pool (`runtime/thread_pool.h`) is placed in `thread_pool/thread_pool.tla`.
This model was verified in `TLA+ Toolbox` tool with the following parameters: `max_producers = 2`,
`max_workers = 2`, `queue_max_size = 2`, `max_controllers = 2`, deadlock check.
The verification process took about 2 hours on machine with the following configuration:
Intel Core i7-9850H CPU @ 2.60GHz, 12 cores, 16 GB RAM.