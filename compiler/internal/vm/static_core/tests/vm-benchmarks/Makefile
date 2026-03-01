.PHONY: clean vmb-build

VMB = ./run-vmb.sh
OUTDIR ?= $(HOME)/vmb-build
BIN_PY = python3

clean:
	rm -rf dist generated src/generated examples/benchmarks/generated
	rm -rf .pytest_cache
	rm -rf src/vmb/.pytest_cache
	rm -rf src/vmb.egg-info
	rm -rf .mypy_cache
	rm -rf .tox
	find . -type d -name __pycache__ -exec rm -rf {} +

uninstall:
	$(BIN_PY)  -m pip uninstall -y vmb || true

dir:
	mkdir -p $(OUTDIR)

vmb-build: dir uninstall
	rm -rf $(OUTDIR) || true
	$(BIN_PY)  -c 'import build' || $(BIN_PY)  -m pip install build
	$(BIN_PY)  -m build --outdir=$(OUTDIR)
	$(BIN_PY)  -m pip install $(OUTDIR)/vmb-*-py3-none-any.whl

vmb: vmb-build clean

help:
	$(VMB) help || true
	$(VMB) version
	$(VMB) list

tox: dir
	$(BIN_PY)  -m tox --workdir=$(OUTDIR)
	rm -rf src/vmb.egg-info/

