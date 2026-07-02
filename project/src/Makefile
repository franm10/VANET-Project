#
# OMNeT++/OMNEST Makefile for $(LIB_PREFIX)NetworkProjectVanet
#
# This file was generated with the command:
#  opp_makemake --make-so -f --deep -KINET_PROJ=../../inet -KVEINS_PROJ=../../veins -DINET_IMPORT -DVEINS_IMPORT -I$$\(INET_PROJ\)/src -I$$\(VEINS_PROJ\)/src -I. -L$$\(INET_PROJ\)/src -L$$\(VEINS_PROJ\)/src -lINET$$\(D\) -lveins$$\(D\)
#

# Name of target to be created (-o option)
TARGET = $(LIB_PREFIX)NetworkProjectVanet$(D)$(SHARED_LIB_SUFFIX)
TARGET_DIR = .

# C++ include paths (with -I)
INCLUDE_PATH = -I$(INET_PROJ)/src -I$(VEINS_PROJ)/src -I.

# Additional object and library files to link with
EXTRA_OBJS =

# Additional libraries (-L, -l options)
LIBS = $(LDFLAG_LIBPATH)$(INET_PROJ)/src $(LDFLAG_LIBPATH)$(VEINS_PROJ)/src  -lINET$(D) -lveins$(D)

# Output directory
PROJECT_OUTPUT_DIR = ../out
PROJECTRELATIVE_PATH = src
O = $(PROJECT_OUTPUT_DIR)/$(CONFIGNAME)/$(PROJECTRELATIVE_PATH)

# Object files for local .cc, .msg and .sm files
OBJS = \
    $O/grayhole/grayhole_attack/AodvGrayhole.o \
    $O/grayhole/grayhole_attackwithtrust/AttackerTrustAwareAODV.o \
    $O/grayhole/grayhole_mitigation/TrustAwareAODV.o \
    $O/grayhole/grayhole_mitigation/TrustManager.o \
    $O/grayhole/grayhole_mitigation/TrustMonitor.o \
    $O/replay/replay_attack/AodvReplay.o \
    $O/replay/replay_mitigation_ecdsa/bloom_filter/EcdsaBloomFilterAodvReplay.o \
    $O/replay/replay_mitigation_ecdsa/map_table/EcdsaAodvReplay.o \
    $O/replay/replay_mitigation_ecdsa/utils/EcdsaUtils.o \
    $O/replay/replay_mitigation_rsa/bloom_filter/RsaBloomFilterAodvReplay.o \
    $O/replay/replay_mitigation_rsa/map_table/RsaAodvReplay.o \
    $O/replay/replay_mitigation_rsa/utils/RsaUtils.o \
    $O/replay/utils/BloomFilter.o \
    $O/veins_inet/VeinsInetApplicationBase.o \
    $O/veins_inet/VeinsInetManager.o \
    $O/veins_inet/VeinsInetManagerBase.o \
    $O/veins_inet/VeinsInetManagerForker.o \
    $O/veins_inet/VeinsInetMobility.o \
    $O/veins_inet/VeinsInetSampleApplication.o \
    $O/grayhole/grayhole_mitigation/messages/IntermediateAck_m_m.o \
    $O/grayhole/grayhole_mitigation/messages/TrustBeacon_m.o \
    $O/replay/replay_mitigation_ecdsa/messages/EcdsaAodvControlPackets_m.o \
    $O/veins_inet/VeinsInetSampleMessage_m.o

# Message files
MSGFILES = \
    grayhole/grayhole_mitigation/messages/IntermediateAck_m.msg \
    grayhole/grayhole_mitigation/messages/TrustBeacon.msg \
    replay/replay_mitigation_ecdsa/messages/EcdsaAodvControlPackets.msg \
    veins_inet/VeinsInetSampleMessage.msg

# SM files
SMFILES =

# Other makefile variables (-K)
INET_PROJ=../../inet
VEINS_PROJ=../../veins

#------------------------------------------------------------------------------

# Pull in OMNeT++ configuration (Makefile.inc)

ifneq ("$(OMNETPP_CONFIGFILE)","")
CONFIGFILE = $(OMNETPP_CONFIGFILE)
else
ifneq ("$(OMNETPP_ROOT)","")
CONFIGFILE = $(OMNETPP_ROOT)/Makefile.inc
else
CONFIGFILE = $(shell opp_configfilepath)
endif
endif

ifeq ("$(wildcard $(CONFIGFILE))","")
$(error Config file '$(CONFIGFILE)' does not exist -- add the OMNeT++ bin directory to the path so that opp_configfilepath can be found, or set the OMNETPP_CONFIGFILE variable to point to Makefile.inc)
endif

include $(CONFIGFILE)

# Simulation kernel and user interface libraries
OMNETPP_LIBS = -loppenvir$D $(KERNEL_LIBS) $(SYS_LIBS)
ifneq ($(TOOLCHAIN_NAME),clangc2)
LIBS += -Wl,-rpath,$(abspath $(INET_PROJ)/src) -Wl,-rpath,$(abspath $(VEINS_PROJ)/src)
endif

COPTS = $(CFLAGS) $(IMPORT_DEFINES) -DINET_IMPORT -DVEINS_IMPORT $(INCLUDE_PATH) -I$(OMNETPP_INCL_DIR)
MSGCOPTS = $(INCLUDE_PATH)
SMCOPTS =

# we want to recompile everything if COPTS changes,
# so we store COPTS into $COPTS_FILE and have object
# files depend on it (except when "make depend" was called)
COPTS_FILE = $O/.last-copts
ifneq ("$(COPTS)","$(shell cat $(COPTS_FILE) 2>/dev/null || echo '')")
$(shell $(MKPATH) "$O" && echo "$(COPTS)" >$(COPTS_FILE))
endif

#------------------------------------------------------------------------------
# User-supplied makefile fragment(s)
# >>>
# inserted from file 'makefrag':
MSGC:=$(MSGC) --msg6
EXTRA_CFLAGS  += -I$(PROJECT_DIR)/src/utils
LIBS += -lssl -lcrypto

# <<<
#------------------------------------------------------------------------------

# Main target
all: $(TARGET_DIR)/$(TARGET)

$(TARGET_DIR)/% :: $O/%
	@mkdir -p $(TARGET_DIR)
	$(Q)$(LN) $< $@
ifeq ($(TOOLCHAIN_NAME),clangc2)
	$(Q)-$(LN) $(<:%.dll=%.lib) $(@:%.dll=%.lib)
endif

$O/$(TARGET): $(OBJS)  $(wildcard $(EXTRA_OBJS)) Makefile $(CONFIGFILE)
	@$(MKPATH) $O
	@echo Creating shared library: $@
	$(Q)$(SHLIB_LD) -o $O/$(TARGET) $(OBJS) $(EXTRA_OBJS) $(AS_NEEDED_OFF) $(WHOLE_ARCHIVE_ON) $(LIBS) $(WHOLE_ARCHIVE_OFF) $(OMNETPP_LIBS) $(LDFLAGS)
	$(Q)$(SHLIB_POSTPROCESS) $O/$(TARGET)

.PHONY: all clean cleanall depend msgheaders smheaders

.SUFFIXES: .cc

$O/%.o: %.cc $(COPTS_FILE) | msgheaders smheaders
	@$(MKPATH) $(dir $@)
	$(qecho) "$<"
	$(Q)$(CXX) -c $(CXXFLAGS) $(COPTS) -o $@ $<

%_m.cc %_m.h: %.msg
	$(qecho) MSGC: $<
	$(Q)$(MSGC) -s _m.cc -MD -MP -MF $O/$(basename $<)_m.h.d $(MSGCOPTS) $?

%_sm.cc %_sm.h: %.sm
	$(qecho) SMC: $<
	$(Q)$(SMC) -c++ -suffix cc $(SMCOPTS) $?

msgheaders: $(MSGFILES:.msg=_m.h)

smheaders: $(SMFILES:.sm=_sm.h)

clean:
	$(qecho) Cleaning $(TARGET)
	$(Q)-rm -rf $O
	$(Q)-rm -f $(TARGET_DIR)/$(TARGET)
	$(Q)-rm -f $(TARGET_DIR)/$(TARGET:%.dll=%.lib)
	$(Q)-rm -f $(call opp_rwildcard, . , *_m.cc *_m.h *_sm.cc *_sm.h)

cleanall:
	$(Q)$(MAKE) -s clean MODE=release
	$(Q)$(MAKE) -s clean MODE=debug
	$(Q)-rm -rf $(PROJECT_OUTPUT_DIR)

# include all dependencies
-include $(OBJS:%=%.d) $(MSGFILES:%.msg=$O/%_m.h.d)
