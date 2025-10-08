# Set RIOT base
RIOTBASE ?= ../RIOT

# Name of application
APPLICATION ?= sensor

# Target board
BOARD ?= nrf52dk


# Include NimBLE
USEPKG += nimble
USEMODULE += nimble_svc_gap
USEMODULE += nimble_svc_gatt
USEMODULE += hts221
# Use automated advertising
USEMODULE += nimble_autoadv
CFLAGS += -DCONFIG_NIMBLE_AUTOADV_DEVICE_NAME='"NimBLE GATT Example"'
CFLAGS += -DCONFIG_NIMBLE_AUTOADV_START_MANUALLY=1
SENSOR_TYPE ?= 0

# Pass it to the compiler
CFLAGS += -DSENSOR_TYPE=$(SENSOR_TYPE)

DEVELHELP ?= 1

# Change this to 0 show compiler invocation lines by default:
CFLAGS_MODULE += -Wno-error=implicit-function-declaration

include $(RIOTBASE)/Makefile.include
