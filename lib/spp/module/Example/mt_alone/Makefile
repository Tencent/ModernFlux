MT_INCL=../sync_incl
MT_LIB=./lib

CXX = g++
CFLAGS += -g -Wall -Wno-write-strings -Werror
CFLAGS += -D_FILE_OFFSET_BITS=64
INC += -I$(MT_INCL)
LIB += -L$(MT_LIB) -lmt -ldl -Wl,-rpath,$(MT_LIB)

ifeq ($(ARCH),32)
	CFLAGS += -m32 -march=pentium4
endif

TARGET = mt_alone
OBJ = mt_alone.o

$(TARGET): $(OBJ)
	$(CXX) $(CFLAGS) $(LIB) -o $@ $^

clean:
	@echo -e Clean ...
	@-rm *.o
	@-rm $(TARGET)

%.o: %.cpp
	@echo -e Compiling $< ...
	$(CXX) $(CFLAGS) -c -o $@ $< $(INC)


