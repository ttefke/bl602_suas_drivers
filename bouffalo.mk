NAME := suas_drivers

COMPONENT_ADD_INCLUDEDIRS += include

ifeq ($(CONFIG_SUAS_WITH_ADC), 1)
COMPONENT_SRCS += \
	adc.c
endif

ifeq ($(CONFIG_SUAS_WITH_DHT22), 1)
COMPONENT_SRCS += \
	dht22.c \
	dht22_asm.S
endif

COMPONENT_SRCDIRS := .
COMPONENT_OBJS := $(pathsubst %.S,%.o, $(COMPONENT_SRCS))
COMPONENT_OBJS := $(pathsubst %.c,%.o, $(COMPONENT_SRCS))