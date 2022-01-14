GXX := g++  
INC := -I.
LDFLAGS := -Wall -fPIC -shared -D_FILE_OFFSET_BITS=64
SOURCES := $(wildcard *.cpp)
OBJS   := $(patsubst %.cpp,%.o,$(SOURCES))
TARGET_SHARED := libJsonCpp.so
TARGET_STATIC := libJsonCpp.a

all: $(TARGET_STATIC) $(TARGET_SHARED)
	
$(TARGET_SHARED): $(OBJS)		
	$(GXX) $(LDFLAGS) $^ -o $(TARGET_SHARED)

$(TARGET_STATIC): $(OBJS)
	ar -rcs $@ $^
	
%.o:%.cpp
	$(GXX) $(LDFLAGS) -c $< $(INC) -o $*.o
	
.PHONY: clean

clean:
	rm *.so *.o *.a -rf
