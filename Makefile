CXX = clang++
CC = gcc

CXXFLAGS := -std=c++1z -stdlib=libc++ -I Spliwaca/Spliwaca/src/ -I Spliwaca/vendor/spdlog/include -I Spliwaca/vendor/timing --define-macro SPLW_DEBUG
CFLAGS := -std=c11
LDFLAGS := 

OBJ_PREFIX = Spliwaca/Spliwaca/src/
OBJS := $(OBJ_PREFIX)Main.o $(OBJ_PREFIX)Transpiler.o $(OBJ_PREFIX)Log.o $(OBJ_PREFIX)Frontend/Lexer/Lexer.o $(OBJ_PREFIX)Frontend/Parser/IdentNode.o $(OBJ_PREFIX)Frontend/Parser/Parser.o $(OBJ_PREFIX)Backend/Generation/Python/Generator.o 

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: test build clean

all: build

build: $(OBJS)
	mkdir -p build
	$(CXX) $(LDFLAGS) $(OBJS) -o transpiler

debug: CXXFLAGS += -D SPLW_DEBUG
debug: build

release: CFLAGS +=
release: CXXFLAGS += -O3
release: build

dist: CXXFLAGS += -D SPLW_NO_INSTRUMENT
dist: release

clean:
	rm -rf src/*.o
	rm -rf src/*.d
	rm -rf srcpp/*.o
	rm -rf srcpp/*.d
	rm -rf build