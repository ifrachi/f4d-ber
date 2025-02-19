CONTIKI_PROJECT = ber
all: $(CONTIKI_PROJECT)


TARGET = simplelink
BOARD = launchpad/cc2650
CORE_SDK=../coresdk_cc13xx_cc26xx

CFLAGS += -I$(CORE_SDK)/include
LDFLAGS += -L$(CORE_SDK)/lib
MAKE_MAC = MAKE_MAC_TSCH
MAKE_ROUTING = MAKE_ROUTING_RPL_CLASSIC
MAKE_NET = MAKE_NET_IPV6
MODULES += os/services/orchestra
# MODULES += os/services/simple-energest
MODULES += os/services/shell

CFLAGS += -w


CONTIKI=../../../contiki-ng

include $(CONTIKI)/Makefile.include
