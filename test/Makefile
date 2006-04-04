LIBS = -lcrypt ../lib/libmenareants.a
CC=g++ -g -O2 -Wall -I../lib/
PRG=${FILE}
CFG=./

OBJS=${FILE}.o

.s.o:
	$(CC) -c $<

%.o: %.cpp
	@echo '$(CC) -c $<'; \
	$(CC) -c $<

${PRG}: ${OBJS}
	${CC} -o $@ ${OBJS} ${LIBS}

clean:
	rm -f *.o *~ *.~* *.save *.bak \#*
