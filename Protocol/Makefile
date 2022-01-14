include ../PreDefine

INC = -I./ -I$(PROTOINC)
PROTOCFLAG = --proto_path=./ --cpp_out=.  

OBJDIR  =./obj
CPPSRCS =$(wildcard ./*.cc)
OBJS    =$(patsubst ./%.cc,$(OBJDIR)/%.o, $(CPPSRCS))

TARGET = lib/libproto.a
    
PROTO_SRCS = $(wildcard ./*.proto)

%.pb.h %.pb.cc: %.proto
	#rm -f $(*F).pb.h $(*F).pb.cc
	$(PROTOC) $(PROTOCFLAG) $<

all:$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
	$(PROTOC) $(PROTOCFLAG) $(PROTO_SRCS)
clear:
	rm -f $(OBJS) $(TARGET)
	rm -rf *~
	rm -rf *.pb.*
	rm -rf $(OBJDIR)/*.d	

$(TARGET):$(OBJS)
	rm -rf $@
	ar q $@ $(OBJS) $(PROTOLIB)
	
obj/%.o : %.cc %.h
	$(CXX) $(CFLAGS) $(INC) -c $< -o $@
	

