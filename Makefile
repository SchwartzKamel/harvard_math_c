BUILD_DIR ?= build
PREFIX ?= /usr/local
GENERATOR ?= Ninja
BUILD_TYPE ?= Debug
BUILD_SHARED_LIBS ?= OFF
HM_BUILD_TESTS ?= ON
HM_BUILD_EXAMPLES ?= ON
HM_BUILD_BENCHMARKS ?= OFF
GSL ?= OFF
HM_BENCHMARK_GSL ?= $(GSL)
BENCHMARK_BUILD_TYPE ?= Release
CMAKE ?= cmake
CTEST ?= ctest
CMAKE_CONFIGURE_FLAGS ?=
PACKAGE_PREFIX := $(abspath $(BUILD_DIR)/package-prefix)
PACKAGE_BUILD_DIR := $(abspath $(BUILD_DIR)/package-consumer)

CMAKE_OPTIONS = \
	-G "$(GENERATOR)" \
	-DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
	-DBUILD_SHARED_LIBS=$(BUILD_SHARED_LIBS) \
	-DHM_BUILD_TESTS=$(HM_BUILD_TESTS) \
	-DHM_BUILD_EXAMPLES=$(HM_BUILD_EXAMPLES) \
	-DHM_BUILD_BENCHMARKS=$(HM_BUILD_BENCHMARKS) \
	-DHM_BENCHMARK_GSL=$(HM_BENCHMARK_GSL) \
	$(CMAKE_CONFIGURE_FLAGS)

.PHONY: all configure build test example benchmark install package-test clean help

all: build

configure:
	$(CMAKE) -S . -B "$(BUILD_DIR)" $(CMAKE_OPTIONS)

build: configure
	$(CMAKE) --build "$(BUILD_DIR)"

test: build
	$(CTEST) --test-dir "$(BUILD_DIR)" --output-on-failure

example: build
	"$(BUILD_DIR)/examples/harvard_math_example"

benchmark:
	$(CMAKE) -S . -B "$(BUILD_DIR)" $(CMAKE_OPTIONS) -DHM_BUILD_BENCHMARKS=ON -DCMAKE_BUILD_TYPE=$(BENCHMARK_BUILD_TYPE)
	$(CMAKE) --build "$(BUILD_DIR)"
	"$(BUILD_DIR)/bench/harvard_math_benchmark" --csv "$(BUILD_DIR)/benchmark.csv"

install: build
	DESTDIR="$(DESTDIR)" $(CMAKE) --install "$(BUILD_DIR)" --prefix "$(PREFIX)"

package-test: build
	rm -rf "$(PACKAGE_PREFIX)" "$(PACKAGE_BUILD_DIR)"
	$(CMAKE) --install "$(BUILD_DIR)" --prefix "$(PACKAGE_PREFIX)"
	$(CMAKE) -S tests/package_consumer -B "$(PACKAGE_BUILD_DIR)" -G "$(GENERATOR)" -DCMAKE_PREFIX_PATH="$(PACKAGE_PREFIX)"
	$(CMAKE) --build "$(PACKAGE_BUILD_DIR)"
	$(CTEST) --test-dir "$(PACKAGE_BUILD_DIR)" --output-on-failure

clean:
	rm -rf "$(BUILD_DIR)"

help:
	@printf '%s\n' \
		'make build       Configure with CMake/Ninja and compile the library' \
		'make test        Build and run CTest' \
		'make example     Build and run the example' \
		'make benchmark   Build and run the optimized reference comparison' \
		'make package-test Install to build/package-prefix and test an external consumer' \
		'make install     Install using PREFIX=/path (default /usr/local)' \
		'make clean       Remove the selected build directory' \
		'Variables: BUILD_DIR, PREFIX, GENERATOR, BUILD_TYPE, BENCHMARK_BUILD_TYPE, BUILD_SHARED_LIBS, GSL, CMAKE_CONFIGURE_FLAGS'
