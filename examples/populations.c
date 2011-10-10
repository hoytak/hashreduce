

// Testing speed of algorithms
// Lucas Koepke
// October 7, 2011

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <math.h>

// include the needed c file
#include "ibd_fatpack.c"

// function declarations

void generatePopulation(int Ni, int Nr, unsigned int seed, HashTable* population);
void Algorithm2(int Ni, int Nr, unsigned int seed);
void Algorithm3(int Ni, int Nr, unsigned int seed);
void Algorithm4(int Ni, int Nr, unsigned int seed);
void Algorithm5(int Ni, int Nr, unsigned int seed);

int main(int argc, char **argv)
{
    double start, elapsed;

    int i, j, k, choice;
    int Ni[5], Nr[6];

    double results[5][7];
    
    unsigned int seed = 0;

    Ni[0] = 1000; Ni[1] = 10000; Ni[2] = 100000; Ni[3] = 1000000;
    Nr[0] = 1; Nr[1] = 10; Nr[2] = 100; Nr[3] = 1000; Nr[4] = 10000;

    if(argc == 2){
	choice = atoi(argv[1]);
    }

    if(argc !=2){
	printf("Incorrect arguments.  Please specify algorithm number to test:\n");
	printf("Algorithm choices are 2, 3, 4, or 5\n");
	printf("Example: ./population_example 2\n");
    }

    switch(choice)
    {
    case 2:

	printf("\nTesting Algorithm 2 ... \n\n");

	results[0][0] = 0.0;

	for(i=0; i<4; i++){
	    results[i+1][0] = (double)Ni[i];
	    for(j=0; j<5; j++){
		results[0][j+1] = (double)Nr[j];
		start = (double)clock();
		for(k=0;k<10;k++){
		    Algorithm2(Ni[i], Nr[j], seed);
		}
		elapsed = ((double)clock() - start)/CLOCKS_PER_SEC;
		printf("%lf\t%d\t%d\n", elapsed/10, Ni[i], Nr[j]);
		results[i+1][j+1] = elapsed;
	    }
	    printf("\n");
	}


	printf("\nResults for Algorithm 2: \n\n");

	for(i=0; i<5; i++){
	    for(j=0; j<6; j++){
		printf("%lf\t", results[i][j]);
	    }
	    printf("\n");
	}
	break;
    
    case 3:
	printf("\nTesting Algorithm 3 ... \n\n");
	
	results[0][0] = 0.0;

	for(i=0; i<4; i++){
	    results[i+1][0] = (double)Ni[i];
	    for(j=0; j<5; j++){
		results[0][j+1] = (double)Nr[j];
		start = (double)clock();
		for(k=0;k<10;k++){
		    Algorithm3(Ni[i], Nr[j], seed);
		}
		elapsed = ((double)clock() - start)/CLOCKS_PER_SEC;
		printf("%lf\t%d\t%d\n", elapsed/10, Ni[i], Nr[j]);
		results[i+1][j+1] = elapsed;
	    }
	    printf("\n");
	}

	printf("\nResults for Algorithm 3: \n\n");

	for(i=0; i<5; i++){
	    for(j=0; j<6; j++){
		printf("%lf\t", results[i][j]);
	    }
	    printf("\n");
	}
	break;

    case 4:
	printf("\nTesting Algorithm 4 ... \n\n");
	
	results[0][0] = 0.0;

	for(i=0; i<4; i++){
	    results[i+1][0] = (double)Ni[i];
	    for(j=0; j<5; j++){
		results[0][j+1] = (double)Nr[j];
		start = (double)clock();
		for(k=0;k<10;k++){
		    Algorithm4(Ni[i], Nr[j], seed);
		}
		elapsed = ((double)clock() - start)/CLOCKS_PER_SEC;
		printf("%lf\t%d\t%d\n", elapsed/10, Ni[i], Nr[j]);
		results[i+1][j+1] = elapsed;
	    }
	    printf("\n");
	}

	printf("\nResults for Algorithm 4: \n\n");

	for(i=0; i<5; i++){
	    for(j=0; j<6; j++){
		printf("%lf\t", results[i][j]);
	    }
	    printf("\n");
	}
	break;

    case 5:
	printf("\nTesting Algorithm 5 ... \n\n");
	
	results[0][0] = 0.0;

	for(i=0; i<4; i++){
	    results[i+1][0] = (double)Ni[i];
	    for(j=0; j<5; j++){
		results[0][j+1] = (double)Nr[j];
		start = (double)clock();
		for(k=0;k<10;k++){
		    Algorithm5(Ni[i], Nr[j], seed);
		}
		elapsed = ((double)clock() - start)/CLOCKS_PER_SEC;
		printf("%lf\t%d\t%d\n", elapsed/10, Ni[i], Nr[j]);
		results[i+1][j+1] = elapsed;
	    }
	    printf("\n");
	}

	printf("\nResults for Algorithm 5: \n\n");

	for(i=0; i<5; i++){
	    for(j=0; j<6; j++){
		printf("%lf\t", results[i][j]);
	    }
	    printf("\n");
	}
	break;
    default:
	printf("\nError: check options specified.\n\n");
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

    vset_ptr equality_vset = EqualityVSet(population1, hash);
    
    O_DECREF(population1);
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
    O_DECREF(equality_mset);

    return;
}


