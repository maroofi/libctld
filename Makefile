CC := gcc
CFLAGS := -I./include -Wall
CLIBS := -lidn2
SHELL = /bin/bash


OUTDIR = bin
DEPS = $(wildcard ./src/*.c)
HDEPS = $(wildcard ./include/*.h)
OBJS = cdict.o clist.o cstrlib.o url_parser.o cmdparser.o ctld.o libctld.o 
LIBOBJS = cdict.o clist.o cstrlib.o libctld.o
OBJSTEST = cdict.o clist.o cstrlib.o libctld.o	test.o
BINNAME=ctld
LIBNAME = libctld.so.1

ctld: dummy $(OBJS) $(HDEPS)
	$(CC) $(CFLAGS) $(addprefix bin/, $(OBJS)) -o bin/$(BINNAME) $(CLIBS)
	$(CC) -shared $(CFLAGS) $(addprefix bin/, $(LIBOBJS)) -Wl,-soname,$(LIBNAME) $ -o bin/$(LIBNAME)

test: dummy $(OBJSTEST) $(HDEPS) test.o
	echo "Executing test rules...."
	$(CC) $(CFLAGS) $(addprefix bin/, $(OBJSTEST)) -o bin/test $(CLIBS)
	./bin/test
	./test/test.sh

cdict.o: src/cdict.c include/cdict.h
	$(CC) $(CFLAGS) -fPIC -c $< -o bin/$@

clist.o: src/clist.c include/clist.h
	$(CC) $(CFLAGS) -fPIC -c $< -o bin/$@

libctld.o: src/libctld.c include/libctld.h
	$(CC) $(CFLAGS) -fPIC -c $<  $ -o bin/$@

cstrlib.o: src/cstrlib.c include/cstrlib.h
	$(CC) $(CFLAGS) -fPIC -c $< -o bin/$@

url_parser.o: src/url_parser.c include/url_parser.h
	$(CC) $(CFLAGS) -fPIC -c $< -o bin/$@

cmdparser.o: src/cmdparser.c include/cmdparser.h
	$(CC) $(CFLAGS) -fPIC -c $< -o bin/$@

ctld.o: src/ctld.c include/libctld.h
	$(CC) $(CFLAGS) -c $< -o bin/$@

test.o: test/test.c
	$(CC) $(CFLAGS) -c $< -o bin/$@


dummy:
	mkdir -p bin
	xxd -i psl.dat > ./include/psl_data.h

.PHONY: clean
clean:
	rm -f bin/$(BINNAME) bin/*.o

