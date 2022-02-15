ifdef ENVIRONMENT
    ENV_FLAG := -e $(ENVIRONMENT)
endif

extra_files = lib/PWMSine/sinlut.h

.PHONY: all
all:            ## Compile firmware.
all: $(extra_files)
	pio run $(ENV_FLAG)

.PHONY: upload
upload: $(extra_files)
upload:         ## Upload firmware.
	pio run $(ENV_FLAG) --target upload

.PHONY: clean
clean:          ## Clean project.
	pio run $(ENV_FLAG) --target clean

.PHONY: update
update:         ## Update PlatformIO core and libraries.
	pio update

.PHONY: help
help:           ## Show this help.
	@grep -F -h "##" $(MAKEFILE_LIST) | sed -e '/unique_BhwaDzu7C/d;s/\\$$//;s/##//'

.PHONY: docs
docs:           ## Generate Doxygen documentation.
# Not cleaning extra_files, because they are currently tracked by git.
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
