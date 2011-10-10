

// Testing speed of algorithms
// Lucas Koepke
// October 7, 2011

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <math.h>

// include the needed c file; normally, this should be handled with
// the linker.
#include "ibd_fatpack.c"

#define TABLE_PRINT_WIDTH  12
#define NUM_REPITITIONS    10

// function declarations
void generatePopulation(int Ni, int Nr, unsigned int seed, HashTable* population);
void Algorithm2(int Ni, int Nr, unsigned int seed);
void Algorithm3(int Ni, int Nr, unsigned int seed);
void Algorithm4(int Ni, int Nr, unsigned int seed);
void Algorithm5(int Ni, int Nr, unsigned int seed);
void print_time(double t);

int main(int argc, char **argv)
{
  double start, elapsed;

  int choice;
  int Ni[6], Nr[6];

  unsigned int seed = 0;

  Ni[0] = 100; 
  Ni[1] = 1000; 
  Ni[2] = 10000; 
  Ni[3] = 100000; 
  Ni[4] = 1000000;
  Ni[5] = 10000000;
    
  Nr[0] = 1; 
  Nr[1] = 10; 
  Nr[2] = 100; 
  Nr[3] = 1000; 
  Nr[4] = 10000;
  Nr[5] = 100000;

  int ani, Ni_start = 0, Ni_end = 6;
  int anr, Nr_start = 0, Nr_end = 6;

  if(argc == 4){
    choice = atoi(argv[1]);

    ani = atoi(argv[2]);
    if(ani == 0 || ani > Nr_end || ani < -Nr_end)
      goto BAD_VALUE;
    if(ani < 0) {
      ++ani;
      Ni_start = 0;
      Ni_end = -ani + 1;
    } else {
      --ani;
      Ni_start = ani;
      Ni_end = ani + 1;
    }

    anr = atoi(argv[3]);

    if(anr == 0 || anr > Nr_end || anr < -Nr_end)
      goto BAD_VALUE;

    if(anr < 0) {
      ++anr;
      Nr_start = 0;
      Nr_end = -anr + 1;
    } else {
      --anr;
      Nr_start = anr;
      Nr_end = anr + 1;
    }

  } else {
  BAD_VALUE:
    printf("\nUsage format: %s <algorithm #> <pop size> <num intervals>. \n", argv[0]);
    printf("Algorithm choices are 2, 3, 4, or 5 (match paper).\n");
    printf("Population size choices are: 1 (100), 2 (1000), 3 (10000), 4 (100000), 5 (1000000), 6 (10000000)\n");
    printf("# Validity Intervals: choices are: 1 (1), 2 (10), 3 (100), 4 (1000), 5 (10000), 6 (100000)\n");
    printf("Passing -n instead of n runs all smaller choices as well.\n\n");
    
    exit(1);
  }

  int ri, ii, k;

  printf("\nTiming Algorithm %d.  Columns # of validity intevals, Rows # of indivduals. Times given in seconds, average of %d.\n\n", choice, NUM_REPITITIONS);

  /* Get the table header. */
  printf("%-*c ", TABLE_PRINT_WIDTH, ' ');
	
  for(ri=Nr_start; ri != Nr_end; ++ri)
    printf("%-*d ", TABLE_PRINT_WIDTH, Nr[ri]);

  printf("\n");

  for(ii=Ni_start; ii!=Ni_end; ++ii) {
    printf("%-*d ", TABLE_PRINT_WIDTH, Ni[ii]);
    fflush(stdout);

    for(ri=Nr_start; ri != Nr_end; ++ri) {
      start = (double)clock();

      for(k=0;k<NUM_REPITITIONS;k++){
	switch(choice) {
	case 2:
	  Algorithm2(Ni[ii], Nr[ri], 100*k + seed);
	case 3:
	  Algorithm3(Ni[ii], Nr[ri], 100*k + seed);
	case 4:
	  Algorithm4(Ni[ii], Nr[ri], 100*k + seed);
	case 5:
	  Algorithm5(Ni[ii], Nr[ri], 100*k + seed);
	}
      }

      elapsed = ((double)clock() - start)/CLOCKS_PER_SEC;
     
      printf("%-*lf ", TABLE_PRINT_WIDTH, elapsed/10);
      fflush(stdout);
    }
    printf("\n");
    fflush(stdout);
  }

  return 0;
}
  
/*****************************************************/
// FUNCTION DEFINITIONS

static inline bool int_order(int a1, int a2) {
  return (a1 < a2);
}

KSORT_INIT(int, int, int_order);

// generates population of specified size Ni
void generatePopulation(int Ni, int Nr, unsigned int seed, HashTable *population)
{
  int i, j, a, b;
  int NUM = 100000;
  int *v = (int*)malloc(2*Nr*sizeof(int));

  LCGState rng_state = Lcg_New(seed);

  for(i=0; i<Ni; i++){
	
    /* form random Nr random integers between 0 and 10000000, then
     * sort the list */
    for(j=0; j<2*Nr; j++)
      v[j] = (int)(Lcg_Next(&rng_state) % NUM);

    ks_introsort_int(2*Nr, v);
    
    /* prints list of random numbers generated */
    /* for(j=0; j<2*Nr; j++){ */
    /*     printf("%d\t%d\n", j, v[j]); */
    /* } */

    /* Add validity range to population for each of Ni individuals*/
    HashObject* temp = Hf_FromInt(NULL, Ni);
       
    /* adding each validity range */
    for(j = 0; j < Nr; j += 2){
      a = v[j];
      b = v[j+1];
      H_AddMarkerValidRange(temp, a, b);
    }

    Ht_Give(population, temp);
  }

  free(v);

  return;
}


void Algorithm2(int Ni, int Nr, unsigned int seed){

    
  HashTable* population1 = NewHashTable();
  HashTable* population2 = NewHashTable();

  generatePopulation(Ni, Nr, seed, population1);
  generatePopulation(Ni, Nr, seed+1, population2);

  vset_ptr equality_vset = EqualityVSet(population1, population2);
    
  O_DECREF(population1);
  O_DECREF(population2);
  O_DECREF(equality_vset);

  return;
}


void Algorithm3(int Ni, int Nr, unsigned int seed){

    
  HashTable* population1 = NewHashTable();
  HashTable* population2 = NewHashTable();
  HashTable* population3 = NewHashTable();

  generatePopulation(Ni, Nr, seed, population1);
  generatePopulation(Ni, Nr, seed+1, population2);
  generatePopulation(Ni, Nr, seed+2, population3);

  HashTable* keys = KeySet(population3);

  HashTable* intersection1 = MSetIntersection(population1, keys);
  HashTable* intersection2 = MSetIntersection(population2, keys);
    

  vset_ptr equality_vset = EqualityVSet(intersection1, intersection2);

  O_DECREF(intersection1);
  O_DECREF(intersection2);

  O_DECREF(population1);
  O_DECREF(population2);
  O_DECREF(population3);
  O_DECREF(equality_vset);

  return;
}


void Algorithm4(int Ni, int Nr, unsigned int seed){

  HashTable* population1 = NewHashTable();

  generatePopulation(Ni, Nr, seed, population1);

  int NUM = 100000;

  LCGState rng_state = Lcg_New(seed);

  int t = (int)(Lcg_Next(&rng_state) % NUM); /* random time t */

  obj_ptr hash = HashAtMarker(population1, t);

  vset_ptr equality_vset = EqualToHash(population1, hash);
    
  O_DECREF(population1);
  O_DECREF(hash);
  O_DECREF(equality_vset);

  return;
}


void Algorithm5(int Ni, int Nr, unsigned int seed){
    
  HashTable* population1 = NewHashTable();
  HashTable* population2 = NewHashTable();

  generatePopulation(Ni, Nr, seed, population1);
  generatePopulation(Ni, Nr, seed+1, population2);

  HashTable* keys = KeySet(population2);   

  mset_ptr equality_mset = MSetDifference(population1, keys);

  O_DECREF(population1);
  O_DECREF(population2);
  O_DECREF(keys);
  O_DECREF(equality_mset);

  return;
}
