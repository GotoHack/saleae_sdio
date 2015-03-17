MY_INCLUDE= -I ../../include/

OBJECTS= CIA.o \
	SdioCmd52.o \
	SdioCmd53.o \
	SdioCmd5.o \
	SdioCmd.o \
	SDIOParser.o 

all: $(OBJECTS) test

test: test.cpp
	g++ -g $(MY_INCLUDE) $< $(OBJECTS) -o test

.cpp.o:
	g++ -g -c $(MY_INCLUDE) $<


clean:
	rm -rf *.o test
