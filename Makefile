CXX      ?= g++
AR       ?= ar
CXXFLAGS ?= -std=c++0x -O2 -g3
CXXFLAGS += -Wall

OGRE_CFLAGS ?= $(shell pkg-config --cflags OGRE)
OGRE_PLUGIN_DIR ?= $(shell pkg-config --variable=plugindir OGRE)
CXXFLAGS += $(OGRE_CFLAGS) -DOGRE_PLUGIN_DIR="\"$(OGRE_PLUGIN_DIR)\""
CXXFLAGS += $(shell pkg-config --cflags OIS)

OGRE_LDFLAGS ?= $(shell pkg-config --libs OGRE)
LDFLAGS  += $(OGRE_LDFLAGS)
LDFLAGS  += $(shell pkg-config --libs OIS)
LDFLAGS  += -lnoise


BINDIR  = bin
BINNAME = army
BIN     = $(BINDIR)/$(BINNAME)

SRCDIR = src

SRCFILES = MilitaryUnitAI.cpp PlatoonAI.cpp MilitaryUnit.cpp Army.cpp Messaging.cpp Papaya.cpp Terrain.cpp App.cpp main.cpp

SRCS = $(addprefix $(SRCDIR)/, $(SRCFILES))
OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.dep)

.PHONY: clean all

all: $(BIN)

$(BINDIR):
	mkdir -p $(BINDIR)

$(BIN): $(BINDIR) $(OBJS)
	$(CXX) $(LDFLAGS) $(OBJS) -o $(BIN)

%.dep: %.cpp
	@rm -f $@
	@$(CC) -MM $(CPPFLAGS) $< > $@.P
	@sed 's,\($(notdir $*)\)\.o[ :]*,$(dir $*)\1.o $@ : ,g' < $@.P > $@
	@rm -f $@.P

clean:
	rm -f $(SRCDIR)/*.o $(SRCDIR)/*.dep $(BIN)
	rm -rf $(BINDIR)

-include $(DEPS)

