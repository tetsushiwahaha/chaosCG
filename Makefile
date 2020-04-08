# choose a compiler
#
#CC = gcc
CC = clang

TARGET = cpS
CPPUTCOLOR = cpPutColor
CPHIST = cpHist
#CFLAGS = -msupersparc -I. #-DDEBUG# -g -DCOLBAND
INCLUDES = -I. -I/opt/local/include
CFLAGS = -g $(INCLUDES)
LIBS = -L/opt/local/lib

.c.o:
	$(CC) $(CFLAGS) $(DEFS) -c $<

all: $(TARGET) $(CPPUTCOLOR) $(CPHIST)

$(TARGET): cpCalc.o cpZrw.o $(TARGET).o
	$(CC) $(CFLAGS) $(LIBS) -o $@ cpCalc.o cpZrw.o $@.o -lz -lm

$(CPPUTCOLOR): cpZrw.o cpPutColor.o
	$(CC) $(CFLAGS) $(LIBS) -o $@ $@.o cpZrw.o  -lz -lpng -lm

$(CPHIST): cpHist.o
	$(CC)  $(CFLAGS) $(LIBS) -o $@ $@.o cpZrw.o  -lz -lm

clean:
	-rm -f *.o *bak $(TARGET) $(CPPUTCOLOR) $(CPHIST)
