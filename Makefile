TARGET := memory_pool
SRC := $(wildcard src/*.c)
OBJ := $(patsubst src/%.c, obj/%.o, $(SRC))
OBJ_prod := $(patsubst src/%.c, obj/%_prod.o, $(SRC))

OBJlib := obj/mem_pool.o


LIBNAME := mpool
LIBDIR := /usr/local/lib
INCLUDEDIR := /usr/local/include
SHAREDLIB := lib$(LIBNAME).so


default: directories $(TARGET)



prod: directories $(TARGET)_prod

directories:
	@if [ ! -d obj ]; then\
		echo "create obj dir";\
		mkdir obj;\
	fi\

library:
	sudo gcc -Wall -fPIC -shared -o $(SHAREDLIB) $(OBJlib)

clean:
	rm -r obj/*.o
	rm *core* 
	rm $(TARGET)

check-linker-path:
	@if [ ! -f /etc/ld.so.conf.d/customtech.conf ]; then \
		echo "setting linker configuration..." ;\
		echo "/usr/local/lib" | sudo tee /etc/ld.so.conf.d/customtech.conf >/dev/null ;\
		sudo ldconfig;\
	fi

$(TARGET) : $(OBJ)
	gcc -o $@ $?  -fPIC -fsanitize=address


obj/%.o : src/%.c
	gcc -g3 -Wall -Wextra -c $< -o $@ -fPIC -Iinclude -fsanitize=address


$(TARGET)_prod : $(OBJ_prod)
	gcc -o $@ $?  

obj/%_prod.o : src/%.c
	gcc -c $< -o $@ -Iinclude



install: $(TARGET) library check-linker-path
	install -d $(INCLUDEDIR) 
	install -m 644 include/mem_pool.h $(INCLUDEDIR)/
	install -m 755 $(SHAREDLIB) $(LIBDIR)
	ldconfig

