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
int supplier_num[SUPPLIER_NUM];
char supplier_names[SUPPLIER_NUM][256];
int supplier_interval[SUPPLIER_NUM];
int supplier_repeat[SUPPLIER_NUM];

//int consumer_num = 8;
pthread_t consumers_t[CONSUMER_NUM];
int consumer_num[CONSUMER_NUM];
char consumer_names[CONSUMER_NUM][256];
int consumer_interval[CONSUMER_NUM];
int consumer_repeat[CONSUMER_NUM];

int item_counters[SUPPLIER_NUM];
int consumedItem_num[CONSUMER_NUM];

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
void* addUnits(void *arg)
{
	unsigned long i = 0;
	int num = *(int*)arg;
	int count_wait = 0; //for repeat
	int interval = supplier_interval[num];
	time_t rawtime;
	struct tm * timeinfo;

	if(interval > 60) {
		interval = 60;
	}

	//sleep(interval);
	while(1) {
		
		char *output = malloc(100);
		
		time ( &rawtime );
		timeinfo = localtime ( &rawtime );

		sprintf(output, "[%d %d %d %02d:%02d:%02d]",timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
		printf("\033[0m");		
		printf("%s\n", output);
		
		if(item_counters[num] < 100) {
			printf("\033[1;32m");
			printf("%s supplied 1 unit. Stock after = %d\n", supplier_names[num], interval);	
			item_counters[num]++;

			interval = supplier_interval[num];
			if(interval > 60) {
				interval = 60;
			}

			count_wait = 0;

			printf("Remaining items #%d: %d\n\n", num, item_counters[num]);
		}
		else {
			printf("\033[0;32m");
			count_wait++;
			if(count_wait >= supplier_repeat[num]){
				interval = interval*2;
				if(interval > 60) {
					interval = 60;
				}
				printf("%s supplier going to wait %d sec (%d/%d)\n\n", supplier_names[num], interval, count_wait, supplier_repeat[num]);
				count_wait = 0;
			}
			else {
				printf("%s supplier going to wait %d sec (%d/%d)\n\n", supplier_names[num], interval, count_wait, supplier_repeat[num]);
			}
		}
		sleep(interval);
		free(output);
	}

	return NULL;
}

void* removeUnits(void *arg)
{
	unsigned long i = 0;
	int num = *(int*)arg;
	int count_wait = 0; //for repeat
	int r;
	int interval = consumer_interval[num];

	if(interval > 60) {
		interval = 60;
	}

	//sleep(interval);
	while(1) {

		r = rand() % SUPPLIER_NUM;

		if(item_counters[consumedItem_num[num]] > 0) {
			printf("\033[1;31m");
			printf("%s consumed 1 unit. stock after = %d\n", supplier_names[consumedItem_num[num]], interval);	
			item_counters[consumedItem_num[num]]--;

			interval = consumer_interval[num];
			if(interval > 60) {
				interval = 60;
			}

			count_wait = 0;
	
			printf("Remaining items #%d: %d\n\n", consumedItem_num[num], item_counters[consumedItem_num[num]]);
		}
		else {
			printf("\033[0;31m");
			count_wait++;
			if(count_wait >= consumer_repeat[num]){
				interval = interval*2;
				if(interval > 60) {
					interval = 60;
				}
				printf("%s consumer going to wait %d sec (%d/%d)\n\n", supplier_names[consumedItem_num[num]], interval, count_wait, consumer_repeat[num]);
				count_wait = 0;
			}
			else {
				printf("%s consumer going to wait %d sec (%d/%d)\n\n", supplier_names[consumedItem_num[num]], interval, count_wait, consumer_repeat[num]);
			}
		}
		sleep(interval);
	}

	return NULL;
}

int main(void)
{
	srand(time(NULL));
	
	config_sup(SUPPLIER_NUM);
	config_con(CONSUMER_NUM);

	for(int i = 0; i < SUPPLIER_NUM; i++){
		supplier_num[i] = i;
		printf("%s\n", supplier_names[i]);
		printf("%d\n", supplier_interval[i]);
		printf("%d\n", supplier_repeat[i]);
	}
	for(int i = 0; i < CONSUMER_NUM; i++){
		for(int j = 0; j < SUPPLIER_NUM; j++) {
			if(!strcmp(consumer_names[i], supplier_names[j])) {
				consumedItem_num[i] = j;
			}
		}
		printf("%s\n", consumer_names[i]);
		printf("%d\n", consumer_interval[i]);
		printf("%d\n", consumer_repeat[i]);
	}

	int err;

	

	for(int i = 0; i < SUPPLIER_NUM; i++)
	{
		err = pthread_create(&(suppliers_t[i]), NULL, &addUnits, &supplier_num[i]);
		if (err != 0)
			printf("\ncan't create thread :[%s]", strerror(err));
	}

	for(int i = 0; i < CONSUMER_NUM; i++)
	{
		consumer_num[i] = i;
		err = pthread_create(&(consumers_t[i]), NULL, &removeUnits, &consumer_num[i]);
		if (err != 0)
			printf("\ncan't create thread :[%s]", strerror(err));
	}

	pthread_exit(NULL);
}
