#!/usr/bin/env bash
. $(dirname $0)/common.inc

# GCC supports -mbranch-protection=gcs only since version 15, so use clang.
CLANG="clang ${TRIPLE:+--target=$TRIPLE}"
echo 'void foo() {}' | $CLANG -mbranch-protection=gcs -c -o /dev/null -xc - >& /dev/null || skip

# a.o is marked with BTI and GCS, and b.o only with BTI.
echo 'void _start() {}' | $CLANG -mbranch-protection=bti+gcs -c -o $t/a.o -xc -
echo 'void foo() {}' | $CLANG -mbranch-protection=bti -c -o $t/b.o -xc -

# Older readelf doesn't know the name of the GCS bit.
./funis -o $t/exe1 $t/a.o
readelf -n $t/exe1 | grep -E 'AArch64 feature: BTI, (GCS|<unknown: 4>)$'

./funis -o $t/exe2 $t/a.o -z gcs=never
readelf -n $t/exe2 | grep 'AArch64 feature: BTI$'

./funis -o $t/exe3 $t/a.o $t/b.o 2> $t/log
readelf -n $t/exe3 | grep 'AArch64 feature: BTI$'
not grep . $t/log

./funis -o $t/exe4 $t/a.o $t/b.o -z gcs=always 2> $t/log
readelf -n $t/exe4 | grep -E 'AArch64 feature: BTI, (GCS|<unknown: 4>)$'
grep 'b.o: -z gcs-report=warning: missing GNU_PROPERTY_AARCH64_FEATURE_1_GCS' $t/log
not grep a.o: $t/log

./funis -o $t/exe5 $t/a.o $t/b.o -z gcs=always -z gcs-report=none 2> $t/log
not grep . $t/log

not ./funis -o $t/exe6 $t/a.o $t/b.o -z gcs-report=error 2> $t/log
grep 'b.o: -z gcs-report=error: missing GNU_PROPERTY_AARCH64_FEATURE_1_GCS' $t/log

# c.so is marked with GCS, and d.so isn't.
echo 'void bar() {}' | $CLANG -fPIC -mbranch-protection=bti+gcs -c -o $t/c.o -xc -
echo 'void baz() {}' | $CLANG -fPIC -mbranch-protection=bti -c -o $t/d.o -xc -
./funis -shared -o $t/c.so $t/c.o
./funis -shared -o $t/d.so $t/d.o

./funis -o $t/exe7 $t/a.o $t/c.so $t/d.so 2> $t/log
not grep . $t/log

./funis -o $t/exe8 $t/a.o $t/c.so $t/d.so -z gcs=always 2> $t/log
grep 'd.so: -z gcs-report-dynamic=warning: missing GNU_PROPERTY_AARCH64_FEATURE_1_GCS' $t/log
not grep c.so: $t/log

./funis -o $t/exe9 $t/a.o $t/d.so -z gcs=always -z gcs-report-dynamic=none \
  --fatal-warnings

not ./funis -o $t/exe10 $t/a.o $t/d.so -z gcs-report-dynamic=error 2> $t/log
grep 'd.so: -z gcs-report-dynamic=error: missing GNU_PROPERTY_AARCH64_FEATURE_1_GCS' $t/log
