TARGET_SRV = bin/dbserver
TARGET_CLI = bin/dbcli

SRC_SRV = $(wildcard src/srv/*.c)
OBJ_SRV = $(patsubst src/srv/%.c,obj/srv/%.o,$(SRC_SRV))

SRC_CLI = $(wildcard src/cli/*.c)
OBJ_CLI = $(patsubst src/cli/%.c,obj/cli/%.o,$(SRC_CLI))

run: clean default

default: $(TARGET_SRV) $(TARGET_CLI)

clean:
	rm -f obj/srv/*.o
	rm -f obj/cli/*.o
	rm -f bin/*
	rm -f *.db

$(TARGET_SRV): $(OBJ_SRV)
	gcc -g -o $@ $?

$(OBJ_SRV): obj/srv/%.o: src/srv/%.c
	gcc -g -c $< -o $@ -Iinclude/common -Iinclude/srv

$(TARGET_CLI): $(OBJ_CLI)
	gcc -g -o $@ $?

$(OBJ_CLI): obj/cli/%.o: src/cli/%.c
	gcc -g -c $< -o $@ -Iinclude/common -Iinclude/cli