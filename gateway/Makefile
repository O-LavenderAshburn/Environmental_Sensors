# Set RIOT base
RIOTBASE ?= ../RIOT

# Name of application
APPLICATION = gateway

# Target board
BOARD ?= nrf52dk

# We use the xtimer and the shell in this example
USEMODULE += shell

# configure and use Nimble
USEPKG += nimble
USEMODULE += nimble_svc_gap
USEMODULE += nimble_scanner
USEMODULE += ztimer
USEMODULE += ztimer_sec
USEMODULE += nimble_scanlist
USEMODULE += printf_float

DEBUG ?= 0

DEVELHELP ?= 1

# Change this to 0 show compiler invocation lines by default:
QUIET ?=NimBLE Environmental Sensing Service Example
include $(RIOTBASE)/Makefile.include