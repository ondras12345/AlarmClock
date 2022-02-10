.PHONY: all upload clean upload help docs check test test_embedded

ifdef ENVIRONMENT
    ENV_FLAG := -e $(ENVIRONMENT)
endif


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

docs:           ## Generate Doxygen documentation.
	doxygen

check:          ## Run static code analysis.
	pio check $(ENV_FLAG)

test:           ## Run native tests.
	pio test -e native

test_embedded:  ## Run tests on the embedded device.
	pio test $(ENV_FLAG)
