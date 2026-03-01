How to run
==========

# assume PANDA and VMB are built and installed per $PANDA_ROOT/tests/vm-benchmarks/readme.md
cd $PANDA_ROOT/tests/vm-benchmarks

# VMB needs variables PANDA_SRC and PANDA_BUILD for these benchmarks
export PANDA_SRC=$PANDA_ROOT
export PANDA_BUILD=$PANDA_ROOT/build

vmb all -p arkts_host --aot-skip-libs -v debug $PANDA_SRC/plugins/ets/tests/benchmarks/ani

Example of the output of successful run
=======================================

arkts_host-default
==================

8 tests; 0 failed; 0 excluded; Time(GM): 1156.7 Size(GM): 161181.0 RSS(GM): 54898.9
===================================================================================


Test                             |   Time   | CodeSize |   RSS    | Status  |
=============================================================================
BasicCall_baselineCallBench      | 1.95e+03 | 1.61e+05 | 5.48e+04 | Passed  |
BasicCall_criticalCallBench      | 4.07e+00 | 1.61e+05 | 5.49e+04 | Passed  |
BasicCall_fastStaticCallBench    | 1.68e+03 | 1.61e+05 | 5.51e+04 | Passed  |
BasicCall_finalCallBench         | 3.25e+03 | 1.61e+05 | 5.49e+04 | Passed  |
BasicCall_finalFastCallBench     | 2.12e+03 | 1.61e+05 | 5.49e+04 | Passed  |
BasicCall_nonStaticCallBench     | 5.61e+03 | 1.61e+05 | 5.48e+04 | Passed  |
BasicCall_nonStaticFastCallBench | 2.34e+03 | 1.61e+05 | 5.48e+04 | Passed  |
BasicCall_staticCallBench        | 2.67e+03 | 1.61e+05 | 5.49e+04 | Passed  |

