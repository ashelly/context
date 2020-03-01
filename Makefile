CFLAGS += -I..

# Specify the target file
OUTPUTFILE  = libcontext.a
lib: $(OUTPUTFILE)


test: main.c context.c dict.c
	$(CC) $^ $(CFLAGS) -g  -o $@
	./test



%.o: %.c
	$(CC) -MMD $(CFLAGS) -g -o $@ -c $<



# Build target lib from source objects
$(OUTPUTFILE): context.o dict.o
	ar r $@ $^
	ranlib $@

context.o: context.h dict.h khash.h
dict.o: dict.h khash.h


CLEANEXTS   = o a d
clean:
	for file in $(CLEANEXTS); do rm -f *.$$file; done
	rm -f test

.PHONY: clean all

all:
	lib

