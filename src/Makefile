SRC = $(shell find ./ -name '*.c')
OBJ = ../osd

.PHONY: clean osd
osd: $(OBJ)

$(OBJ): $(SRC:%.c=%.o)
	$(CC) $^ -rdynamic $(OPT) -o $@

%.o: %.c Makefile
	$(CC) -c $< $(OPT) -o $@

clean:
	rm -rf $(SRC:%.c=%.o) $(OBJ)