#!/usr/bin/env bash
. $(dirname $0)/common.inc

[ "$CC" = cc ] || skip

# ASAN doesn't work with LD_PRELOAD
nm funis | grep '__[at]san_init' && skip

cat <<'EOF' | $CC -xc -c -o $t/a.o -
#include <stdio.h>

int main() {
  printf("Hello\n");
  return 0;
}
EOF

./funis -run $CC -o $t/exe $t/a.o -B/usr/bin

readelf -p .comment $t/exe | grep funis

./funis -run env | grep '^FUNIS_PATH=.*/funis$'

./funis -run /usr/bin/ld --version | grep funis
./funis -run /usr/bin/ld.lld --version | grep funis
./funis -run /usr/bin/ld.gold --version | grep funis

rm -f $t/ld $t/ld.lld $t/ld.gold $t/foo.ld
touch $t/ld $t/ld.lld $t/ld.gold
echo "#!/bin/sh" >$t/foo.ld
chmod 755 $t/ld $t/ld.lld $t/ld.gold $t/foo.ld

./funis -run $t/ld --version | grep funis
./funis -run $t/ld.lld --version | grep funis
./funis -run $t/ld.gold --version | grep funis
./funis -run $t/foo.ld --version | not grep funis

cat <<'EOF' > $t/sh
#!/bin/sh
$1 --version
EOF

chmod 755 $t/sh

./funis -run $t/sh ld --version | grep funis
./funis -run $t/sh $t/foo.ld --version |& not grep funis

./funis -run $t/sh $t/ld --version | grep funis
./funis -run $t/sh $t/ld.lld --version | grep funis
./funis -run $t/sh $t/ld.gold --version | grep funis
./funis -run $t/sh $t/foo.ld --version | not grep funis
