NAME := suas_drivers

COMPONENT_ADD_INCLUDEDIRS += include rfid/src
COMPONENT_PRIV_INCLUDEDIRS += private

COMPONENT_SRCS += \
	74hc595.c \
	adc.c \
	dht22.c \
	dht22_asm.S \
	grove_dls.c \
	i2c.c \
	mmWave.c \
	pwm.c \
	ssd1306.c \
	ultrasonic.c \
	ultrasonic_asm.S \
	arduino.cpp \
	serial.cpp \
	rfid.cpp \
	spi.cpp \
	rfid/src/MFRC522.cpp
	
COMPONENT_SRCDIRS := . rfid/src
COMPONENT_OBJS := $(COMPONENT_SRCS:.S=.o)
COMPONENT_OBJS := $(COMPONENT_OBJS:.c=.o)
COMPONENT_OBJS := $(COMPONENT_OBJS:.cpp=.o)