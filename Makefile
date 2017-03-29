OS = $(shell uname -s)

DEBUG=true

CC 			?= 	cc
CXX			?= 	c++
DESTDIR		?= 	/usr/local
FLAGS 		+= 	-std=c++11
ifdef DEBUG
FLAGS		+=	-ggdb -O0 -fsanitize=address
else
FLAGS		+=	-O2
endif

LIBS 		= 	
CFLAGS		+=	-I. $(FLAGS) -c -MMD -Wall -Wextra
LDFLAGS		+=	$(FLAGS)

SOURCES		= 	test.cxx
OBJECTS		= 	$(SOURCES:.cxx=.o)
DEPENDENCIES=	$(SOURCES:.cxx=.d)
EXECUTABLE	=	test

.PHONY: all clean pages runtests uninstall install installman

all: $(EXECUTABLE)

-include $(DEPENDENCIES)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) -o $@ $(OBJECTS) $(LDFLAGS)

uninstall:
	-rm $(DESTDIR)/include/argsplus.hxx
	-rmdir $(DESTDIR)/include
	-rm $(DESTDIR)/share/man/man3/argsplus_*.3.bz2
	-rmdir -p $(DESTDIR)/share/man/man3

install:
	mkdir -p $(DESTDIR)/include
	cp argsplus.hxx $(DESTDIR)/include

installman: doc/man
	mkdir -p $(DESTDIR)/share/man/man3
	cp doc/man/man3/*.3.bz2 $(DESTDIR)/share/man/man3

clean:
	rm -rv $(EXECUTABLE) $(OBJECTS) $(DEPENDENCIES) doc

pages:
	-rm -r pages/*
	doxygen Doxyfile
	cp -rv doc/html/* pages/

doc/man: 
	doxygen Doxyfile
	bzip2 doc/man/man3/*.3

runtests: test
	./test

%.o: %.cxx
	$(CXX) $< -o $@ $(CFLAGS)
