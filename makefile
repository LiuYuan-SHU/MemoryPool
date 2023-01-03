CC=g++
EXECUTABLE=MemoryPool
OPTIONS=-Wall
C98=-D_C98
C11=-D_C11

default:
	@echo "Options:"
	@echo "    11: Compile codes in C-11"
	@echo "    98: Compile codes in C-98"
	@echo "    clean: Clean cached files"

commonOps: test.cpp
	$(CC) $(OPTIONS) -c test.cpp -o test.o

98: C-98/MemoryPool.tcc C-98/MemoryPool.h commonOps
	$(CC) $(OPTIONS) $(C98) -o $(EXECUTABLE) test.o 

11: C-11/MemoryPool.tcc C-11/MemoryPool.h commonOps
	$(CC) $(OPTIONS) $(C11) -o $(EXECUTABLE) test.o 
	
clean:
	rm -f C-98/*.o
	rm -f C-11/*.o
	rm -f $(EXECUTABLE)

