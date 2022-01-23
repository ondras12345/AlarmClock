.PHONY: all upload clean upload help docs test test_embedded

all:        ## Compile firmware.
	pio run

upload:     ## Upload firmware.
	pio run --target upload

clean:      ## Clean project.
	pio run --target clean

update:     ## Update PlatformIO core and libraries.
	pio update

help:       ## Show this help.
	@grep -F -h "##" $(MAKEFILE_LIST) | sed -e '/unique_BhwaDzu7C/d;s/\\$$//;s/##//'

docs:       ## Generate Doxygen documentation.
	doxygen

test:       ## Run native tests.
	pio test -e native

test_embedded:  ## Run tests on the embedded device.
	pio test
