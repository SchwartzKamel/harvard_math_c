BUILD_DIR ?= build
PREFIX ?= /usr/local
GENERATOR ?= Ninja
BUILD_TYPE ?= Debug
BUILD_SHARED_LIBS ?= OFF
HM_BUILD_TESTS ?= ON
HM_BUILD_EXAMPLES ?= ON
CMAKE ?= cmake
CTEST ?= ctest
CMAKE_CONFIGURE_FLAGS ?=

CMAKE_OPTIONS = \
	-G "$(GENERATOR)" \
	-DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
	-DBUILD_SHARED_LIBS=$(BUILD_SHARED_LIBS) \
	-DHM_BUILD_TESTS=$(HM_BUILD_TESTS) \
	-DHM_BUILD_EXAMPLES=$(HM_BUILD_EXAMPLES) \
	$(CMAKE_CONFIGURE_FLAGS)

.PHONY: all configure build test example install package-test clean help

all: build

configure:
	$(CMAKE) -S . -B "$(BUILD_DIR)" $(CMAKE_OPTIONS)

build: configure
	$(CMAKE) --build "$(BUILD_DIR)"

test: build
	$(CTEST) --test-dir "$(BUILD_DIR)" --output-on-failure

example: build
	"$(BUILD_DIR)/examples/harvard_math_example"

install: build
	DESTDIR="$(DESTDIR)" $(CMAKE) --install "$(BUILD_DIR)" --prefix "$(PREFIX)"

package-test: build
	rm -rf "$(BUILD_DIR)/package-prefix" "$(BUILD_DIR)/package-consumer"
	$(CMAKE) --install "$(BUILD_DIR)" --prefix "$(CURDIR)/$(BUILD_DIR)/package-prefix"
	$(CMAKE) -S tests/package_consumer -B "$(BUILD_DIR)/package-consumer" -G "$(GENERATOR)" -DCMAKE_PREFIX_PATH="$(CURDIR)/$(BUILD_DIR)/package-prefix"
	$(CMAKE) --build "$(BUILD_DIR)/package-consumer"
	$(CTEST) --test-dir "$(BUILD_DIR)/package-consumer" --output-on-failure

clean:
	rm -rf "$(BUILD_DIR)"

help:
	@printf '%s\n' \
		'make build       Configure with CMake/Ninja and compile the library' \
		'make test        Build and run CTest' \
		'make example     Build and run the example' \
		'make package-test Install to build/package-prefix and test an external consumer' \
		'make install     Install using PREFIX=/path (default /usr/local)' \
		'make clean       Remove the selected build directory' \
		'Variables: BUILD_DIR, PREFIX, GENERATOR, BUILD_TYPE, BUILD_SHARED_LIBS, CMAKE_CONFIGURE_FLAGS'
