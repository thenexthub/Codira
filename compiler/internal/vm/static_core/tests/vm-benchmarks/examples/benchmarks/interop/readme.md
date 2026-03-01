
# Interoperability benchmarks

## I) Dynamic to dynamic (d2d)
```sh
vmb all -p interop_d2d -T d2d --src-langs=js --langs=js ./examples/benchmarks

# these tests could also be run on js platforms
vmb all -p node_host -T d2d -v debug ./examples/benchmarks
vmb all -p v_8_host -T d2d -v debug ./examples/benchmarks
vmb all -p ark_js_vm_host -T d2d --src-langs=js --langs=js ./examples/benchmarks
```

## II) Static to static (s2s)
```sh
vmb all -p arkts_host -A -T s2s ./examples/benchmarks
```
## III) Static to dynamic (s2d)
```sh
vmb all -p interop_s2d -T s2d ./examples/benchmarks
```

## IV) Dynamic to Static (d2s)
```sh
vmb all -p interop_d2s -T d2s ./examples
```
