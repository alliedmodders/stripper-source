# (C)2004-2008 SourceMod Development Team
# Makefile written by David "BAILOPAN" Anderson

SMSDK = ..
HL2SDK_ORIG = ../../hl2sdk
HL2SDK_OB = ../../hl2sdk-ob
HL2SDK_OB_VALVE = ../../hl2sdk-ob-valve
HL2SDK_L4D = ../../hl2sdk-l4d
HL2SDK_L4D2 = ../../hl2sdk-l4d2
MMSOURCE17 = ../../mmsource-1.7
MMSOURCE18 = ../../mmsource-1.8

#####################################
### EDIT BELOW FOR OTHER PROJECTS ###
#####################################

OBJECTS = stripper_mm.cpp

##############################################
### CONFIGURE ANY OTHER FLAGS/OPTIONS HERE ###
##############################################

C_OPT_FLAGS = -DNDEBUG -O3 -funroll-loops -pipe -fno-strict-aliasing
C_DEBUG_FLAGS = -D_DEBUG -DDEBUG -g -ggdb3
C_GCC4_FLAGS = -fvisibility=hidden
CPP_GCC4_FLAGS = -fvisibility-inlines-hidden
CPP = gcc-4.1

override ENGSET = false
ifeq "$(ENGINE)" "original"
	HL2SDK = $(HL2SDK_ORIG)
	HL2PUB = $(HL2SDK)/public
	HL2LIB = $(HL2SDK)/linux_sdk
	CFLAGS += -DSOURCE_ENGINE=1
	METAMOD = $(MMSOURCE17)/core-legacy
	INCLUDE += -I$(HL2SDK)/public/dlls
	BINARY = stripper.14.ep1.so
	LIB_SUFFIX = _i486.so
	override ENGSET = true
endif
ifeq "$(ENGINE)" "orangebox"
	HL2SDK = $(HL2SDK_OB)
	HL2PUB = $(HL2SDK)/public
	HL2LIB = $(HL2SDK)/lib/linux
	CFLAGS += -DSOURCE_ENGINE=3 -DORANGEBOX_BUILD
	METAMOD = $(MMSOURCE17)/core
	INCLUDE += -I$(HL2SDK)/public/game/server
	BINARY = stripper.16.ep2.so
	LIB_SUFFIX = _i486.so
	override ENGSET = true
endif
ifeq "$(ENGINE)" "orangeboxvalve"
	HL2SDK = $(HL2SDK_OB_VALVE)
	HL2PUB = $(HL2SDK)/public
	HL2LIB = $(HL2SDK)/lib/linux
	CFLAGS += -DSOURCE_ENGINE=3 -DORANGEBOX_BUILD
	METAMOD = $(MMSOURCE17)/core
	INCLUDE += -I$(HL2SDK)/public/game/server
	BINARY = stripper.16.ep2v.so
	LIB_PREFIX = lib
	LIB_SUFFIX = .so
	override ENGSET = true
endif
ifeq "$(ENGINE)" "left4dead"
	HL2SDK = $(HL2SDK_L4D)
	HL2PUB = $(HL2SDK)/public
	HL2LIB = $(HL2SDK)/lib/linux
	CFLAGS += -DSOURCE_ENGINE=4
	METAMOD = $(MMSOURCE17)/core
	INCLUDE += -I$(HL2SDK)/public/game/server
	BINARY = stripper.16.l4d.so
	LIB_SUFFIX = _i486.so
	override ENGSET = true
endif
ifeq "$(ENGINE)" "left4dead2"
	HL2SDK = $(HL2SDK_L4D2)
	HL2PUB = $(HL2SDK)/public
	HL2LIB = $(HL2SDK)/lib/linux
	CFLAGS += -DSOURCE_ENGINE=7
	METAMOD = $(MMSOURCE18)/core
	INCLUDE += -I$(HL2SDK)/public/game/server
	BINARY = stripper.16.l4d2.so
	LIB_PREFIX = lib
	LIB_SUFFIX = .so
	override ENGSET = true
endif

CFLAGS += -DSE_EPISODEONE=1 -DSE_DARKMESSIAH=2 -DSE_ORANGEBOX=3 -DSE_LEFT4DEAD=4 -DSE_LEFT4DEAD2=7

LINK += $(HL2LIB)/tier1_i486.a $(HL2LIB)/mathlib_i486.a \
		$(LIB_PREFIX)vstdlib$(LIB_SUFFIX) \
		$(LIB_PREFIX)tier0$(LIB_SUFFIX) \
		-lpthread -static-libgcc
	
INCLUDE += -I. -I$(HL2PUB) -I$(HL2PUB)/engine -I$(HL2PUB)/mathlib -I$(HL2PUB)/vstdlib \
	-I$(HL2PUB)/tier0 -I$(HL2PUB)/tier1 -I$(METAMOD) -I$(METAMOD)/sourcehook \
	-I$(SMSDK)/public -I$(SMSDK)/public/sourcepawn

CFLAGS += -D_LINUX -Dstricmp=strcasecmp -D_stricmp=strcasecmp -D_strnicmp=strncasecmp -Dstrnicmp=strncasecmp \
	-D_snprintf=snprintf -D_vsnprintf=vsnprintf -D_alloca=alloca -Dstrcmpi=strcasecmp -Wall -Werror \
	-Wno-uninitialized -mfpmath=sse -msse -DSOURCEMOD_BUILD -DHAVE_STDINT_H -DSM_DEFAULT_THREADER -m32
CPPFLAGS += -Wno-non-virtual-dtor -fno-exceptions -fno-rtti

################################################
### DO NOT EDIT BELOW HERE FOR MOST PROJECTS ###
################################################

ifeq "$(DEBUG)" "true"
	BIN_DIR = Debug.$(ENGINE)
	CFLAGS += $(C_DEBUG_FLAGS)
else
	BIN_DIR = Release.$(ENGINE)
	CFLAGS += $(C_OPT_FLAGS)
endif

GCC_VERSION := $(shell $(CPP) -dumpversion >&1 | cut -b1)
ifeq "$(GCC_VERSION)" "4"
	CFLAGS += $(C_GCC4_FLAGS)
	CPPFLAGS += $(CPP_GCC4_FLAGS)
endif

OBJ_LINUX := $(OBJECTS:%.cpp=$(BIN_DIR)/%.o)
OBJ_LINUX := $(OBJ_LINUX:%.c=$(BIN_DIR)/%.o)

$(BIN_DIR)/%.o: %.cpp
	$(CPP) $(INCLUDE) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

$(BIN_DIR)/%.o: %.c
	$(CPP) $(INCLUDE) $(CFLAGS) -o $@ -c $<

all: check
	mkdir -p $(BIN_DIR)/systems
	mkdir -p $(BIN_DIR)/thread
	ln -sf $(HL2LIB)/$(LIB_PREFIX)vstdlib$(LIB_SUFFIX) .;
	ln -sf $(HL2LIB)/$(LIB_PREFIX)tier0$(LIB_SUFFIX) .;
	$(MAKE) -f Makefile sourcemod

check:
	if [ "$(ENGSET)" = "false" ]; then \
		echo "You must supply ENGINE=left4dead or ENGINE=orangebox or ENGINE=original"; \
		exit 1; \
	fi

sourcemod: check $(OBJ_LINUX)
	$(CPP) $(INCLUDE) $(OBJ_LINUX) $(LINK) -m32 -shared -ldl -lm -o$(BIN_DIR)/$(BINARY)

debug:
	$(MAKE) -f Makefile all DEBUG=true

default: all

clean: check
	rm -rf $(BIN_DIR)/*.o
	rm -rf $(BIN_DIR)/systems/*.o
	rm -rf $(BIN_DIR)/thread/*.o
	rm -rf $(BIN_DIR)/$(BINARY)
