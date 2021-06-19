/*****************************************************************
 Implementation of the robust taboo search of: E. Taillard
 "Robust taboo search for the quadratic assignment problem", 
 Parallel Computing 17, 1991, 443-455.

 Data file format: 
  n, optimum solution value
 (nxn) flow matrix,
 (nxn) distance matrix

 Copyright : E. Taillard, 1990-2004
 Standard C version with slight improvement regarding to
 1991 version: non uniform tabu duration
E. Taillard, 14.03.2006

 Compatibility: Unix and windows gcc, g++, bcc32.
 This code can be freely used for non-commercial purpose.
 Any use of this implementation or a modification of the code
 must acknowledge the work of E. Taillard

****************************************************************/
#include <stdio.h>
#include <stdlib.h>

const int infinite = 999999999;
const int FALSE = 0;
const int TRUE = 1;

typedef int*  type_vector;
typedef int** type_matrix;

int opt;
double  somme_sol = 0.0;

/*************** L'Ecuyer random number generator ***************/
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
  if (h == 0) return(1.0); else return(h*invm);
 }

/*********** return an integer between low and high *************/
int unif(int low, int high)
 {return low + (int)((double)(high - low + 1) * rando()) ;}

void transpose(int *a, int *b) {int temp = *a; *a = *b; *b = temp;}

int min(int a, int b) {if (a < b) return(a); else return(b);}

double cube(double x) {return x*x*x;}


/*--------------------------------------------------------------*/
/*       compute the cost difference if elements i and j        */
/*         are transposed in permutation (solution) p           */
/*--------------------------------------------------------------*/
int compute_delta(int n, type_matrix a, type_matrix b,
                   type_vector p, int i, int j)
 {int d; int k;
  d = (a[i][i]-a[j][j])*(b[p[j]][p[j]]-b[p[i]][p[i]]) +
      (a[i][j]-a[j][i])*(b[p[j]][p[i]]-b[p[i]][p[j]]);
  for (k = 0; k < n; k = k + 1) if (k!=i && k!=j)
    d = d + (a[k][i]-a[k][j])*(b[p[k]][p[j]]-b[p[k]][p[i]]) +
            (a[i][k]-a[j][k])*(b[p[j]][p[k]]-b[p[i]][p[k]]);
  return(d);
 }

/*--------------------------------------------------------------*/
/*      Idem, but the value of delta[i][j] is supposed to       */
/*    be known before the transposition of elements r and s     */
/*--------------------------------------------------------------*/
int compute_delta_part(type_matrix a, type_matrix b,
                        type_vector p, type_matrix delta, 
                        int i, int j, int r, int s)
  {return(delta[i][j]+(a[r][i]-a[r][j]+a[s][j]-a[s][i])*
     (b[p[s]][p[i]]-b[p[s]][p[j]]+b[p[r]][p[j]]-b[p[r]][p[i]])+
     (a[i][r]-a[j][r]+a[j][s]-a[i][s])*
     (b[p[i]][p[s]]-b[p[j]][p[s]]+b[p[j]][p[r]]-b[p[i]][p[r]]) );
  }

void tabu_search(int n,                  /* problem size */
                 type_matrix a,          /* flows matrix */
                 type_matrix b,          /* distance matrix */
                 type_vector best_sol,   /* best solution found */
                 int *best_cost,         /* cost of best solution */
                 int tabu_duration,      /* parameter 1 (< n^2/2) */
                 int aspiration,         /* parameter 2 (> n^2/2)*/
                 int nr_iterations,      /* number of iterations */ 
                 int *best_iter)
           
 
 {type_vector p;                        /* current solution */
  type_matrix delta;                    /* store move costs */
  type_matrix tabu_list;                /* tabu status */
  int current_iteration;                /* current iteration */
  int current_cost;                     /* current sol. value */
  int i, j, k, i_retained, j_retained;  /* indices */
  int min_delta;                        /* retained move cost */
  int autorized;                        /* move not tabu? */
  int aspired;                          /* move forced? */
  int already_aspired;                  /* in case many moves forced */

  /***************** dynamic memory allocation *******************/
  p = (int*)calloc(n, sizeof(int));
  delta = (int**)calloc(n,sizeof(int*));
  for (i = 0; i < n; i = i+1) delta[i] = (int*)calloc(n, sizeof(int));
  tabu_list = (int**)calloc(n,sizeof(int*));
  for (i = 0; i < n; i = i+1) tabu_list[i] = (int*)calloc(n, sizeof(int));


  /************** current solution initialization ****************/
  for (i = 0; i < n; i = i + 1) p[i] = best_sol[i];

  /********** initialization of current solution value ***********/
  /**************** and matrix of cost of moves  *****************/
  current_cost = 0;
  for (i = 0; i < n; i = i + 1) for (j = 0; j < n; j = j + 1)
   {current_cost = current_cost + a[i][j] * b[p[i]][p[j]];
    if (i < j) {delta[i][j] = compute_delta(n, a, b, p, i, j);};
   };
  *best_cost = current_cost;

  /****************** tabu list initialization *******************/
  for (i = 0; i < n; i = i + 1) for (j = 0; j < n; j = j+1)
    tabu_list[i][j] = -(n*i + j);

  /******************** main tabu search loop ********************/
  for (current_iteration = 1; current_iteration <= nr_iterations && *best_cost > opt; 
       current_iteration = current_iteration + 1)
   {/** find best move (i_retained, j_retained) **/
    i_retained = infinite;       /* in case all moves are tabu */
    j_retained = infinite;
    min_delta = infinite;
    already_aspired = FALSE;
    
    for (i = 0; i < n-1; i = i + 1) 
      for (j = i+1; j < n; j = j+1)
       {autorized = (tabu_list[i][p[j]] < current_iteration) || 
                    (tabu_list[j][p[i]] < current_iteration);

        aspired =
         (tabu_list[i][p[j]] < current_iteration-aspiration)||
         (tabu_list[j][p[i]] < current_iteration-aspiration)||
         (current_cost + delta[i][j] < *best_cost);                

        if ((aspired && !already_aspired) || /* first move aspired*/
           (aspired && already_aspired &&    /* many move aspired*/
            (delta[i][j] < min_delta)   ) || /* => take best one*/
           (!aspired && !already_aspired &&  /* no move aspired yet*/
            (delta[i][j] < min_delta) && autorized))
          {i_retained = i; j_retained = j;
           min_delta = delta[i][j];
           if (aspired) {already_aspired = TRUE;};
          };
       };

    if (i_retained == infinite) printf("All moves are tabu! \n"); 
    else 
     {/** transpose elements in pos. i_retained and j_retained **/
      transpose(&p[i_retained], &p[j_retained]);
      /* update solution value*/
      current_cost = current_cost + delta[i_retained][j_retained];
      /* forbid reverse move for a random number of iterations*/
      tabu_list[i_retained][p[j_retained]] = 
        current_iteration + (int)(cube(rando())*tabu_duration);
      tabu_list[j_retained][p[i_retained]] = 
         current_iteration + (int)(cube(rando())*tabu_duration);

      /* best solution improved ?*/
      if (current_cost < *best_cost)
       {*best_cost = current_cost;
        for (k = 0; k < n; k = k+1) best_sol[k] = p[k];
//         printf("Solution of value: %d found at iter. %d\n", current_cost, current_iteration);
        *best_iter = current_iteration;
       };

      /* update matrix of the move costs*/
      for (i = 0; i < n-1; i = i+1) for (j = i+1; j < n; j = j+1)
        if (i != i_retained && i != j_retained && 
            j != i_retained && j != j_retained)
         {delta[i][j] = 
            compute_delta_part(a, b, p, delta, 
                               i, j, i_retained, j_retained);}
        else
         {delta[i][j] = compute_delta(n, a, b, p, i, j);};
     };
      
   }; 
  /* free memory*/
  free(p);
  for (i=0; i < n; i = i+1) free(delta[i]); free(delta);
  for (i=0; i < n; i = i+1) free(tabu_list[i]); 
  free(tabu_list);
} /* tabu*/

void generate_random_solution(int n, type_vector  p)
 {int i;
  for (i = 0; i < n;   i++) p[i] = i;
  for (i = 0; i < n-1; i++) transpose(&p[i], &p[unif(i, n-1)]);
 }

int main(int argc, char *argv[])
 {int n;                    /* problem size */
  type_matrix a, b;         /* flows and distances matrices*/
  type_vector solution;     /* solution (permutation) */
  int cost, iter;           /* solution cost and iteration */
  int nr_iterations,        /* number of tabu search iterations */
      nr_resolutions, no_res; /* number of trials */

//   FILE* data_file;       
//   char file_name[30];
  int i, j;
  char bidon[1000];

  if (argc < 3) {
    printf("Usage: %s #iteration #trial < qap_file\n", argv[0]);
    return EXIT_FAILURE;
  }
  nr_iterations = atoi(argv[1]);
  nr_resolutions = atoi(argv[2]);

  /************** read file name and problem size ***************/
//   printf("Data file name : \n");
//   scanf("%s",file_name); printf(file_name); printf("\n");
//   data_file = fopen(file_name,"r");
//   fscanf(data_file,"%d%d", &n, &opt);
  scanf("%d%d", &n, &opt);
//   fscanf(data_file,"%[^\n]", bidon);
  scanf("%[^\n]", bidon);

//   printf("Number of tabu search iterations:\n");
//   scanf("%d",&nr_iterations);
//   printf("Number of trials:\n");
//   scanf("%d",&nr_resolutions);

  /****************** dynamic memory allocation ******************/
  solution = (int*)calloc(n, sizeof(int));

  a = (int**)calloc(n, sizeof(int*));
  for (i = 0; i < n; i = i+1) a[i] = (int*)calloc(n, sizeof(int));
  b = (int**)calloc(n,sizeof(int*));
  for (i = 0; i < n; i = i+1) b[i] = (int*)calloc(n, sizeof(int));

  /************** read flows and distances matrices **************/
  for (i = 0; i < n; i++) for (j = 0; j < n; j = j+1)
//     fscanf(data_file,"%d", &a[i][j]);
    scanf("%d", &a[i][j]);


  for (i = 0; i < n; i = i+1) for (j = 0; j < n; j = j+1)
//     fscanf(data_file,"%d", &b[i][j]);
    scanf("%d", &b[i][j]);
    
//   fclose(data_file);
  
  for(no_res = 1; no_res <= nr_resolutions; no_res++)
   {generate_random_solution(n, solution);

    tabu_search(n, a, b,                     /* problem data */
               solution, &cost,              /* tabu search results */
               8*n, n*n*5,                   /* parameters */
               nr_iterations,                /* number of iterations */
               &iter);

//     printf("%d Solution found by tabu search:\n", cost);
//     for (i = 0; i < n; i = i+1) printf("%d ", solution[i]); 
    printf("@%d %d [", iter, cost);
    for (i = 0; i < n; i++) {
      printf("%d", solution[i]);
      if (i < n - 1) printf(",");
    }
    printf("]\n");
    somme_sol += cost;
   }
   
//   printf("Average cost: %f, average dev: %f\n", 
//           somme_sol/nr_resolutions, 100*(somme_sol/nr_resolutions - opt)/opt);

  free(solution);
  for (i = 0; i < n; i = i+1) free(b[i]);
  free(b);
  for (i = 0; i < n; i = i+1) free(a[i]);
  free(a);
  fflush(stdin);
//   printf("Press return to end programme\n");
//   getchar();
  return EXIT_SUCCESS;
 }
 

