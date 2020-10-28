SUBDIRS = include src tests
AM_LDFLAGS = -shared -O3
AUTOMAKE_OPTIONS = subdir-objects
include Makefile.am.coverage

flash:
	arm-none-eabi-size $(top_builddir)/src/rtpython
	arm-none-eabi-objcopy -O binary $(top_builddir)/src/rtpython $(top_builddir)/src/rtpython.hex
	$(RTPYTHON_TARGET_CMD_FLASH)

EXTRA_DIST =                   \
        rust/build.rs            \
        rust/Cargo.toml          \
        rust/src/lib.rs\
        rust/Cargo.lock

if DEBUG_RELEASE
CARGO_RELEASE_ARGS=
else
CARGO_RELEASE_ARGS=--release
endif
CARGO_VERBOSE = $(cargo_verbose_$(V))
cargo_verbose_ = $(cargo_verbose_$(AM_DEFAULT_VERBOSITY))
cargo_verbose_0 =
cargo_verbose_1 = --verbose
RUST_LIB=@abs_top_builddir@/rust/target/@RUST_TARGET_SUBDIR@/librtpython_rust.a
#@abs_top_builddir@/rust/target/@RUST_TARGET_SUBDIR@/librtpython_rust.a:
all-local-dis: $(EXTRA_DIST)
	cd $(top_srcdir)/rust && \
	rustc --crate-type staticlib -o @abs_top_builddir@/rust/librs_rust.a @abs_top_srcdir@/src/rust/rust.rs
#CARGO_TARGET_DIR=@abs_top_builddir@/rust/target cargo build --target thumbv7m-none-eabi $(CARGO_VERBOSE) $(CARGO_RELEASE_ARGS)
check-local:
	cd $(srcdir)/rust && \
	CARGO_TARGET_DIR=@abs_top_builddir@/rust/target cargo test
clean-local:
	cd $(top_srcdir)/rust && \
	CARGO_TARGET_DIR=@abs_top_builddir@/rust/target cargo clean

dist-hook:
	(cd $(distdir)/rust && \
	cargo vendor -q && \
	mkdir .cargo && \
	cp cargo-vendor-config .cargo/config)
