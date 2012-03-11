CXX      ?= g++
AR       ?= ar
CXXFLAGS ?= -std=c++0x -O2 -g3
CXXFLAGS += -Wall

CXXFLAGS += $(shell pkg-config --cflags OGRE) -DOGRE_PLUGIN_DIR="\"$(shell pkg-config --variable=plugindir OGRE)\""
CXXFLAGS += $(shell pkg-config --cflags OIS)

LDFLAGS  += $(shell pkg-config --libs OGRE)
LDFLAGS  += $(shell pkg-config --libs OIS)
LDFLAGS  += -lnoise


BINDIR  = bin
BINNAME = army
BIN     = $(BINDIR)/$(BINNAME)

SRCDIR = src

SRCFILES = main.cpp App.cpp Terrain.cpp

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

