#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <ctime>
#include <time.h>
#include <chrono>
#include <pthread.h>
#include <semaphore.h>
#include <vector>
#include <random>
#include <string.h>
#include <fstream>


using namespace std;

int nw,nr,kw,kr,cmSeed,rmSeed;

sem_t mutex; //semaphore mutex
sem_t rwmutex; //semaphore rwmutex
sem_t sum;
int read_count=0;  //count of readers
double total_avg[2]; //contains average time for readers and writers
FILE* output;



void * reader_func(void *n){ //reader thread function
	double average=0;
	int id=(intptr_t)(n);
	for(int i=0;i<kr;i++){
		time_t my_time;
		struct tm * timeinfo; 
		time (&my_time);
		timeinfo = localtime (&my_time);
		auto start = std::chrono::system_clock::now();
		fprintf(output,"%dth CS request by Thread %d at %d:%d\n",i+1,id,timeinfo->tm_min,timeinfo->tm_sec);


		sem_wait(&mutex);     //protecting mutual exclusion
		read_count++;
		if(read_count==1){
			sem_wait(&rwmutex); //wait
		}
		sem_post(&mutex);   //signals


		timeinfo = localtime (&my_time);
		fprintf(output,"%dth CS Entry by Thread %d at %d:%d\n",i+1,id,timeinfo->tm_min,timeinfo->tm_sec);//entry to cs
		srand(cmSeed);
		int  randCSTime = (random()%40)+1;
		usleep(randCSTime);
		tm *pre=localtime(&my_time);
		fprintf(output,"%dth CS exit by reader thread%d at %d:%d \n",i+1,id,1+pre->tm_min,1+pre->tm_sec);


		sem_wait(&mutex);    //wait call
		auto end = std::chrono::system_clock::now();
		read_count--;
		if(read_count==0){
			sem_post(&rwmutex);  //signals
		}
		sem_post(&mutex);   //signals


		std::chrono::duration<double> elapsed_seconds = end-start;
		average+=elapsed_seconds.count();
		srand(rmSeed);
		int randRemTime = (random()%40)+1;
		usleep(randRemTime);
	}
	sem_wait(&sum);
	total_avg[0]+=1000*average/(double)kr;
	sem_post(&sum);
	pthread_exit(0);
}

void * writer_func(void * n){ //writers thread function 
	double average=0;
	int id=intptr_t(n);
	for(int i=0;i<kw;i++){
		time_t my_time;
		struct tm * timeinfo; 
		time (&my_time);
		timeinfo = localtime (&my_time);
		auto start = std::chrono::system_clock::now();
		fprintf(output,"%dth CS request by writer Thread %d at %d:%d\n",i+1,id,timeinfo->tm_min,timeinfo->tm_sec);


		sem_wait(&rwmutex);  //protecting mutual exclusion


		timeinfo = localtime (&my_time);
		fprintf(output,"%dth CS Entry by writer Thread %d at %d:%d\n",i+1,id,timeinfo->tm_min,timeinfo->tm_sec);//entry to cs
		srand(cmSeed);
		int  randCSTime = (random()%40)+1;
		usleep(randCSTime);
		timeinfo=localtime(&my_time);
		fprintf(output,"%dth CS exit by writer thread%d at %d:%d \n",i+1,id,1+timeinfo->tm_min,1+timeinfo->tm_sec);
		auto end = std::chrono::system_clock::now();


		sem_post(&rwmutex);   //signals


		std::chrono::duration<double> elapsed_seconds = end-start;
		average+=elapsed_seconds.count();
		srand(rmSeed);
		int randRemTime = (random()%40)+1;
		usleep(randRemTime);
		
	}
	sem_wait(&sum);
	total_avg[1]+=1000*average/(double)kr;
	sem_post(&sum);
	pthread_exit(0);
}


int main(){
	output=fopen("RW_log.txt","w");
	sem_init(&mutex,0,1);  //semaphore initialised to 1
	sem_init(&rwmutex,0,1);//semaphore initialised to 1
	sem_init(&sum,0,1);//semaphore initialised to 1
	ifstream myfile;
	myfile.open("input_param.txt"); //reading from input file
	myfile>>nw;
	myfile>>nr;
	myfile>>kw;
	myfile>>kr;
	myfile>>cmSeed;
	myfile>>rmSeed;
	int k=0;
	pthread_t writer[nw];  //create the array of writers thread
	pthread_t reader[nr];  //create the array of readers thread
	while(k<nw){
		int a=pthread_create(&writer[k],NULL,writer_func,(void*)(intptr_t)(k+1)); //writer function call
		if(a!=0){
			cout<<"Cannot create the thread\n";
			return 0;
		}
		k++;
	}
	k=0;
	while(k<nr){
		int b=pthread_create(&reader[k],NULL,reader_func,(void*)(intptr_t)(k+1)); //reader function call
		if(b!=0){
			cout<<"Cannot create the thread\n";
			return 0;
		}
		k++;
	}
	for(int i=0;i<nr;i++){
		pthread_join(reader[i],NULL);  //waits for all thread to finish execution
	}
	for(int i=0;i<nw;i++){
		pthread_join(writer[i],NULL);   //waits for all thread to finish execution
	}
	ofstream avg;
    avg.open("Average_time.txt",std::ios_base::app);//average time file
    avg<<"\n Average time for RW program values for kr="<<kr<<" and kw="<<kw<<"\n";
    avg<<"Average time forthread is="<<(total_avg[1]+total_avg[0])/2<<"\n\n\n";
    avg.close();
	//cout<<total_avg[0]<<"\n";
	//cout<<total_avg[1]<<"\n";
	fclose(output);
	sem_destroy(&mutex);
	sem_destroy(&rwmutex);
	sem_destroy(&sum);
	return 0;
}






