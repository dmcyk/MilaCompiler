CPP_FILES := $(wildcard src/*.cpp)
OBJ_FILES = $(patsubst src/%.cpp,obj/%.o,$(CPP_FILES))
OBJREL_FILES = $(patsubst src/%.cpp,obj_rel/%.o,$(CPP_FILES))
LD_FLAGS := -L/usr/local/opt/llvm/lib 
CC_FLAGS := -I/usr/local/opt/llvm/include
CPP_FLAGS := -std=c++1z

.PHONY: prepare

release: $(OBJREL_FILES) 
	clang++ $(CPP_FLAGS) $(LD_FLAGS) `llvm-config --cxxflags --ldflags --system-libs --libs all` -O3 -o Mila $^

debug: $(OBJ_FILES) 
	clang++ $(CPP_FLAGS) $(LD_FLAGS) `llvm-config --cxxflags --ldflags --system-libs --libs all` -g -o Mila $^

obj/%.o: src/%.cpp | prepare
	clang++ $(CPP_FLAGS) $(CC_FLAGS) -g -c -o $@ $<


obj_rel/%.o: src/%.cpp | prepare
	clang++ $(CPP_FLAGS) $(CC_FLAGS) -c -O3 -o $@ $<

prepare:
	mkdir -p obj
	mkdir -p obj_rel

clean:
	-rm -rf ./obj >/dev/null 2>&1
	-rm -rf ./obj_rel >/dev/null 2>&1
	-rm -f Mila
