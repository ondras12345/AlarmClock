.PHONY: all upload clean upload help docs check test test_embedded

ifdef ENVIRONMENT
    ENV_FLAG := -e $(ENVIRONMENT)
endif


all: lib/PWMSine/sinlut.h
all:            ## Compile firmware.
	pio run $(ENV_FLAG)

upload:         ## Upload firmware.
	pio run $(ENV_FLAG) --target upload

clean:          ## Clean project.
	pio run $(ENV_FLAG) --target clean

update:         ## Update PlatformIO core and libraries.
	pio update

help:           ## Show this help.
	@grep -F -h "##" $(MAKEFILE_LIST) | sed -e '/unique_BhwaDzu7C/d;s/\\$$//;s/##//'

docs: lib/PWMSine/sinlut.h
docs:           ## Generate Doxygen documentation.
	doxygen

check: lib/PWMSine/sinlut.h
check:          ## Run static code analysis.
	pio check $(ENV_FLAG)

test: lib/PWMSine/sinlut.h
test:           ## Run native tests.
	pio test -e native

test_embedded: lib/PWMSine/sinlut.h
test_embedded:  ## Run tests on the embedded device.
	pio test $(ENV_FLAG)

lib/PWMSine/sinlut.h: lib/PWMSine/sinlut.py
	cd lib/PWMSine && ./sinlut.py
