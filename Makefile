# Makefile wrapper for CMake build

.PHONY: build install clean

build:
	@cmake -B bin -S . -DCMAKE_BUILD_TYPE=Release
	@cmake --build bin

install: build
	@mkdir -p ~/.smashedit
	@cp bin/smashedit ~/.smashedit/
	@echo "Installed to ~/.smashedit/smashedit"

clean:
	@rm -rf bin
	@echo "Cleaned build artifacts"
