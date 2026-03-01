# TreeMap and TreeSet implementation

Here are files implementing AVL-tree and TreeMap data structure, based on AVL-tree.
It is, in turn, used to implement TreeSet functionality.

Some sort of support of generics is done using [Jinja](https://palletsprojects.com/p/jinja/).
To expand `*.j2` files one can call `jinja2 -DK=Short -DV=String Set.ets.j2 > Set.ets`