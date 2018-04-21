#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define NUM_THREADS 16
#define SUPPLIER_NUM 5
#define CUNSUMER_NUM 8

pthread_mutex_t lock;

char goodsName[5][257] = {"Null","Null","Null","Null","Null"}; // array to keep goods' name
int goods[5] = {0,0,0,0,0}; //number of each good
int goods_num = 0; //number of goods that were supplied

int chk_finishArr[] = {0,0,0,0,0,0,0,0,0,0,0,0,0}; //use to check if all thread finish

// structure of supplier and comsumer var.
struct arg_struct {
	long tid; //thread number
	int num; //number of role
	char name[257]; 
	int interval;
	int repeat;
};

void *supplier(void *arguments)
{
	pthread_mutex_lock(&lock); //lock

	struct arg_struct *supplier1 = arguments; //structure var to get information
	
	//keep information on vars
	char name[257];
	strcpy(name,supplier1 -> name);
	int interval = supplier1 -> interval;
	int repeat = supplier1 -> repeat;
	int num = supplier1 -> num;
	long tid = supplier1 -> tid;

	int chk = 0; //chk if u can add goods or not
	int goods_index = 0; //index of goods in global arrays (goodsName,goods)
	int chk_repeat = 0; //the repeated time, use to compair
	
	int localInterval = interval; //current interval time

	for(int i=0;i<SUPPLIER_NUM;i++) //loop chk if goods already exist
	{
		if(strcmp(goodsName[i],name) == 0)
		{
			chk = 1;
			goods_index = i; //index of this goods
		}
	}

	if(chk == 0) //if goods not exist, index and create it
	{
		if(goods_num < 5) //if slot is not full
		{
			goods_index = goods_num++; //indexing
			strcpy(goodsName[goods_index], name); //add good's name
			chk = 1;
		}
		else //if slot full (this case would not come from this requirement-> pigeon hole)
		{
			printf("slot is already Full");
		}
	}

	if(chk == 1) //ready to add
	{
		
		while(chk_repeat < repeat) //loop untill complete the repeat number
		{
			// sleep ... seconds(interval)
			unsigned int time_to_sleep = localInterval;
			while(time_to_sleep)
				time_to_sleep = sleep(time_to_sleep);

			if(goods[goods_index] < 100) //if goods' number under 100 pieces
			{
				//add good
				goods[goods_index]++;
				chk_repeat++;

				//current interval time set to initial value
				localInterval = interval;

				//print log
				time_t ltime;
				time(&ltime); 
				char *t = ctime(&ltime);
				if (t[strlen(t)-1] == '\n') t[strlen(t)-1] = '\0';
				printf("%s supplier %d supplied %s one unit. Stock after = %d\n", t, num, goodsName[goods_index], goods[goods_index]);
				
				//write output.txt
				FILE *f = fopen("output.txt", "a");
				fprintf(f, "%s supplier %d supplied %s one unit. Stock after = %d\n", t, num, goodsName[goods_index], goods[goods_index]);
				fclose(f);

			}
			else //if goods' number over 100 pieces
			{
				if(localInterval*2 < 60) localInterval *= 2; //interval time X2

				//print log
				time_t ltime;
				time(&ltime); 
				char *t = ctime(&ltime);
				if (t[strlen(t)-1] == '\n') t[strlen(t)-1] = '\0';
				printf("%s %s supplier %d going to wait (%d seconds).\n", t, name, num, localInterval);
				
				//write output.txt
				FILE *f = fopen("output.txt", "a");
				fprintf(f, "%s %s supplier %d going to wait (%d seconds).\n", t, name, num, localInterval);
				fclose(f);
			}

		}
		

	}

	//chk if all the threads finished
	chk_finishArr[tid] = 1;
	int chk_finish = 0;
	for(int i=0;i<13;i++)
	{
		if(chk_finishArr[i] == 0)
		{
			chk_finish = 1;
		}
	}
	if(!chk_finish)
	{

		FILE *f = fopen("output.txt", "a");
		 
		int sum = 0;
		for(int i=0;i<goods_num;i++) sum += goods[i];
		printf("All threads completed. Final stock = %d\n",sum);
		
		fprintf(f, "All threads completed. Final stock = %d\n",sum);
		for(int j=0;j<goods_num;j++)
		{
			//if all thread finished, print log
			printf("%s remain : %d",goodsName[j], goods[j]);
			//write output.txt
			fprintf(f, "%s remain : %d",goodsName[j], goods[j]);
		}
		fclose(f);
	}


	pthread_mutex_unlock(&lock); //unlock

	
}

void *consumer(void *arguments)
{
	pthread_mutex_lock(&lock); //lock

	struct arg_struct *consumer1 = arguments; //structure var to get information

	//keep information on vars
	char name[257];
	strcpy(name,consumer1 -> name);
	int interval = consumer1 -> interval;
	int repeat = consumer1 -> repeat;
	int num = consumer1 -> num;
	long tid = consumer1 -> tid;

	int chk = 0; //chk if u can add goods or not
	int goods_index = 0; //index of goods in global arrays (goodsName,goods)
	int chk_repeat = 0; //the repeated time, use to compair

	int localInterval = interval; //current interval time

	while(chk_repeat < repeat) //loop untill complete the repeat number
	{
		// sleep ... seconds(interval)
		unsigned int time_to_sleep = localInterval;
		while(time_to_sleep)
			time_to_sleep = sleep(time_to_sleep);

		//loop chk if goods already exist
		for(int i=0;i<SUPPLIER_NUM;i++)
		{
			if(strcmp(goodsName[i],name) == 0)
			{
				chk = 1;
				goods_index = i; //if good exist, get index
			}
		}

		if(chk == 1) //if have good
		{
			if(goods[goods_index] > 0) //can consume (have more than 0)
			{
				//consume
				goods[goods_index]--;
				chk_repeat++;
				
				//current interval time set to initial value
				localInterval = interval; 

				//print log
				time_t ltime;
				time(&ltime); 
				char *t = ctime(&ltime);
				if (t[strlen(t)-1] == '\n') t[strlen(t)-1] = '\0';
				printf("%s consumer %d consumed %s one unit. Stock after = %d\n",t, num, goodsName[goods_index], goods[goods_index]);

				//write output.txt
				FILE *f = fopen("output.txt", "a");
				fprintf(f, "%s consumer %d consumed %s one unit. Stock after = %d\n",t, num, goodsName[goods_index], goods[goods_index]);
				fclose(f);
				

			}
			else //if goods' number not enough
			{
				if(localInterval*2 <= 60) localInterval *= 2; //interval time X2

				//print log
				time_t ltime;
				time(&ltime); 
				char *t = ctime(&ltime);
				if (t[strlen(t)-1] == '\n') t[strlen(t)-1] = '\0'; 
				printf("%s %s consumer %d going to wait (%d seconds).\n", t, name, num, localInterval);

				//write output.txt
				FILE *f = fopen("output.txt", "a");
				fprintf(f, "%s %s consumer %d going to wait (%d seconds).\n", t, name, num, localInterval);
				fclose(f);
			}

		}
		
		else //good is not exist and hasn't printed yet
		{
			if(localInterval*2 <= 60) localInterval *= 2;

			//print log
			time_t ltime;
			time(&ltime); 
			char *t = ctime(&ltime);
			if (t[strlen(t)-1] == '\n') t[strlen(t)-1] = '\0'; 
			printf("%s %s consumer %d going to wait (%d seconds).\n", t, name, num, localInterval);

			//write output.txt
			FILE *f = fopen("output.txt", "a");
			fprintf(f, "%s %s consumer %d going to wait (%d seconds).\n", t, name, num, localInterval);
			fclose(f);
		}
		
	}

	//chk if all the threads finished
	chk_finishArr[tid] = 1;
	int chk_finish = 0;
	for(int i=0;i<13;i++)
	{
		if(chk_finishArr[i] == 0)
		{
			chk_finish = 1;
		}
	}
	if(!chk_finish)
	{
		FILE *f = fopen("output.txt", "a");

		int sum = 0;
		for(int i=0;i<goods_num;i++) sum += goods[i];
		printf("All threads completed. Final stock = %d\n",sum);

		fprintf(f, "All threads completed. Final stock = %d\n",sum);

		for(int j=0;j<goods_num;j++)
		{
			//if all thread finished, print log
			printf("%s remain : %d\n",goodsName[j], goods[j]);
			//write output.txt
			fprintf(f, "%s remain : %d\n",goodsName[j], goods[j]);
		}
		fclose(f);
	}

	pthread_mutex_unlock(&lock); //unlock
}


int main (int argc, char *argv[])
{
	pthread_t threads[NUM_THREADS]; //thread
	int rc;
	long t;

	//clear file output.txt
	fclose(fopen("output.txt", "w"));
	
	//keep config flie name (txt)
	char listTxt[13][257] = {"supplier1.txt","supplier2.txt","supplier3.txt","supplier4.txt","supplier5.txt"
				,"consumer1.txt","consumer2.txt","consumer3.txt","consumer4.txt","consumer5.txt"
				,"consumer6.txt","consumer7.txt","consumer8.txt"};

	char line[3][257];
	FILE *plist = NULL; 

	//check lock
	if (pthread_mutex_init(&lock, NULL) != 0)
	{
 		printf("\n mutex init failed\n");
        	return 1;
    	}
	
	//crate 5 supplier vars
	struct arg_struct supplierArr[5];

	//thread 0-4 for suppliers
	for(t=0; t<5; t++)
	{
		int i = 0;

		//read txt file
		plist = fopen(listTxt[t], "r");
		while(fgets(line[i], 257, plist)) {
			/* get rid of ending \n from fgets */
			line[i][strlen(line[i]) - 1] = '\0';
			i++;
		}

		fclose(plist);

		//keep value in vars
	    	strcpy(supplierArr[t].name,line[0]);
	    	supplierArr[t].interval = atoi(line[1]);
		supplierArr[t].repeat = atoi(line[2]);
		supplierArr[t].tid = t;
		supplierArr[t].num = t+1;

		//create thread
		printf("In main: creating thread %ld\n", t);
		rc = pthread_create(&threads[t], NULL, supplier, (void *) &supplierArr[t]);
		if (rc){
			printf("ERROR; return code from pthread_create() is %d\n", rc);
			exit(-1);
		}
		printf("\tsupplier %d %s interval : %d repeat : %d\n",supplierArr[t].num, supplierArr[t].name, supplierArr[t].interval, supplierArr[t].repeat);
	}
	
	//crate 5 consumer vars
	struct arg_struct consumerArr[5];

	//thread 5-12 for consumers
	for(t=5; t<13; t++)
	{
		int i = 0;
	
		//read txt file
		plist = fopen(listTxt[t], "r");
		while(fgets(line[i], 257, plist)) {
			// get rid of ending \n from fgets 
			line[i][strlen(line[i]) - 1] = '\0';
			i++;
		}

		fclose(plist);

		//keep value in vars
	    	strcpy(consumerArr[t].name,line[0]);
	    	consumerArr[t].interval = atoi(line[1]);
		consumerArr[t].repeat = atoi(line[2]);
		consumerArr[t].tid = t;
		consumerArr[t].num = t - 4;

		//create thread
		printf("In main: creating thread %ld\n", t);
		rc = pthread_create(&threads[t], NULL, consumer, (void *) &consumerArr[t]);
		if (rc){
			printf("ERROR; return code from pthread_create() is %d\n", rc);
			exit(-1);
		}
		printf("\tconsumer %d %s interval : %d repeat : %d\n",consumerArr[t].num, consumerArr[t].name, consumerArr[t].interval, consumerArr[t].repeat);
	}
	
	printf("\n");
	
	/* close thread */
	
	pthread_mutex_destroy(&lock);
	pthread_exit(NULL);
}
