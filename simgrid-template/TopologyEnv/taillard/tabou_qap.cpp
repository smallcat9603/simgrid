/*****************************************************************/
// Implementation of the robust taboo search of: E. Taillard
// "Robust taboo search for the quadratic assignment problem", 
// Parallel Computing 17, 1991, 443-455.
//
// Data file format: 
//  n,
// (nxn) flow matrix,
// (nxn) distance matrix
//
// Copyright : E. Taillard, 1990-2004
// This code can be freely used for non-commercial purpose.
// Any use of this implementation or a modification of the code
// must acknowledge the work of E. Taillard
/****************************************************************/

using namespace std;
#include <iostream>
#include <fstream>
#include <stdlib.h>

const long infinite = 999999999;

typedef int*   type_vector;
typedef long** type_matrix;


/*************** L'Ecuyer random number generator ***************/
const long m = 2147483647; const long m2 = 2145483479; 
const long a12= 63308; const long q12=33921; const long r12=12979; 
const long a13=-183326; const long q13=11714; const long r13=2883; 
const long a21= 86098; const long q21=24919; const long r21= 7417; 
const long a23=-539608; const long q23= 3976; const long r23=2071;
const double invm = 4.656612873077393e-10;

long x10 = 12345, x11 = 67890, x12 = 13579, // init. de la
     x20 = 24680, x21 = 98765, x22 = 43210; // valeur des germes

double rando()
 {long h, p12, p13, p21, p23;
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
long unif(long low, long high)
 {return(low + long(double(high - low + 1) * rando() ));}

void transpose(int & a, int & b) {long temp = a; a = b; b = temp;}

int min(long a, long b) {if (a < b) return(a); else return(b);}



/*--------------------------------------------------------------*/
/*       compute the cost difference if elements i and j        */
/*         are transposed in permutation (solution) p           */
/*--------------------------------------------------------------*/
long compute_delta(int n, type_matrix & a, type_matrix & b,
                   type_vector & p, int i, int j)
 {long d; int k;
  d = (a[i][i]-a[j][j])*(b[p[j]][p[j]]-b[p[i]][p[i]]) +
      (a[i][j]-a[j][i])*(b[p[j]][p[i]]-b[p[i]][p[j]]);
  for (k = 1; k <= n; k = k + 1) if (k!=i && k!=j)
    d = d + (a[k][i]-a[k][j])*(b[p[k]][p[j]]-b[p[k]][p[i]]) +
            (a[i][k]-a[j][k])*(b[p[j]][p[k]]-b[p[i]][p[k]]);
  return(d);
 }

/*--------------------------------------------------------------*/
/*      Idem, but the value of delta[i][j] is supposed to       */
/*    be known before the transposition of elements r and s     */
/*--------------------------------------------------------------*/
long compute_delta_part(type_matrix & a, type_matrix & b,
                        type_vector & p, type_matrix & delta, 
                        int i, int j, int r, int s)
  {return(delta[i][j]+(a[r][i]-a[r][j]+a[s][j]-a[s][i])*
     (b[p[s]][p[i]]-b[p[s]][p[j]]+b[p[r]][p[j]]-b[p[r]][p[i]])+
     (a[i][r]-a[j][r]+a[j][s]-a[i][s])*
     (b[p[i]][p[s]]-b[p[j]][p[s]]+b[p[j]][p[r]]-b[p[i]][p[r]]) );
  }

void tabu_search(long n,                  // problem size
                 type_matrix & a,         // flows matrix
                 type_matrix & b,         // distance matrix
                 type_vector & best_sol,  // best solution found
                 long & best_cost,        // cost of best solution
                 long min_size,           // parameter 1 (< n^2/2)
                 long max_size,           // parameter 2 (< n^2/2)
                 long aspiration,         // parameter 3 (> n^2/2)
                 long nr_iterations,      // number of iterations 
                 long & best_iter)
           
 
 {type_vector p;                        // current solution
  type_matrix delta;                    // store move costs
  type_matrix tabu_list;                // tabu status
  long current_iteration;               // current iteration
  long current_cost;                    // current sol. value
  int i, j, k, i_retained, j_retained;  // indices

  /***************** dynamic memory allocation *******************/
  p = new int[n+1];
  delta = new long* [n+1];
  for (i = 1; i <= n; i = i+1) delta[i] = new long[n+1];
  tabu_list = new long* [n+1];
  for (i = 1; i <= n; i = i+1) tabu_list[i] = new long[n+1];

  /************** current solution initialization ****************/
  for (i = 1; i <= n; i = i + 1) p[i] = best_sol[i];

  /********** initialization of current solution value ***********/
  /**************** and matrix of cost of moves  *****************/
  current_cost = 0;
  for (i = 1; i <= n; i = i + 1) for (j = 1; j <= n; j = j + 1)
   {current_cost = current_cost + a[i][j] * b[p[i]][p[j]];
    if (i < j) {delta[i][j] = compute_delta(n, a, b, p, i, j);};
   };
  best_cost = current_cost;

  /****************** tabu list initialization *******************/
  for (i = 1; i <= n; i = i + 1) for (j = 1; j <= n; j = j+1)
    tabu_list[i][j] = -(n*i + j);

  /******************** main tabu search loop ********************/
  for (current_iteration = 1; current_iteration <= nr_iterations; 
       current_iteration = current_iteration + 1)
   {/** find best move (i_retained, j_retained) **/

    i_retained = infinite;       // in case all moves are tabu
    long min_delta = infinite;   // retained move cost
    int autorized;               // move not tabu?
    int aspired;                 // move forced?
    int already_aspired = false; // in case many moves forced

    for (i = 1; i < n; i = i + 1) 
      for (j = i+1; j <= n; j = j+1)
       {autorized = (tabu_list[i][p[j]] < current_iteration) || 
                    (tabu_list[j][p[i]] < current_iteration);

        aspired =
         (tabu_list[i][p[j]] < current_iteration-aspiration)||
         (tabu_list[j][p[i]] < current_iteration-aspiration)||
         (current_cost + delta[i][j] < best_cost);                

        if ((aspired && !already_aspired) || // first move aspired
           (aspired && already_aspired &&    // many move aspired
            (delta[i][j] < min_delta)   ) || // => take best one
           (!aspired && !already_aspired &&  // no move aspired yet
            (delta[i][j] < min_delta) && autorized))
          {i_retained = i; j_retained = j;
           min_delta = delta[i][j];
           if (aspired) {already_aspired = true;};
          };
       };

    if (i_retained == infinite) cout << "All moves are tabu! \n"; 
    else 
     {/** transpose elements in pos. i_retained and j_retained **/
      transpose(p[i_retained], p[j_retained]);
      // update solution value
      current_cost = current_cost + delta[i_retained][j_retained];
      // forbid reverse move for a random number of iterations
      tabu_list[i_retained][p[j_retained]] = 
        current_iteration + unif(min_size,max_size);
      tabu_list[j_retained][p[i_retained]] = 
        current_iteration + unif(min_size,max_size);

      // best solution improved ?
      if (current_cost < best_cost)
       {best_cost = current_cost;
        for (k = 1; k <= n; k = k+1) best_sol[k] = p[k];
//         cout << "Solution of value " << best_cost 
//              << " found at iter. " << current_iteration << '\n';
        best_iter = current_iteration;
       };

      // update matrix of the move costs
      for (i = 1; i < n; i = i+1) for (j = i+1; j <= n; j = j+1)
        if (i != i_retained && i != j_retained && 
            j != i_retained && j != j_retained)
         {delta[i][j] = 
            compute_delta_part(a, b, p, delta, 
                               i, j, i_retained, j_retained);}
        else
         {delta[i][j] = compute_delta(n, a, b, p, i, j);};
     };
      
   }; 
  // free memory
  delete[] p;
  for (i=1; i <= n; i = i+1) delete[] delta[i]; delete[] delta;
  for (i=1; i <= n; i = i+1) delete[] tabu_list[i]; 
  delete[] tabu_list;
} // tabu

void generate_random_solution(long n, type_vector  & p)
 {int i;
  for (i = 0; i <= n; i = i+1) p[i] = i;
  for (i = 1; i <  n; i = i+1) transpose(p[i], p[unif(i, n)]);
 }

int n;                    // problem size
type_matrix a, b;         // flows and distances matrices
type_vector solution;     // solution (permutation) 
long cost, iter;          // solution cost and its iteration

// ifstream data_file;       
// char file_name[30];
int i, j;

int main(int argc, char *argv[])
 {/************** read file name and problem size ***************/
  char trash[1000];
//   cout << "Data file name : \n";
//   cin >> file_name; cout << file_name << '\n';
//   data_file.open(file_name);
//   data_file >> n;
  scanf("%d", &n);
  scanf("%[^\n]", trash);

  /****************** dynamic memory allocation ******************/
  a = new long* [n+1];
  for (i = 1; i <= n; i = i+1) a[i] = new long[n+1];
  b = new long* [n+1];
  for (i = 1; i <= n; i = i+1) b[i] = new long[n+1];
  solution = new int[n+1];

  /************** read flows and distances matrices **************/
  for (i = 1; i <= n; i = i+1) for (j = 1; j <= n; j = j+1)
    scanf("%d", &a[i][j]);
//     data_file >> a[i][j];
  for (i = 1; i <= n; i = i+1) for (j = 1; j <= n; j = j+1)
    scanf("%d", &b[i][j]);
//     data_file >> b[i][j];
//   data_file.close();
 
  generate_random_solution(n, solution);
  tabu_search(n, a, b,                     // problem data
              solution, cost,              // tabu search results
              9*n/10, 11*n/10, n*n*2,      // parameters
              1000000,                     // number of iterations 
              iter);

//   cout << "Solution found by tabu search :\n";
//   for (i = 1; i <= n; i = i+1) cout << solution[i] << ' '; 
//   cout << '\n';
  printf("@%d %d [", iter, cost);
  for (i = 1; i <= n; i++) {
    printf("%d", solution[i] - 1);
    if (i < n) printf(",");
  }
  printf("]\n");
  return 0;
 }
