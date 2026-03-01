# Recheck test system


## Testing algorithm:
1) Build es2panda
2) Compile plugin in build directory with :
```bash
ninja e2p_test_plugin_recheck
```
2) Run runner.sh using:
```bash
# Start runner
bash runner.sh --recheck --processes=all --build-dir=<build> --work-dir=<build>tools/es2panda/test/unit/recheck/recheck_src/src --es2panda-timeout=120 --timeout=120

```

## Ignore list:
You can find it in `<ets_frontend>/ets2panda/test/test-lists/recheck/recheck-ignored.txt`.  
There are ignored negative tests that come first. Next come the falls ater recheck that should be reduced.