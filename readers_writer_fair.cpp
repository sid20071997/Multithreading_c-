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

sem_t in;  //semaphore in
sem_t out; //semaphore out
sem_t sum;   ///semaphore out
sem_t wrt;   //semaphore wrt
int inside=0;
int outside=0;
bool writer_request=false; //request of writer 
double total_avg[2];  //stores the averaeg waiting time for reader and witer
FILE* output;



void * reader_func(void *n){  //reader function
	
	double average=0;
	int id=(intptr_t)(n);  //id
	for(int i=0;i<kr;i++){
		time_t my_time;
		struct tm * timeinfo; 
		time (&my_time);
		timeinfo = localtime (&my_time);
		auto start = std::chrono::system_clock::now();
		fprintf(output,"%dth CS request by Thread %d at %d:%d\n",i+1,id,timeinfo->tm_min,timeinfo->tm_sec);


		//sem_wait(&mutex);
		sem_wait(&in);     //wait call
		inside++;
		sem_post(&in);   //signal call
		timeinfo = localtime (&my_time);
		sem_wait(&out);    //awit call


		fprintf(output,"%dth CS Entry by Thread %d at %d:%d\n",i+1,id,timeinfo->tm_min,timeinfo->tm_sec);//entry to cs
		srand(cmSeed);
		int  randCSTime = (random()%40)+1;
		usleep(randCSTime);
		tm *pre=localtime(&my_time);


		outside++;      //number of writers waiting
		if(writer_request==true && inside==outside){
			sem_post(&wrt);
		}
		sem_post(&out);   //signal


		fprintf(output,"%dth CS exit by reader thread%d at %d:%d \n",i+1,id,1+pre->tm_min,1+pre->tm_sec);
		auto end = std::chrono::system_clock::now();
		std::chrono::duration<double> elapsed_seconds = end-start;
		average+=elapsed_seconds.count();
		srand(rmSeed);
		int randRemTime = (random()%40)+1;
		usleep(randRemTime);
	}


	sem_wait(&sum);    //wait call
	total_avg[0]+=1000*average/(double)kr;
	sem_post(&sum);  //signal caall
	pthread_exit(0);    //pthread exit
}

void * writer_func(void * n){    //writer call this function
	double average=0;
	int id=intptr_t(n);
	time_t reqTime,enterTime,exitTime;
	for(int i=0;i<kw;i++){
		time_t my_time;
		struct tm * timeinfo; 
		time (&my_time);
		timeinfo = localtime (&my_time);
		auto start = std::chrono::system_clock::now();
		fprintf(output,"%dth CS request by writer Thread %d at %d:%d\n",i+1,id,timeinfo->tm_min,timeinfo->tm_sec);


		sem_wait(&in);    //wait calll
		sem_wait(&out);   //signal call
		if(inside==outside){
			sem_post(&out);  //signal call
		}
		else{
			writer_request=true;
			sem_post(&out);
			sem_wait(&wrt);
			writer_request=false;
		}

		timeinfo = localtime (&my_time);
		fprintf(output,"%dth CS Entry by writer Thread %d at %d:%d\n",i+1,id,timeinfo->tm_min,timeinfo->tm_sec);//entry to cs
		srand(cmSeed);
		int  randCSTime = (random()%40)+1;
		usleep(randCSTime);
		timeinfo=localtime(&my_time);
		fprintf(output,"%dth CS exit by writer thread%d at %d:%d \n",i+1,id,1+timeinfo->tm_min,1+timeinfo->tm_sec);
		auto end = std::chrono::system_clock::now();
		sem_post(&in);
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
	output=fopen("FairRW_log.txt","w");
	sem_init(&in,0,1);//semaphore initialised to 1
	sem_init(&out,0,1);//semaphore initialised to 1
	sem_init(&sum,0,1);//semaphore initialised to 1
	sem_init(&wrt,0,0);//semaphore initialised to 1
	ifstream myfile;
	myfile.open("input_param.txt");
	myfile>>nw;
	myfile>>nr;
	myfile>>kw;
	myfile>>kr;
	myfile>>cmSeed;
	myfile>>rmSeed;
	int k=0;
	pthread_t writer[nw];
	pthread_t reader[nr];
	while(k<nw){
		int a=pthread_create(&writer[k],NULL,writer_func,(void*)(intptr_t)(k+1));
		if(a!=0){
			cout<<"Cannot create the thread\n";
			return 0;
		}
		k++;
	}
	k=0;
	while(k<nr){
		int b=pthread_create(&reader[k],NULL,reader_func,(void*)(intptr_t)(k+1));
		if(b!=0){
			cout<<"Cannot create the thread\n";
			return 0;
		}
		k++;
	}
	for(int i=0;i<nr;i++){
		pthread_join(reader[i],NULL);
	}
	for(int i=0;i<nw;i++){
		pthread_join(writer[i],NULL);
	}
	ofstream avg;
    avg.open("Average_time.txt",std::ios_base::app);//average time file
    avg<<"\n Average time for RW FAIR program values for kr="<<kr<<" and kw="<<kw<<"\n";
    avg<<"Average time forthread is="<<(total_avg[1]+total_avg[0])/2<<"\n\n\n";
    avg.close();
	//cout<<total_avg[0]<<"\n";
	//cout<<total_avg[1]<<"\n";
	fclose(output);
	sem_destroy(&in);
	sem_destroy(&out);
	sem_destroy(&wrt);
	sem_destroy(&sum);
	return 0;
}






