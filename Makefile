.PHONY: monitor upload build

build:
	@pio run

monitor:
	@pio device monitor

upload:
	@pio run --target upload