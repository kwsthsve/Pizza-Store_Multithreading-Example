// APIS

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>


// Our constant values

#define Ntel 3
#define Ncook 2
#define Noven 10
#define Ndeliverer 7
#define Torderlow 1
#define Torderhigh 5
#define Norderlow 1
#define Norderhigh 5
#define Tpaymentlow 1
#define Tpaymenthigh 2
#define Cpizza 10
#define Pfail 5
#define Tprep 1
#define Tbake 10
#define Tpack 2
#define Tdellow 5
#define Tdelhigh 15


// Customer struct definition

typedef struct customer {
	int ID;
	int numberOfPizzas;
	int timeFromFirstOrder;
} CUSTOMER;


void *BestPizzaStore(void *t);