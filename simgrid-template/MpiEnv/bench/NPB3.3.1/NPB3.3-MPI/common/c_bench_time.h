/*****************************************************************/
/******     C  _  B E N C H _ T I M E                       ******/
/*****************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <mpi.h>
#define NBENCHVALUES 50

typedef struct list_element {
  struct list_element* sibling;
  struct list_element* next;
  char count;
  double values[NBENCHVALUES];
} list_element_t; 


list_element_t* init_list();
void insert_value(list_element_t* root, unsigned int id, double value);
double bench_start();
void bench_stop(list_element_t* root, unsigned int id, unsigned int idstart,double start);
void bench_finalize(list_element_t* root);


#ifdef SAMPLE_ALL
 #define BENCH_INIT 
#define BENCH_START(id,iter,thres) SMPI_SAMPLE_GLOBAL(iter,thres){
#define BENCH_STOP(id) }
 #define BENCH_FINALIZE 
#else
 #define BENCH_INIT double _benchtime;unsigned int _benchcurrent=0xFFFF; list_element_t* _benchroot=init_list();
#define BENCH_START(id,iter,thres) _benchcurrent=id;_benchtime=bench_start();
 #define BENCH_STOP(id) bench_stop(_benchroot,id,_benchcurrent,_benchtime);_benchcurrent=0xFFFF;
 #define BENCH_FINALIZE  bench_finalize(_benchroot);
#endif
