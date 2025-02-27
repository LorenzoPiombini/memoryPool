TARGET := memory_pool
SRC := $(wildcard src/*.c)
OBJ := $(patsubst src/%.c, obj/%.o, $(SRC))

default: directories $(TARGET)



directories:
	@if [ ! -d obj ]; then\
		echo "create obj dir";\
		mkdir obj;\
	fi\

clean:
	rm -r obj/*.o
	rm $(TARGET)


$(TARGET) : $(OBJ)
	gcc -o $@ $?  


obj/%.o : src/%.c
	gcc -g3 -Wall -Wextra -c $< -o $@ -Iinclude 
