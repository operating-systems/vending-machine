#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<time.h>

#define MAX 100
#define SUPPLIER_NUM 5
#define CONSUMER_NUM 8
char day_name[7][5] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
char month_name[12][5] = {"Jan","Feb","Mar","Apr","May","June","July","Aug","Sep","Oct","Nov","Dec"};

pthread_t suppliers_t[SUPPLIER_NUM];
int supplier_num[SUPPLIER_NUM];
char supplier_names[SUPPLIER_NUM][256];
int supplier_interval[SUPPLIER_NUM];
int supplier_repeat[SUPPLIER_NUM];

pthread_t consumers_t[CONSUMER_NUM];
int consumer_num[CONSUMER_NUM];
char consumer_names[CONSUMER_NUM][256];
int consumer_interval[CONSUMER_NUM];
int consumer_repeat[CONSUMER_NUM];

int item_counters[SUPPLIER_NUM];
int consumedItem_num[CONSUMER_NUM];

pthread_mutex_t mutex[SUPPLIER_NUM] = PTHREAD_MUTEX_INITIALIZER;

//Read information from supplier configuration file
void config_sup(int num){
	FILE* fp;
	char file_name[50];
	for(int i = 1; i <= num; i++){
		sprintf(file_name, "supplier%d.txt", i); 
		fp = fopen(file_name, "r");
			
		fscanf(fp, "%[^\n\t]", supplier_names[i-1]);
		fscanf(fp, "%d", &supplier_interval[i-1]);
		fscanf(fp, "%d", &supplier_repeat[i-1]);
		fclose(fp);
	}
}
//Read information from consumer configuration file
void config_con(int num){
	FILE* fp;
	char file_name[50];
	for(int i = 1; i <= num; i++){
		sprintf(file_name, "consumer%d.txt", i); 
		fp = fopen(file_name, "r");
			
		fscanf(fp, "%[^\n\t]", consumer_names[i-1]);
		fscanf(fp, "%d", &consumer_interval[i-1]);
		fscanf(fp, "%d", &consumer_repeat[i-1]);
		fclose(fp);
	}
}
//Add item into vending machine
void* addUnits(void *arg)
{
	unsigned long i = 0;
	//num is supplier number
	int num = *(int*)arg;
	int count_wait = 0; //for repeat
	int interval = supplier_interval[num];

	//Time stamp
	time_t rawtime;
	struct tm * timeinfo;

	//Check if interval is over 60
	if(interval > 60) {
		interval = 60;
	}

	while(1) {

		//Permit only one thread to do work once a time
		pthread_mutex_lock(&mutex[num]);		

		char *output = malloc(100);

		time ( &rawtime );
		timeinfo = localtime ( &rawtime );

		//Print out time and date
		sprintf(output, "%s %s %d %02d:%02d:%02d %d",day_name[timeinfo->tm_wday],month_name[timeinfo->tm_mon],timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec ,timeinfo->tm_year+1900);

		//If the item is not full, so supplier can add an item
		if(item_counters[num] < 100) {

			item_counters[num]++;
            printf("\033[0m%s \033[1;32m%s supplied 1 unit. Going to wait for %d sec\nRemaining items %s: %d\n\n", output, supplier_names[num], interval, supplier_names[num], item_counters[num]);
			interval = supplier_interval[num];
			if(interval > 60) {
				interval = 60;
			}

			count_wait = 0;

			//printf("Remaining items %s: %d\n\n", supplier_names[num], item_counters[num]);
		}

		//Otherwise supplier has to wait for interval time
		else {
			printf("\033[0;32m");
			count_wait++;

			//If count_wait is equal to repeat value then double up the interval value
			if(count_wait >= supplier_repeat[num]){

				interval = interval*2;
	
				//Check if interval is over 60
				if(interval > 60) {
					interval = 60;
				}

				printf("\033[0m%s \033[0;32m%s is full. %s supplier going to wait for %d sec (%d/%d)\n\n", output, supplier_names[num], supplier_names[num], interval, count_wait, supplier_repeat[num]);
				count_wait = 0;
			}
			else {
				printf("\033[0m%s \033[0;32m%s is full. %s supplier going to wait for %d sec (%d/%d)\n\n", output, supplier_names[num], supplier_names[num], interval, count_wait, supplier_repeat[num]);
			}
		}

		//Unlock and give permission to the others
		pthread_mutex_unlock(&mutex[num]);

		//Force supplier to wait for interval time
		sleep(interval);

		free(output);
	}

	return NULL;
}

//Remove item from vending machine
void* removeUnits(void *arg)
{
	unsigned long i = 0;
	//num is consumer number
	int num = *(int*)arg;
	int count_wait = 0; //for repeat
	int interval = consumer_interval[num];

	//Time stamp
	time_t rawtime;
	struct tm * timeinfo;

	//Check if interval is over 60
	if(interval > 60) {
		interval = 60;
	}

	while(1) {
		
		//Permit only one thread to do work once a time
		pthread_mutex_lock( &mutex[consumedItem_num[num]] );
		
		char *output = malloc(100);
		
		time ( &rawtime );
		timeinfo = localtime ( &rawtime );

		//Print out time and date
		sprintf(output, "%s %s %d %02d:%02d:%02d %d",day_name[timeinfo->tm_wday],month_name[timeinfo->tm_mon],timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec ,timeinfo->tm_year+1900);	

		//If the item is not empty, so consumer can consume an item
		if(item_counters[consumedItem_num[num]] > 0) {
            item_counters[consumedItem_num[num]]--;
			printf("\033[0m%s \033[1;31m%s(Thread %d) consumed 1 unit. Going to wait for %d sec\nRemaining items %s: %d\n\n",output, supplier_names[consumedItem_num[num]],num, interval, supplier_names[consumedItem_num[num]], item_counters[consumedItem_num[num]]);
			

			interval = consumer_interval[num];
			if(interval > 60) {
				interval = 60;
			}

			count_wait = 0;
	
			//printf("Remaining items %s: %d\n\n", supplier_names[consumedItem_num[num]], item_counters[consumedItem_num[num]]);
		}

		//Otherwise consumer has to wait for interval time
		else {
			count_wait++;
	
			//If count_wait is equal to repeat value then double up the interval value
			if(count_wait >= consumer_repeat[num]){

				interval = interval*2;

				//Check if interval is over 60
				if(interval > 60) {
					interval = 60;
				}

				printf("\033[0m%s \033[0;31m%s is out of order. %s(Thread %d) consumer going to wait for %d sec (%d/%d)\n\n",output, supplier_names[consumedItem_num[num]], supplier_names[consumedItem_num[num]],num, interval, count_wait, consumer_repeat[num]);
				count_wait = 0;
			}
			else {
				printf("\033[0m%s \033[0;31m%s is out of order. %s(Thread %d) consumer going to wait for %d sec (%d/%d)\n\n",output, supplier_names[consumedItem_num[num]], supplier_names[consumedItem_num[num]],num, interval, count_wait, consumer_repeat[num]);
			}
		}

		//Unlock and give permission to the others
		pthread_mutex_unlock( &mutex[consumedItem_num[num]] );

		//Force consumer to wait for interval time
		sleep(interval);
		free(output);
	}

	return NULL;
}

int main(void)
{
	srand(time(NULL));
	
	//Read information from 5 supplier configuration files
	config_sup(SUPPLIER_NUM);
	//Read information from 8 consumer configuration files
	config_con(CONSUMER_NUM);
	
	//Match suppliers and consumers to item number
	for(int i = 0; i < SUPPLIER_NUM; i++){
		supplier_num[i] = i;
	}
	for(int i = 0; i < CONSUMER_NUM; i++){
		consumer_num[i] = i;
		for(int j = 0; j < SUPPLIER_NUM; j++) {
			if(!strcmp(consumer_names[i], supplier_names[j])) {
				consumedItem_num[i] = j;
			}
		}
	}

	int err;
	
	//Create supplier threads
	for(int i = 0; i < SUPPLIER_NUM; i++)
	{
		err = pthread_create(&(suppliers_t[i]), NULL, &addUnits, &supplier_num[i]);
		if (err != 0)
			printf("\ncan't create thread :[%s]", strerror(err));
	}

	//Create consumer threads
	for(int i = 0; i < CONSUMER_NUM; i++)
	{
		err = pthread_create(&(consumers_t[i]), NULL, &removeUnits, &consumer_num[i]);
		if (err != 0)
			printf("\ncan't create thread :[%s]", strerror(err));
	}

	pthread_exit(NULL);
}
