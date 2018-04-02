#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<time.h>

#define MAX 100
#define SUPPLIER_NUM 5
#define CONSUMER_NUM 8
//int supplier_num = 5;
pthread_t suppliers_t[SUPPLIER_NUM];
char suppliers[SUPPLIER_NUM][256];
int supplier_interval[SUPPLIER_NUM];
int supplier_repeat[SUPPLIER_NUM];

//int consumer_num = 8;
pthread_t consumers_t[CONSUMER_NUM];
char consumers[CONSUMER_NUM][256];
int consumer_interval[CONSUMER_NUM];
int consumer_repeat[CONSUMER_NUM];

int goods_counters[SUPPLIER_NUM];

void config_sup(int num){
	FILE* fp;
	char file_name[50];
	for(int i = 1; i <= num; i++){
		sprintf(file_name, "supplier%d.txt", i); 
		fp = fopen(file_name, "r");
			
		fscanf(fp, "%[^\n\t]", suppliers[i-1]);
		fscanf(fp, "%d", &supplier_interval[i-1]);
		fscanf(fp, "%d", &supplier_repeat[i-1]);
		fclose(fp);
	}
}
void config_con(int num){
	FILE* fp;
	char file_name[50];
	for(int i = 1; i <= num; i++){
		sprintf(file_name, "consumer%d.txt", i); 
		fp = fopen(file_name, "r");
			
		fscanf(fp, "%[^\n\t]", consumers[i-1]);
		fscanf(fp, "%d", &consumer_interval[i-1]);
		fscanf(fp, "%d", &consumer_repeat[i-1]);
		fclose(fp);
	}
}
void* addUnits(void *arg)
{
	unsigned long i = 0;
	int num = (int)arg;
	int count_wait = 0, multiplier = 1; //for repeat

	sleep(supplier_interval[num]);
	while(1) {
		//printf("%d\n\n", num);
		if(goods_counters[num] < 100) {
			printf("Supplier \"%s\" adds an item\n", suppliers[num]);	
			goods_counters[num]++;

			multiplier = 1;
			count_wait = 0;

			printf("Remaining items \"%s\": %d\n\n", suppliers[num], goods_counters[num]);
		}
		else {
			printf("Supplier \"%s\" going to wait\n\n", suppliers[num]);
			count_wait++;
			if(count_wait % supplier_repeat[num] == 0){
				if(supplier_interval[num]*multiplier <= 60)
					multiplier *= 2;
			}		
		}
		sleep(supplier_interval[num]*multiplier);
	}

	return NULL;
}

void* removeUnits(void *arg)
{
	unsigned long i = 0;
	int num = (int)arg;

	int r;

	r = rand() % SUPPLIER_NUM;

	sleep(consumer_interval[num]);
	while(1) {
		if(goods_counters[r] > 0) {
			printf("Consumer \"%s\" removes an item\n", consumers[num]);	
			goods_counters[r]--;
	
			printf("Remaining items \"%s\": %d\n\n", suppliers[r], goods_counters[r]);
		}
		else {
			printf("Consumer \"%s\" waiting for \"%s\" \n\n", consumers[num], suppliers[r]);
		}
		sleep(consumer_interval[num]);
	}

	return NULL;
}

int main(void)
{
	srand(time(NULL));
	
	config_sup(SUPPLIER_NUM);
	config_con(CONSUMER_NUM);

	for(int i = 0; i < SUPPLIER_NUM; i++){
		printf("%s\n", suppliers[i]);
		printf("%d\n", supplier_interval[i]);
		printf("%d\n", supplier_repeat[i]);
	}
	for(int i = 0; i < CONSUMER_NUM; i++){
		printf("%s\n", consumers[i]);
		printf("%d\n", consumer_interval[i]);
		printf("%d\n", consumer_repeat[i]);
	}

	int err;

	for(int i = 0; i < SUPPLIER_NUM; i++)
	{	printf("%d\n", i);
		err = pthread_create(&(suppliers_t[i]), NULL, &addUnits, (void*)(int)i);
		if (err != 0)
			printf("\ncan't create thread :[%s]", strerror(err));
	}

	sleep(10);
	for(int i = 0; i < CONSUMER_NUM; i++)
	{
		err = pthread_create(&(consumers_t[i]), NULL, &removeUnits, (void*)(int)i);
		if (err != 0)
			printf("\ncan't create thread :[%s]", strerror(err));
	}

	pthread_exit(NULL);
}
