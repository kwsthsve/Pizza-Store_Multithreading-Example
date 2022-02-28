#include "main.h"

pthread_mutex_t countMutex_tel;
pthread_cond_t countThresholdCondition_tel;

pthread_mutex_t countMutex_cook;
pthread_cond_t countThresholdCondition_cook;

pthread_mutex_t countMutex_oven;
pthread_cond_t countThresholdCondition_oven;

pthread_mutex_t countMutex_packer;
pthread_cond_t countThresholdCondition_packer;

pthread_mutex_t countMutex_delivery;
pthread_cond_t countThresholdCondition_delivery;

pthread_mutex_t countMutex_screen;


int PizzaPrice = Cpizza;
int numberOfPhones = Ntel;
int numberOfCooks = Ncook;
int numberOfOvens = Noven;
int numberOfDelivery = Ndeliverer;
int Packer = 1;
unsigned int seed;
int dailyIncome = 0;
int failedOrders = 0;
int approvedOrders = 0;

struct timespec *orderTime;
int *onHoldTime;
int *storeTOhomeTime;
int *pizzaCoolTime;


void *BestPizzaStore(void *t) {
	int threadId = *(int *) t;
	int rcTel;
	int rcCook;
	int rcOven = -1;
	int rcPacker = -1;
	int rcDelivery;
	int rcScreen;
	struct timespec orderTimeReady;
	struct timespec orderTimeDelivered;
	struct timespec customerOnHold;
	struct timespec pizzaReady;
	CUSTOMER cust;
	
	
	
	if (threadId == 1){
		clock_gettime(CLOCK_REALTIME, &orderTime[threadId-1]);
	}else { 
		sleep(rand_r(&seed) % Norderhigh + Norderlow);	
		clock_gettime(CLOCK_REALTIME, &orderTime[threadId-1]);
	}
	
	
		
	// Telephone logic
	
	rcTel = pthread_mutex_lock(&countMutex_tel);
	if (rcTel != 0) {	
		printf("ERROR: return code from pthread_mutex_lock() is %d\n", rcTel);
		pthread_exit(&rcTel);
	}
	
	while (numberOfPhones == 0) {
		rcTel = pthread_cond_wait(&countThresholdCondition_tel, &countMutex_tel);
	}
	
	clock_gettime(CLOCK_REALTIME, &customerOnHold);
	
	// Lock-screen for printing information
	
	rcScreen = pthread_mutex_lock(&countMutex_screen);
	
	printf("Welcome to our pizza store %d\n", threadId);
	
	rcScreen = pthread_mutex_unlock(&countMutex_screen);
	
	
	// Initialize the Customer struct with unique information
	cust.ID = threadId;
	cust.numberOfPizzas = rand_r(&seed) % Torderhigh + Torderlow;
	cust.timeFromFirstOrder = orderTime[cust.ID-1].tv_sec - orderTime[0].tv_sec;
	
	
	// Elapsed time while on hold
	onHoldTime[cust.ID-1] = customerOnHold.tv_sec - orderTime[cust.ID-1].tv_sec;

	numberOfPhones--;
	
	rcTel = pthread_mutex_unlock(&countMutex_tel);
	
	sleep(rand_r(&seed) % Tpaymenthigh + Tpaymentlow);
	
	rcTel = pthread_mutex_lock(&countMutex_tel);
	
	int bernDist = rand_r(&seed) % 100 + 1; 
	
	
	if (bernDist > Pfail){
		
		// Lock-screen for printing information
		rcScreen = pthread_mutex_lock(&countMutex_screen);
		printf("Order of %d pizzas has successfully been scheduled for preparation! Order ID <%d>\n", cust.numberOfPizzas, cust.ID);
		rcScreen = pthread_mutex_unlock(&countMutex_screen);
		
		approvedOrders++;
		dailyIncome += PizzaPrice*cust.numberOfPizzas;
		numberOfPhones++;
	}else {
		
		// Lock-screen for printing information
		rcScreen = pthread_mutex_lock(&countMutex_screen);
		printf("Couldn't complete payment! Order ID <%d> is being cancelled...\n", cust.ID);
		rcScreen = pthread_mutex_unlock(&countMutex_screen);
		
		storeTOhomeTime[cust.ID] = -1;
		pizzaCoolTime[cust.ID] = -1;
		failedOrders++;
		numberOfPhones++;
		rcTel = pthread_cond_signal(&countThresholdCondition_tel);
		rcTel = pthread_mutex_unlock(&countMutex_tel);
		pthread_exit(NULL);
	}	
	
	rcTel = pthread_cond_signal(&countThresholdCondition_tel);
	rcTel = pthread_mutex_unlock(&countMutex_tel);
	
	
	
	// Cook-Oven-Packer logic (avoid DEADLOCK)
	
	rcCook = pthread_mutex_lock(&countMutex_cook);
	if (rcCook != 0) {	
		printf("ERROR: return code from pthread_mutex_lock() is %d\n", rcCook);
		pthread_exit(&rcCook);
	}
	
	while (numberOfCooks == 0) {
		rcCook = pthread_cond_wait(&countThresholdCondition_cook, &countMutex_cook);
	}
	
	numberOfCooks--;
	
	rcCook = pthread_mutex_unlock(&countMutex_cook);
	
	sleep(Tprep*cust.numberOfPizzas);
	
	rcCook = pthread_mutex_lock(&countMutex_cook);
	
	while (pthread_mutex_trylock(&countMutex_oven) != 0){
		/*WAITING FOR LOCK*/
	}	
	
	rcCook = pthread_mutex_unlock(&countMutex_cook);
	
	while (numberOfOvens < cust.numberOfPizzas) {
		rcOven = pthread_cond_wait(&countThresholdCondition_oven, &countMutex_oven);
	}
	
	numberOfOvens -= cust.numberOfPizzas;
	
	rcCook = pthread_mutex_lock(&countMutex_cook); 
	
	numberOfCooks++;
	
	rcCook = pthread_cond_signal(&countThresholdCondition_cook);
	rcCook = pthread_mutex_unlock(&countMutex_cook);
	
	rcOven = pthread_mutex_unlock(&countMutex_oven);
	
	sleep(Tbake);
	clock_gettime(CLOCK_REALTIME, &pizzaReady);
	
	rcOven = pthread_mutex_lock(&countMutex_oven);
	
	while (pthread_mutex_trylock(&countMutex_packer) != 0){
		/*WAITING FOR LOCK*/
	}	
	
	rcOven = pthread_mutex_unlock(&countMutex_oven);
	
	while (Packer == 0) {
		rcCook = pthread_cond_wait(&countThresholdCondition_packer, &countMutex_packer);
	}
	
	Packer--;
	
	rcPacker = pthread_mutex_unlock(&countMutex_packer);
	
	sleep(Tpack);
	clock_gettime(CLOCK_REALTIME, &orderTimeReady);
	
	
	// Lock-screen for printing information
	
	rcScreen = pthread_mutex_lock(&countMutex_screen);
	
	printf("Order with ID <%d> got prepared in %ld minutes!\n", cust.ID, (orderTimeReady.tv_sec-orderTime[cust.ID-1].tv_sec));
	
	rcScreen = pthread_mutex_unlock(&countMutex_screen);
	
	
	
	
	rcPacker = pthread_mutex_lock(&countMutex_packer);
	
	Packer++;
	
	rcOven = pthread_mutex_lock(&countMutex_oven);
	
	numberOfOvens += cust.numberOfPizzas;
	
	rcOven = pthread_cond_signal(&countThresholdCondition_oven);
	rcOven = pthread_mutex_unlock(&countMutex_oven);
	
	rcPacker = pthread_cond_signal(&countThresholdCondition_packer);
	rcPacker = pthread_mutex_unlock(&countMutex_packer);
	
	
	
	// Delivery logic
	
	rcDelivery = pthread_mutex_lock(&countMutex_delivery);
	if (rcDelivery != 0) {	
		printf("ERROR: return code from pthread_mutex_lock() is %d\n", rcDelivery);
		pthread_exit(&rcDelivery);
	}
	
	while (numberOfDelivery == 0) {
		rcDelivery = pthread_cond_wait(&countThresholdCondition_delivery, &countMutex_delivery);
	}
	
	numberOfDelivery--;
	
	rcDelivery = pthread_mutex_unlock(&countMutex_delivery);
	
	int deliveryTime = rand_r(&seed) % Tdelhigh + Tdellow;
	
	sleep(deliveryTime);
	clock_gettime(CLOCK_REALTIME, &orderTimeDelivered);
	
	
	// Lock-screen for printing information
	
	rcScreen = pthread_mutex_lock(&countMutex_screen);
	
	storeTOhomeTime[cust.ID-1] = orderTimeDelivered.tv_sec - orderTime[cust.ID-1].tv_sec;
	pizzaCoolTime[cust.ID-1] = orderTimeDelivered.tv_sec - pizzaReady.tv_sec;
	printf("Order with ID <%d> arrived to destination in %d minutes!\n", cust.ID, storeTOhomeTime[cust.ID-1]);
	
	rcScreen = pthread_mutex_unlock(&countMutex_screen);
	
	
	
	sleep(deliveryTime);
	
	rcDelivery = pthread_mutex_lock(&countMutex_delivery);
	
	numberOfDelivery++;
	
	rcDelivery = pthread_cond_signal(&countThresholdCondition_delivery);
	rcDelivery = pthread_mutex_unlock(&countMutex_delivery);
	
	
	pthread_exit(NULL);
	
}

int main(int argc, char *argv[]) {
	if (argc != 3) {
		printf("ERROR: the program should take two arguments, the number of orders and the seed!\n");
		exit(-1);
	}
	
	int numberOfCustomers = atoi(argv[1]);
	
	seed = atoi(argv[2]);
	
	if (numberOfCustomers < 0) {
		printf("ERROR: the number of orders to execute should be a positive number. Current number given %d.\n", numberOfCustomers);
		exit(-1);
	}
	
	
	pthread_t *threads;
	
	
	// Memory allocation

	threads = malloc(numberOfCustomers * sizeof(pthread_t));
	
	orderTime = malloc(numberOfCustomers * sizeof(struct timespec));
	
	onHoldTime = malloc(numberOfCustomers * sizeof(int));
	
	storeTOhomeTime = malloc(numberOfCustomers * sizeof(int));
	
	pizzaCoolTime = malloc(numberOfCustomers * sizeof(int));
	
	
	// mutex and condition variable initialization
	
	pthread_mutex_init(&countMutex_tel, NULL);
	pthread_cond_init(&countThresholdCondition_tel,NULL);
	
	pthread_mutex_init(&countMutex_cook, NULL);
	pthread_cond_init(&countThresholdCondition_cook,NULL);
	
	pthread_mutex_init(&countMutex_oven, NULL);
	pthread_cond_init(&countThresholdCondition_oven,NULL);
	
	pthread_mutex_init(&countMutex_packer, NULL);
	pthread_cond_init(&countThresholdCondition_packer,NULL);
	
	pthread_mutex_init(&countMutex_delivery, NULL);
	pthread_cond_init(&countThresholdCondition_delivery,NULL);
	
	pthread_mutex_init(&countMutex_screen, NULL);
	

	if ((threads == NULL) || (orderTime == NULL) || (onHoldTime == NULL) || (storeTOhomeTime == NULL) || (pizzaCoolTime == NULL)) {
		printf("NOT ENOUGH MEMORY!\n");
		return -1;
	}
	
	int rc;
	int countArray[numberOfCustomers];
   	int threadCount;
	
	
	// creating the threads with target at BestPizzaStore function
	
   	for(threadCount = 0; threadCount < numberOfCustomers; threadCount++) {
    		
		countArray[threadCount] = threadCount + 1;
    	rc = pthread_create(&threads[threadCount], NULL, BestPizzaStore, &countArray[threadCount]);
		
    	if (rc != 0) {
    		printf("ERROR: return code from pthread_create() is %d\n", rc);
       		exit(-1);
       	}
       	
    }

	// pthread_join so the MAIN thread waits for all the secondary threads to finish
	
	for (threadCount = 0; threadCount < numberOfCustomers; threadCount++) {
		rc = pthread_join(threads[threadCount], NULL);
		
		if (rc != 0) {
			printf("ERROR: return code from pthread_join() is %d\n", rc);
			exit(-1);		
		}
	}
	
	
	
	int maxPizzaCoolTime = pizzaCoolTime[0];
	int maxOnHoldTime = onHoldTime[0];
	int maxStoreToHomeTime = storeTOhomeTime[0];
	
	int sumPCT = 0;
	int sumOHT = 0;
	int sumSTHT = 0;
	
	for (int i = 0; i < numberOfCustomers; i++){
		if (pizzaCoolTime[i] > maxPizzaCoolTime){
			maxPizzaCoolTime = pizzaCoolTime[i];
		}
		
		if (pizzaCoolTime[i] != -1){
			sumPCT += pizzaCoolTime[i];
		}	

		if (onHoldTime[i] > maxOnHoldTime){
			maxOnHoldTime = onHoldTime[i];
		}
		
		sumOHT += onHoldTime[i];
		
		if (storeTOhomeTime[i] > maxStoreToHomeTime){
			maxStoreToHomeTime = storeTOhomeTime[i];
		}

		if (storeTOhomeTime[i] != -1){
			sumSTHT += storeTOhomeTime[i];
		}		
	}

	double averagePizzaCoolTime = (double)sumPCT/ (double)approvedOrders;
	double averageOnHoldTime = (double)sumOHT/ (double)numberOfCustomers;
	double averageStoreToHomeTime = (double)sumSTHT/ (double)approvedOrders;
	
	
	
	// Lock-screen for printing information
	
	rc = pthread_mutex_lock(&countMutex_screen);
	
	printf("\n------------------STATS------------------\n");
	
	printf("\nDAILY INCOME : %d $\n", dailyIncome);
	printf("SUCCESSFULL ORDERS : %d\n", approvedOrders);
	printf("FAILED ORDERS : %d\n", failedOrders);
	printf("\nAVERAGE ON HOLD TIME : %f\n", averageOnHoldTime);
	printf("MAXIMUM ON HOLD TIME : %d\n", maxOnHoldTime);
	printf("\nAVERAGE OVERALL CUSTOMER SERVICE TIME : %f\n", averageStoreToHomeTime);
	printf("MAXIMUM OVERALL CUSTOMER SERVICE TIME : %d\n", maxStoreToHomeTime);
	printf("\nAVERAGE PIZZA COOLING TIME : %f\n", averagePizzaCoolTime);
	printf("MAXIMUM PIZZA COOLING TIME : %d\n", maxPizzaCoolTime);
	
	printf("\n------------------GOODBYE------------------\n");
	
	rc = pthread_mutex_unlock(&countMutex_screen);
	
	
	
	// free the memory-allocated space
	
	free(threads);
	free(orderTime);
	free(onHoldTime);
	free(storeTOhomeTime);
	free(pizzaCoolTime);
	
	// destroy all mutexes and condition variables
	
	pthread_mutex_destroy(&countMutex_tel);
	pthread_cond_destroy(&countThresholdCondition_tel);
	
	pthread_mutex_destroy(&countMutex_cook);
	pthread_cond_destroy(&countThresholdCondition_cook);
	
	pthread_mutex_destroy(&countMutex_oven);
	pthread_cond_destroy(&countThresholdCondition_oven);
	
	pthread_mutex_destroy(&countMutex_packer);
	pthread_cond_destroy(&countThresholdCondition_packer);
	
	pthread_mutex_destroy(&countMutex_delivery);
	pthread_cond_destroy(&countThresholdCondition_delivery);
	
	pthread_mutex_destroy(&countMutex_screen);
	
	
	return 1;
	
}


