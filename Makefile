# Makefile
#
# Copyright (c) 2018 Jonathan Lemos
#
# This software may be modified and distributed under the terms
# of the MIT license.  See the LICENSE file for details.

NAME=simpletest
VERSION=0.1
SHELL=/bin/sh

LIBRARY=libsimpletest.a
DEMO=demo

CXX:=g++
CXXFLAGS:=-Wall -Wextra -pedantic -std=c++17 -DPROG_NAME=\"$(NAME)\" -DPROG_VERSION=\"$(VERSION)\"
DBGFLAGS:=-g
RELEASEFLAGS:=-O2
LDFLAGS=-lmega

DIRECTORIES=$(shell find . -type d 2>/dev/null | sed -re 's|^.*\.git.*$$||' | awk 'NF')
FILES=$(foreach directory,$(DIRECTORIES),$(shell ls $(directory) | egrep '^.*\.cpp$$' | sed -re 's|^$(DEMO).cpp$$||;s|^(.+)\.cpp$$|\1|' | awk 'NF'))

SOURCEFILES=$(foreach file,$(FILES),$(file).cpp)
OBJECTS=$(foreach file,$(FILES),$(file).o)
DBGOBJECTS=$(foreach file,$(FILES),$(file).dbg.o)

release: $(OBJECTS)
	ar rcs $(LIBRARY) $(OBJECTS)

debug: $(DBGOBJECTS)
	ar rcs $(LIBRARY) $(DBGOBJECTS)

demo: $(DEMO).dbg.o debug
	$(CXX) -o $(DEMO) $(DEMO).dbg.o $(DBGOBJECTS) $(LIBRARY) $(CXXFLAGS) $(DBGFLAGS) $(LDFLAGS)

.PHONY: docs
docs:
	doxygen Doxyfile

%.o: %.cpp
	$(CXX) -c -o $@ $< $(CXXFLAGS) $(RELEASEFLAGS)

%.dbg.o: %.cpp
	$(CXX) -c -o $@ $< $(CXXFLAGS) $(DBGFLAGS)

.PHONY: clean
clean:
	rm -f $(OBJECTS) $(DBGOBJECTS) $(LIBRARY) $(DEMO) $(DEMO).dbg.o
	rm -rf docs
