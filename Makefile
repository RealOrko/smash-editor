# Makefile wrapper for CMake build

.PHONY: build install clean

build:
	@if [ -f bin/CMakeCache.txt ] && ! grep -q "$(CURDIR)" bin/CMakeCache.txt 2>/dev/null; then \
		rm -rf bin; \
	fi
	@cmake -B bin -S . -DCMAKE_BUILD_TYPE=Release 2>/dev/null || (rm -rf bin && cmake -B bin -S . -DCMAKE_BUILD_TYPE=Release)
	@cmake --build bin

install: build
	@mkdir -p ~/.smashedit
	@cp bin/smashedit ~/.smashedit/
	@echo "Installed to ~/.smashedit/smashedit"

clean:
	@rm -rf bin
	@echo "Cleaned build artifacts"

