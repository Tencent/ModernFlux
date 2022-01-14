GXX := /usr/bin/g++
LIB_DIR := ../
IFLAGS := -I$(LIB_DIR) 
LFLAGS := $(LIB_DIR)/libJsonCpp.a 
SOURCES := $(wildcard *.cpp)
OBJS   := $(patsubst %.cpp,%.o,$(SOURCES))
TARGET := test.cgi 

ifeq ($(findstring .fcg,$(TARGET)), .fcg)
    OFLAGS := -fPIC -shared -z defs
else
    OFLAGS := 
endif

all:$(TARGET) 

$(TARGET): $(OBJS) $(LFLAGS)
	$(GXX) $(OFLAGS) -o $(TARGET) $(OBJS) $(LFLAGS)

%.o:%.cpp
	$(GXX) $(IFLAGS) $< -c -o $*.o
	
.PHONY: clean

clean:
	rm *.so *.o -rf *.cgi *.fcg
