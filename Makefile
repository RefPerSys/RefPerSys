
RPS_GIT_ID:= $(shell ./generate-gitid.sh)

RPS_CORE_HEADERS = $(sort $(wildcard *_rps.hh))
RPS_CORE_SOURCES = $(sort $(wildcard *_rps.cc))
RPS_CORE_OBJECTS = $(patsubst %.cc, %.o, $(RPS_CORE_SOURCES))

RPS_QT_HEADERS = $(sort $(wildcard *_qrps.cc))
RPS_QT_SOURCES = $(sort $(wildcard *_qrps.hh))
RPS_QT_OBJECTS = $(patsubst $(RPS_QT_SOURCES))
RPS_QT_MOC = moc

RPS_BUILD_CCACHE = ccache
RPS_BUILD_CC = gcc
RPS_BUILD_CXX = g++
RPS_BUILD_DIALECTFLAGS = -std=gnu++17
RPS_BUILD_WARNFLAGS = -Wall Wextra
RPS_BUILD_OPTIMFLAGS = -O1 -g3

RPS_INCLUDE_DIRS = /usr/local/include
RPS_INCLUDE_FLAGS = $(patsubst %, -I %, $(RPS_INCLUDE_DIRS))

RPS_PKG_CONFIG = pkg-config
RPS_PKG_NAMES = Qt5Core Qt5Gui Qt5Widgets Qt5Network jsoncpp
RPS_PKG_CFLAGS = $(sehll $(RPS_PKG_CONFIG) --cflags $(RPS_PKG_NAMES))
RPS_PKG_LIBS = $(shell $(RPS_PKG_CONFIG) -- libs $(RPS_PKG_NAMES))

CC = $(RPS_BUILD_CCACHE) $(RPS_BUILD_CC)
CXX = $(RPS_BUILD_CCACHE) $(RPS_BUILD_CXX)
CXXFLAGS += $(RPS_BUILD_DIALECTFLAGS) $(RPS_BUILD_OPTIMFLAGS) \
	    $(RPS_BUILD_WARNFLAGS) $(RPS_BUILD_INCLUDE_FLAGS) \
	    $(RPS_PKG_CFLAGS) -DRPS_GITID=$(RPS_GIT_ID)

all: refpersys

refpersys: $(RPS_CORE_OBJECTS) $(PRS_QT_OBJECTS)

%.o: %.cc$
	$(CXX) $(CXXFLAGS) -o $@ $<

