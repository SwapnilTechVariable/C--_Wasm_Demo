CC := g++
EMCC := em++
FLAGS := -std=c++17
BASE64_DIR := cpp-base64

base64:
	$(CC) $(FLAGS) -c $(BASE64_DIR)/base64.cpp -o $(BASE64_DIR)/base64-11.o

base64_wasm:
	$(EMCC) $(FLAGS) -c $(BASE64_DIR)/base64.cpp -o $(BASE64_DIR)/base64-11.o

bs_files := $(wildcard cpp-base64/*.o)

project: base64
	$(CC) $(FLAGS) -o main main.cpp $(bs_files) 

project_wasm: base64_wasm
	$(EMCC) $(FLAGS) main.cpp -lembind -o main.html $(bs_files) -s ALLOW_MEMORY_GROWTH

project_wasm_debug: base64_wasm
	$(EMCC) $(FLAGS) main.cpp -g -gdwarf-5 -gsplit-dwarf -gpubnames -O0 -lembind -o main.html $(bs_files) -s ALLOW_MEMORY_GROWTH=1 -sTOTAL_MEMORY=50MB -sWASM_BIGINT -sERROR_ON_WASM_CHANGES_AFTER_LINK -s NO_EXIT_RUNTIME=1

