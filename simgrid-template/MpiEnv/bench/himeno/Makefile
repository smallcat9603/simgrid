
include config/make.def

OBJS = himenoBMTxps.o

CFLAGS=-g

RM = /bin/rm
PROG = himeno
PROGDIR=bin

${PROGDIR}/${PROG}: all

${PROG} : all

all: ${OBJS} param.h
	${CLINK} ${CLINKFLAGS} -o ${PROGDIR}/${PROG} ${OBJS} ${CMPI_LIB}

.c.o : param.h
	 ${MPICC}  ${CFLAGS} ${CMPI_INC} -c ${CFLAGS} $<

clean :
	${RM} -f ${PROGDIR}/${PROG} ${OBJS} core log*.txt param.h
	
