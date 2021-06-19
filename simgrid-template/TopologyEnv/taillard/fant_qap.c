#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

/* Programme for approximately solving the quadratic assignment problem.
   Language : c; compiler gcc should work.
   

   Method: FANT, Described in E. D. Taillard, 
   "FANT: Fast ant system",
   Technical report IDSIA-46-98, IDSIA, Lugano, 1998.

   Implementation : E. Taillard, 14. 10. 2010
   Copyright :      E. Taillard, 14. 10. 2010
   Available on :   http://mistic.heig-vd.ch/taillard/codes.dir/fant_qap.c

   Data :
     size of the problem, parameter R, number of FANT iterations
     distance matrix   
     flow matrix

   Exemple of valid data 
  (for problem tai10b, to be included in a file, e. g. tai10b.dat) :

  10

 0   19  78  60 79  45  65  37 103  34
 19   0  65  45 76  63  79  22 109  19
 78  65   0  21 44 113 104  72  97  73
 60  45  21   0 53  99  97  50 102  51
 79  76  44  53  0  98  74  93  54  92
 45  63 113  99 98   0  42  81  97  78
 65  79 104  97 74  42   0 100  57  98
 37  22  72  50 93  81 100   0 130   3
103 109  97 102 54  97  57 130   0 128
 34  19  73  51 92  78  98   3 128   0

   0    1    0    2  0    0  15    0   0  172
 171    0    0   14  0   61  30    0 886   45
  43    0    0 2106  1    0   0    0   0    0
 361    0    0    0  0    0   0    0   0    0
   2  123    1    0  0   49  18    0 335 2417
1096    0    0    0  0    0 952    5   0    0
 207 3703   27    0  4    0   0    0 202    0
  16   58    0    0  0    0 546    0  42 1213
   0    0    0   53  0  546   7 2649   0   86
   0    0 6707    1 12 7124   1    0   1    0

Running with the data given above the programme gives :

Data file name :
toto.txt
 parameter R and number of iterations :
5 100
New best solution value, cost : 1217793  Found at iteration : 1
4 5 0 3 7 6 1 2 8 9
New best solution value, cost : 1187126  Found at iteration : 6
4 2 8 7 6 3 0 5 1 9
New best solution value, cost : 1183760  Found at iteration : 25
4 5 0 3 6 7 8 2 1 9
*/

const int infinite = 2099999999;

typedef int*  type_vector;
typedef int **type_matrix;




/************* random number generator, of Lecuyer ****************/
double rando()
 {
  static int x10 = 12345, x11 = 67890, x12 = 13579, /* initial value*/
             x20 = 24680, x21 = 98765, x22 = 43210; /* of seeds*/
  const int m = 2147483647; const int m2 = 2145483479; 
  const int a12= 63308; const int q12=33921; const int r12=12979; 
  const int a13=-183326; const int q13=11714; const int r13=2883; 
  const int a21= 86098; const int q21=24919; const int r21= 7417; 
  const int a23=-539608; const int q23= 3976; const int r23=2071;
  const double invm = 4.656612873077393e-10;
  int h, p12, p13, p21, p23;
  h = x10/q13; p13 = -a13*(x10-h*q13)-h*r13;
  h = x11/q12; p12 = a12*(x11-h*q12)-h*r12;
  if (p13 < 0) p13 = p13 + m; if (p12 < 0) p12 = p12 + m;
  x10 = x11; x11 = x12; x12 = p12-p13; if (x12 < 0) x12 = x12 + m;
  h = x20/q23; p23 = -a23*(x20-h*q23)-h*r23;
  h = x22/q21; p21 = a21*(x22-h*q21)-h*r21;
  if (p23 < 0) p23 = p23 + m2; if (p21 < 0) p21 = p21 + m2;
  x20 = x21; x21 = x22; x22 = p21-p23; if(x22 < 0) x22 = x22 + m2;
  if (x12 < x22) h = x12 - x22 + m; else h = x12 - x22;
  if (h == 0) {printf("!!!!! h == 0\n"); return(1.0); }
  else return(h*invm);
 }


int unif(int low, int high)
 {return low + (int)((double)(high - low + 1) * rando()) ;}


void swap(int *a, int *b) {int temp = *a; *a = *b; *b = temp;}
/**********************************************************/


void print(int n, type_vector p) 
// print vector p
{ int i; 
  for (i = 0; i < n; i++) 
    printf("%d ",p[i]);
  printf("\n");
}
   
// compute the value of move (r, s) on solution p
int compute_delta(int n, type_matrix a, type_matrix  b,
                  type_vector  p, int r, int s)
{ int d; int k;
  d = (a[r][r]-a[s][s])*(b[p[s]][p[s]]-b[p[r]][p[r]]) +
      (a[r][s]-a[s][r])*(b[p[s]][p[r]]-b[p[r]][p[s]]);
  for (k = 0; k < n; k++) if (k!=r && k!=s)
    d = d + (a[k][r]-a[k][s])*(b[p[k]][p[s]]-b[p[k]][p[r]]) +
            (a[r][k]-a[s][k])*(b[p[s]][p[k]]-b[p[r]][p[k]]);
  return d;
 }

// compute the cost of solution p
int compute_cost(int n, type_matrix a, type_matrix b, type_vector p)
{ int c = 0; int i, j;
  for (i = 0; i < n; i++) 
    for (j = 0; j < n; j++)
      c += a[i][j] * b[p[i]][p[j]];
  return c;
 }

// generate a random permutation p
void generate_random_permutation(int n, type_vector   p)
 {int i;
  for (i = 0; i < n; i++) p[i] = i;
  for (i = 0; i < n-1; i++) swap(&p[i], &p[unif(i, n-1)]);
 }

// local search
// Scan the neighbourhood at most twice
// Perform improvements as soon as they are found
void local_search(int n, type_matrix  a, type_matrix  b,
                  type_vector  p, int *cost)
 {int r, s, i, j, scan_nr, nr_moves;
  int delta;
  // set of moves, numbered from 0 to index
  type_vector move;
  move = (int*) malloc((n*(n-1)/2)*sizeof(int));
  nr_moves = 0;
  for (i = 0; i < n-1; i++)
    for (j=i+1; j < n; j++) move[nr_moves++] = n*i+j;
  int improved = true;
  for (scan_nr = 0;  scan_nr < 2 && improved;  scan_nr++)
  { improved = false;
    for (i = 0; i < nr_moves-1; i++)
      swap(&move[i], &move[unif(i+1, nr_moves-1)]);
    for (i = 0; i < nr_moves; i++)
    {
      r = move[i]/n;
      s = move[i]%n;
      delta = compute_delta(n, a, b, p, r, s);
      if (delta < 0)
      { *cost += delta; swap(&p[r], &p[s]); 
        improved = true;
      }
    }
  }
  free(move);
 }


/************************ memory management *************************/

// (re-) initialization of the memory
void init_trace(int n, int increment, type_matrix trace)
 {int i, j;
  for (i = 0; i < n; i++) 
    for (j = 0; j < n; j++)
      trace[i][j] = increment;
 }

// memory update
void update_trace(int n, type_vector p, type_vector best_p,
                   int *increment, int R, type_matrix trace)
{ int i = 0;
  while (i < n && p[i] == best_p[i]) i++;
  if (i == n)
  {
    (*increment)++;
    init_trace(n, *increment, trace);
  }
  else
    for (i = 0; i < n; i++)
    {
      trace[i][p[i]] += *increment;
      trace[i][best_p[i]] += R;
    }
 }

// generate a solution with probability of setting p[i] == j
// proportionnal to trace[i][j]
void generate_solution_trace(int n, type_vector p, type_matrix trace)
{
  int i, j, k, target, sum;
  type_vector nexti, nextj, sum_trace;
  nexti = (int*) malloc(n*sizeof(int));
  nextj = (int*) malloc(n*sizeof(int));
  sum_trace = (int*) calloc(n, sizeof(int));

  generate_random_permutation(n, nexti);
  generate_random_permutation(n, nextj);
  for (i = 0; i < n; i++)
    for (j = 0; j < n; j++)
      sum_trace[i] += trace[i][j];

  for (i = 0; i < n; i++)
  {
    target = unif(0, sum_trace[nexti[i]]-1);
    j = i;
    sum = trace[nexti[i]][nextj[j]];
    while (sum < target)
    {
      j++;
      sum += trace[nexti[i]][nextj[j]];
    }
    p[nexti[i]] = nextj[j];
    for (k = i; k <n; k++)
      sum_trace[nexti[k]] -= trace[nexti[k]][nextj[j]];
    swap(&nextj[j], &nextj[i]);
  }
  free(nexti);
  free(nextj);
  free(sum_trace);
}
  



/********************************************************************/



int main(int argc, char *argv[])
 {int  n;                                 // size of the problem
  int cost, best_cost, best_iter;         // cost of current solution, best cost
  type_matrix a, b;                       // flow and distance matrices
  type_vector p, best_p;                  // current solution and best solution
  int nb_iterations;                      // number of FANT iterations
  type_matrix trace;                      // ant memory
  int increment, R, rmax;                 // parameter for managing the traces
  int k, no_iteration;                    // iteration counters

  char trash[1000];
  int i, j;

  if (argc < 3) {
    printf("Usage: %s #iteration #trial < qap_file\n", argv[0]);
    return EXIT_FAILURE;
  }
  nb_iterations = atoi(argv[1]);
  rmax = atoi(argv[2]);

  scanf("%d", &n);           // read problem size
  scanf("%[^\n]\n", trash);  // ignore rest of the line

  // Memory allocation
  a = (int**)malloc(n * sizeof(int*));
  for (i = 0; i < n; i++) 
    a[i] = (int*) malloc(n*sizeof(int));

  b = (int**)malloc(n * sizeof(int*));
  for (i = 0; i < n; i++) 
    b[i] = (int*) malloc(n*sizeof(int));

  // Reading flow and distance matrix
  for (i = 0; i < n; i++) 
    for (j = 0; j < n; j++)
      scanf("%d", &a[i][j]);
  for (i = 0; i < n; i++) 
    for (j = 0; j < n; j++)
      scanf("%d", &b[i][j]);

  // Outer loop increments R upto rmax.  Added by Ikki
  for (R = 1; R <= rmax; R++) {
	
    best_p = (int*) calloc(n, sizeof(int)); // must be initially different from p
    p = (int*) malloc(n * sizeof(int));
  
    trace = (int**)malloc(n * sizeof(int*));
    for (i = 0; i < n; i++) 
      trace[i] = (int*) malloc(n*sizeof(int));
    increment = 1;
    init_trace(n, increment, trace);
    best_cost = infinite;
  
    // FANT iterations
    for (no_iteration = 1; no_iteration <= nb_iterations;
         no_iteration = no_iteration + 1)
                                                     
    { // Build a new solution
      generate_solution_trace(n, p, trace);
      cost = compute_cost(n, a, b, p);
      // Improve solution with a local search
      local_search(n, a, b, p, &cost);
  
      // Best solution improved ?
      if (cost < best_cost)
      { best_cost = cost; 
        for (k = 0; k < n; k = k + 1) best_p[k] = p[k];
        best_iter = no_iteration;
        increment = 1;
        init_trace(n, increment, trace);
       }
      else                                              
        // Memory update
        update_trace(n, p, best_p, &increment, R, trace);
     };

    printf("@%d %d [", best_iter, best_cost);
    for (i = 0; i < n; i++) {
      printf("%d", best_p[i]);
      if (i < n - 1) printf(",");
    }
    printf("]\n");
    
    free(p);
    free(best_p);
    for (i = 0; i < n; i++) 
      free(trace[i]);
    free(trace);
  }
  
  // ending the programme
  for (i = 0; i < n; i++) 
    free(a[i]);
  free(a);
  for (i = 0; i < n; i++) 
    free(b[i]);
  free(b);

  fflush(stdin);
  getchar();
  return EXIT_SUCCESS;
 }
