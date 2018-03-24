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

int n,k,previous,post;

pthread_barrier_t mybarrier;  //pthread barrier
sem_t sum; //semaphore to protect the average time sum
int count_in,count_out;  //count of readers
double total_avg; //contains average time for readers and writers
double *per_avg; //value stores the average
FILE* output;



void *test_barrier(void *j) //test barrier function 
{
	int prev_time,post_time;
	double average=0;
	int id=intptr_t(j);
	for(int i=0;i<k;i++)
	{
		time_t my_time;
		struct tm * timeinfo; 
		time (&my_time);
		timeinfo = localtime (&my_time); //find the time of system
		fprintf(output,"Going to sleep before the %d barrier invocation at %d:%d by thread %d\n",i+1,timeinfo->tm_min,timeinfo->tm_sec,id);

		srand(previous);
		prev_time = (random()%4000)+1;//simulates the function
		usleep(prev_time);


		timeinfo=localtime(&my_time);  //find the time of system
		fprintf(output,"Before the %d barrier invocation at %d:%d by thread%d\n",i+1,timeinfo->tm_min,timeinfo->tm_sec,id);

		auto start = std::chrono::system_clock::now();//timer starts

		pthread_barrier_wait(&mybarrier);//pthread library barrier fuunction


		auto end = std::chrono::system_clock::now();  //timer ends
		std::chrono::duration<double> elapsed_seconds = end-start;//simulates the function
		average+=elapsed_seconds.count();


		timeinfo=localtime(&my_time);//simulates the function
		fprintf(output,"After the %d barrier invocation at %d:%d by thread%d\n",i+1,timeinfo->tm_min,timeinfo->tm_sec,id);

		timeinfo = localtime (&my_time);//simulates the function
		fprintf(output,"Going to sleep after the %d barrier invocation at %d:%d by thread %d\n",i+1,timeinfo->tm_min,timeinfo->tm_sec,id);
		
		srand(post);//simulates the function
		int post_time = (random()%4000)+1;//simulates the function
		usleep(post_time);//simulates the function

		
		
	}
	per_avg[id-1]=average/(double) k;  //average time
	sem_wait(&sum);
	total_avg+=1000*average/(double)k;//average time
	sem_post(&sum);//average time
	pthread_exit(0);
}


int main()
{
	int i=0;
	sem_init(&sum,0,1);
	ifstream myfile;
	myfile.open("input_param.txt"); //reading from input file
	myfile>>n;
	myfile>>k;
	myfile>>previous;
	myfile>>post;
	pthread_t threads[n];
	pthread_barrier_init(&mybarrier,NULL,n);
	output=fopen("Pthread_barr_log.txt","w");  //creates the output file
	per_avg =new double[n];
	while(i<n)
	{
		int a=pthread_create(&threads[i],NULL,test_barrier,(void*)(intptr_t)(i+1)); //writer function call
		if(a!=0)
		{
			cout<<"Cannot create the thread\n";
			return 0;
		}
		i++;
	}
	for(int i=0;i<n;i++)
	{
		pthread_join(threads[i],NULL);  //waits for all thread to finish execution
	}
	ofstream avg;
    avg.open("Average_time.txt",std::ios_base::app);//average time file
    avg<<"For pthread_barr program******************\n";
    for (int i = 0; i < n; ++i)
    {
    	avg<<"\nAverage time for thread"<<i+1<<" is ="<<1000*per_avg[i]<<"\n";
    }
    avg<<"\n Average time for PTHREAD_BARR program values for n="<<n<<" and k="<<k<<"\n";//average time
    avg<<"Average time for thread is ="<<total_avg/n<<"\n\n\n";//average time
    avg.close();
	fclose(output);
	sem_destroy(&sum);//destroys the semaphore
	delete [] per_avg;
	return 0;
}