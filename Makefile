ifdef ENVIRONMENT
    ENV_FLAG := -e $(ENVIRONMENT)
endif

extra_files = lib/PWMSine/sinlut.h include/version.h

.PHONY: all
all:            ## Compile firmware.
all: $(extra_files)
	pio run $(ENV_FLAG)

.PHONY: upload
upload: $(extra_files)
upload:         ## Upload firmware.
	pio run $(ENV_FLAG) --target upload

.PHONY: bootloader
bootloader: $(extra_files)
bootloader:     ## Burn fuses and bootloader.
	pio run $(ENV_FLAG) --target bootloader

.PHONY: clean
clean:          ## Clean project.
# Not cleaning extra_files, because they are currently tracked by git.
	pio run $(ENV_FLAG) --target clean
	-rm include/version.h

.PHONY: update
update:         ## Update PlatformIO core and libraries.
	pio update

.PHONY: help
help:           ## Show this help.
	@grep -F -h "##" $(MAKEFILE_LIST) | sed -e '/unique_BhwaDzu7C/d;s/\\$$//;s/##//'

.PHONY: docs
docs:           ## Generate Doxygen documentation.
docs: $(extra_files)
	doxygen

.PHONY: check
check:          ## Run static code analysis.
check: $(extra_files)
	pio check $(ENV_FLAG)

.PHONY: test
test:           ## Run native tests.
test: $(extra_files)
	pio test -e native

.PHONY: test_embedded
test_embedded:  ## Run tests on the embedded device.
test_embedded: $(extra_files)
	pio test $(ENV_FLAG)

lib/PWMSine/sinlut.h: lib/PWMSine/sinlut.py
	cd lib/PWMSine && ./sinlut.py

# This needs to be regenerated on every run to be able to
# detect the -dirty state.
.PHONY: include/version.h
include/version.h:
	echo "#define AlarmClock_version \"$(shell git describe --dirty --always --tags)\"" > include/version.h
	echo "#define AlarmClock_build_time \"$(shell date --utc +"%Y%m%dT%H%M%SZ")\"" >> include/version.h
