using namespace std;
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <math.h>
#include <time.h>

/****************************************************************/
/*
	Solver for Quadratic Bottleneck Assignment Problem
	by Ikki Fujiwara <ikki@nii.ac.jp>  2012/09/12
	based on:

    This programme implement a simulated annealing for the
    quadratic assignment problem along the lines describes in
    the article D. T. Connoly, "An improved annealing scheme for 
    the QAP", European Journal of Operational Research 46, 1990,
    93-100.

    Compiler : g++ or CC should work. 

    Author : E. Taillard, 
             EIVD, Route de Cheseaux 1, CH-1400 Yverdon, Switzerland

    Date : 16. 3. 98

    Format of data file : Example for problem nug5 :

5

0 1 1 2 3
1 0 2 1 2
1 2 0 1 2
2 1 1 0 1
3 2 2 1 0

0 5 2 4 1
5 0 3 0 2
2 3 0 0 0
4 0 0 0 5
1 2 0 5 0

   Additionnal parameters : Number of iterations, number of runs

*/

/********************************************************************/

const long n_max = 1401; // Max number of element + 1
const long infini = 1399999999;
const long nb_iter_initialisation = 1000; // Connolly proposes nb_iterations/100

typedef long type_vecteur[n_max];
typedef long type_matrice[n_max][n_max];

/*--------------- choses manquantes -----------------*/
enum booleen {faux, vrai};


long max(long a, long b) {if (a > b) return(a); else return(b);};
double max(double a, double b) {if (a > b) return(a); else return(b);}
long min(long a, long b) {if (a < b) return(a); else return(b);}
double min(double a, double b) {if (a < b) return(a); else return(b);}
void swap(long &a, long &b) {long temp = a; a = b; b = temp;}

double temps() {return(double(clock())/double(1000));}

void a_la_ligne(ifstream & fichier_donnees)
{char poubelle[1000]; fichier_donnees.getline(poubelle, sizeof(poubelle));}
/*-------------------------------------------------*/

/************* random number generators ****************/

const long m = 2147483647; const long m2 = 2145483479; 
const long a12 = 63308; const long q12 = 33921; const long r12 = 12979; 
const long a13 = -183326; const long q13 = 11714; const long r13 = 2883; 
const long a21 = 86098; const long q21 = 24919; const long r21 = 7417; 
const long a23 = -539608; const long q23 = 3976; const long r23 = 2071;
const double invm = 4.656612873077393e-10;
long x10 = 12345, x11 = 67890, x12 = 13579, 
     x20 = 24680, x21 = 98765, x22 = 43210;

double mon_rand()
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

long unif(long low, long high)
 {return(low + long(double(high - low + 1) *  mon_rand() ));
 }

/************************** sa for qap ********************************/

void lire(long &n, type_matrice &a, type_matrice &b)
 {ifstream fichier_donnees;
  char nom_fichier[30];
  long i, j;

  cout << "Data file name : \n";
  cin >> nom_fichier;
  cout << nom_fichier << '\n';
  fichier_donnees.open(nom_fichier);
  fichier_donnees >> n; a_la_ligne(fichier_donnees);
  for (i = 1; i <= n; i = i+1) for (j = 1; j <= n; j = j+1)
    fichier_donnees >> a[i][j];
  for (i = 1; i <= n; i = i+1) for (j = 1; j <= n; j = j+1)
    fichier_donnees >> b[i][j];
  fichier_donnees.close();
 }

#if 1
long calc_delta_cost(long n, type_matrice & a, type_matrice & b,
                     type_vecteur & p, long r, long s, long cost) {
	long d1;
	long cost1 = 0;
	long pi, pj;
	
	for (long i = 1; i <= n; i++) {
		for (long j = 1; j <= n; j++) {
			pi = (i == r) ? p[s] : (i == s) ? p[r] : p[i];
			pj = (j == r) ? p[s] : (j == s) ? p[r] : p[j];
			cost1 = max(cost1, a[i][j] * b[pi][pj]);
		}
	}
	d1 = cost1 - cost;
	
	return(d1);
}
#else
long calc_delta_cost(long n, type_matrice & a, type_matrice & b,
                     type_vecteur & p, long r, long s, type_vecteur & q) {
	// q[giving_the_max_cost] = 1, q[other] = 0
	long d1, d2;
	long cost1 = 0;
	long cost2 = 0;
	long pi, pj;
	
	for (long i = 1; i <= n; i++) {
		for (long j = 1; j <= n; j++) {
			pi = (i == r) ? p[s] : (i == s) ? p[r] : p[i];
			pj = (j == r) ? p[s] : (j == s) ? p[r] : p[j];
			cost1 = max(cost1, a[i][j] * b[pi][pj]);
		}
	}
	d1 = cost1 - cost;
	
	/*
	// “ü‚ê‘Ö‚¦ˆÊ’u‚ÉŒ³‚ÌÅ‘å’l‚ª‚ ‚é‚©‚È‚¢‚©‚ÅˆÓ–¡‚ª•Ï‚í‚éB‚±‚ê‚¶‚á‚¾‚ß
	cost2 = cost;
	for (long k = 1; k <= n; k++) {
		cost2 = max(cost2, a[r][k] * b[p[s]][p[k]]);
		cost2 = max(cost2, a[s][k] * b[p[r]][p[k]]);
		cost2 = max(cost2, a[k][r] * b[p[k]][p[s]]);
		cost2 = max(cost2, a[k][s] * b[p[k]][p[r]]);
	}
	d2 = cost2 - cost;
	
	if (cost1 != cost2) {
		cout << "cost=" << cost << ", " << cost1 << ", " << cost2 << endl;
	}
	*/
	
	return(d1);
}
#endif

long calc_cost(long n, type_matrice & a, type_matrice & b, type_vecteur & p) {
	long i, j;
	long cost = 0;
	for (i = 1; i <= n; i++) {
		for (j = 1; j <= n; j++) {
			cost = max(cost, a[i][j] * b[p[i]][p[j]]);
		}
	}
	return(cost);
}

void tire_solution_aleatoire(long n, type_vecteur & p)
 {long i;
  for (i = 1; i <= n; i = i+1) p[i] = i;
  for (i = 2; i <= n; i = i+1) swap(p[i-1], p[unif(i-1, n)]);
 }

void recuit(long n, type_matrice & a, type_matrice & b,
            type_vecteur & meilleure_sol, long & min_cost, 
            long nb_iterations)

 {type_vecteur p;
  type_vecteur q;
  long i, r, s;
  long delta;
  double cpu = temps();
  long k = n*(n-1)/2, mxfail = k, nb_fail, no_iteration;
  long dmin = infini, dmax = 0;
  double t0, tf, beta, tfound, temperature;
  long best_iter;

  for (i = 1; i <= n; i = i + 1) p[i] = meilleure_sol[i];
  for (i = 1; i <= n; i = i + 1) q[i] = 0;
  long cost = calc_cost(n, a, b, p);
  min_cost = cost;

  for (no_iteration = 1; no_iteration <= nb_iter_initialisation;
       no_iteration = no_iteration+1)
   {r = unif(1, n);
    s = unif(1, n-1);
    if (s >= r) s = s+1;

    delta = calc_delta_cost(n, a, b, p, r, s, cost);
    if (delta > 0)
     {dmin = min(dmin, delta); dmax = max(dmax, delta);}; 
    cost = cost + delta;
    swap(p[r], p[s]);
   };
  t0 = dmin + (dmax - dmin)/10.0;
  tf = dmin;
  beta = (t0 - tf)/(nb_iterations*t0*tf);

  nb_fail = 0;
  tfound = t0;
  temperature = t0;
  r = 1; s = 2;
  for (no_iteration = 1; 
       no_iteration <= nb_iterations - nb_iter_initialisation; 
       no_iteration = no_iteration + 1)
    { temperature = temperature / (1.0 + beta*temperature);

      s = s + 1;
      if (s > n)
       {r = r + 1; 
        if (r > n - 1) r = 1;
        s = r + 1;
       };

      delta = calc_delta_cost(n, a, b, p, r, s, cost);
      if ((delta < 0) || (mon_rand() < exp(-double(delta)/temperature)) ||
           mxfail == nb_fail)
       {cost = cost + delta; swap(p[r], p[s]); nb_fail = 0;}
      else nb_fail = nb_fail + 1;

      if (mxfail == nb_fail) {beta = 0; temperature = tfound;};
      if (cost < min_cost)
       {min_cost = cost;
        for (i = 1; i <= n; i = i + 1) meilleure_sol[i] = p[i];
        tfound = temperature;
//         cout << "Iteration = " << no_iteration  
//              << "  Cost = " << min_cost 
//              << "  Computational time = " << temps() - cpu <<  '\n';
        best_iter = no_iteration;
       };
 
   };

//   cout << "Best solution found : \n";
//   for (i = 1; i <= n; i = i + 1) cout << meilleure_sol[i] << ' ';
//   cout << '\n';
  printf("@%d %d [", best_iter, min_cost);
  for (i = 1; i <= n; i++) {
    printf("%d", meilleure_sol[i] - 1);
    if (i < n) printf(",");
  }
  printf("]\n");
 }


long  n, nb_iterations, nb_res, no_res;
long cost;
type_matrice a, b;
type_vecteur p;

int main(int argc, char *argv[])
 {
  long i, j;
  char trash[1000];
  if (argc < 3) {
    printf("QAP MinMax solver v2.01 (%s) by Ikki\n", __DATE__);
    printf("Usage: %s #iteration #trial < qap_file\n", argv[0]);
    return EXIT_FAILURE;
  }
//   lire(n, a, b);
//   cout << "nr iterations, nr resolutions : \n";
//   cin >> nb_iterations >> nb_res;
  nb_iterations = atoi(argv[1]);
  nb_res = atoi(argv[2]);
  scanf("%d", &n);
  if (n > n_max - 1) {
    cerr << "N=" << n << " exceeds Nmax=" << n_max << ". Exit." << endl;
    return EXIT_FAILURE;
  };
  scanf("%[^\n]", trash);
  for (i = 1; i <= n; i++) for (j = 1; j <= n; j++)
    scanf("%d", &a[i][j]);
  for (i = 1; i <= n; i++) for (j = 1; j <= n; j++)
    scanf("%d", &b[i][j]);
  for (no_res = 1; no_res <= nb_res; no_res = no_res + 1)
  for (no_res = 1; no_res <= nb_res; no_res = no_res + 1)
   {tire_solution_aleatoire(n, p);
    recuit(n, a, b, p, cost, nb_iterations);
   };
 }
