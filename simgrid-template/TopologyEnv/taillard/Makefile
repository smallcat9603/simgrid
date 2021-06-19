CC     = g++
CFLAGS = -O3 -march=native

all : fant_qap tabou_qap tabou_qap2 sa_qap sa_qbap

fant_qap : fant_qap.c
	$(CC) $(CFLAGS) fant_qap.c -o $@

tabou_qap : tabou_qap.cpp
	$(CC) $(CFLAGS) tabou_qap.cpp -o $@

tabou_qap2 : tabou_qap2.c
	$(CC) $(CFLAGS) tabou_qap2.c -o $@

sa_qap : sa_qap.cpp
	$(CC) $(CFLAGS) sa_qap.cpp -o $@

sa_qbap : sa_qbap.cpp
	$(CC) $(CFLAGS) sa_qbap.cpp -o $@

clean :
	rm fant_qap tabou_qap tabou_qap2 sa_qap sa_qbap
