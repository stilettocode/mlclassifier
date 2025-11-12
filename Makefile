# Compiler
CXX ?= g++

# Compiler flags
CXXFLAGS ?= --std=c++17 -Wall -Werror -pedantic -g -Wno-sign-compare -Wno-comment

classifier.exe: classifier.cpp
	$(CXX) $(CXXFLAGS) classifier.cpp -o $@

# disable built-in rules
.SUFFIXES: