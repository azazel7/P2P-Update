SRCDIR=src/
OBJDIR=obj/

SRC=$(shell find $(SRCDIR) -name "*.c")
OBJ=$(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRC))

LINKLIB= -lpthread

EXEC=p2p
MAIN=main.c
TESTEUR=testeur.c

CC=gcc

all: main $(SRCDIR)/config.h


main: $(OBJ)
	${CC} $(MAIN) $^ -o $(EXEC) $(LINKLIB)

testeur: $(OBJ)
	${CC}  $(TESTEUR) $^ -o $(EXEC) $(LINKLIB)

rebuild: clean all

clean:
	-rm -f $(OBJ) 

iut: all
	cp $(EXEC) ~/Bureau/
	chmod u+x ~/Bureau/$(EXEC)

run: all
	./$(EXEC)

run_iut: iut
	~/Bureau/$(EXEC)

${OBJDIR}/%.o: ${SRCDIR}/%.c
	@$(CC) -o $@ -c $< ${CFLAGS}


