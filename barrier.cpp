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

sem_t mutex; //semaphore mutex
sem_t waiting; //semaphore rwmutex
sem_t sum;  //semaphore to protect the average time
int count_in,count_out;  //count of readers
double total_avg; //contains average time for readers and writers
double * per_avg;
FILE* output;
bool flag;


void barrier_init()  //funtion to initialise the barrier
{
	sem_init(&mutex,0,1); //mutex initialisation
	sem_init(&waiting,0,0); //waiting semaphore
	count_in=0;
	count_out=0;
	flag=false;//flag to wait for n processes
	return;
}

void barrier_point()
{
	sem_wait(&mutex);
	count_in++;        //increments the count in protection
	sem_post(&mutex);

	if(count_in==n)
	{
		flag=false;
		sem_post(&waiting);  //flag for waiting 
	}
	sem_wait(&waiting);
	sem_post(&waiting);

	sem_wait(&mutex);
	count_out++;     //increments the count in protection
	sem_post(&mutex);

	while(count_out%n!=0);
	sem_wait(&mutex);
	if(flag==false)
	{
		flag=true;
		count_in=0;
		sem_init(&waiting,0,0);//seamphore reinitialisation
	}
	sem_post(&mutex);
}

void *test_barrier(void *j)
{
	int prev_time,post_time;
	double average=0;
	int id=intptr_t(j);
	for(int i=0;i<k;i++)
	{
		time_t my_time;
		struct tm * timeinfo; 
		time (&my_time);
		timeinfo = localtime (&my_time);
		fprintf(output,"Going to sleep before the %d barrier invocation at %d:%d by thread %d\n",i+1,timeinfo->tm_min,timeinfo->tm_sec,id);

		//timeinfo = localtime (&my_time);
		//fprintf(output,"%dth CS Entry by writer Thread %d at %d:%d\n",i+1,id,timeinfo->tm_min,timeinfo->tm_sec);//entry to cs
		

		srand(previous);//simulates the function
		prev_time = (random()%4000)+1;//simulates the function
		usleep(prev_time);//simulates the function


		timeinfo=localtime(&my_time);
		fprintf(output,"Before the %d barrier invocation at %d:%d by thread%d\n",i+1,timeinfo->tm_min,timeinfo->tm_sec,id);

		auto start = std::chrono::system_clock::now();//simulates the function
		barrier_point();
		auto end = std::chrono::system_clock::now();
		std::chrono::duration<double> elapsed_seconds = end-start;//simulates the function
		average+=elapsed_seconds.count();//simulates the function


		timeinfo=localtime(&my_time);
		fprintf(output,"After the %d barrier invocation at %d:%d by thread%d\n",i+1,timeinfo->tm_min,timeinfo->tm_sec,id);

		timeinfo = localtime (&my_time);
		fprintf(output,"Going to sleep after the %d barrier invocation at %d:%d by thread %d\n",i+1,timeinfo->tm_min,timeinfo->tm_sec,id);
		
		srand(post);//simulates the function
		int post_time = (random()%4000)+1;//simulates the function
		usleep(post_time);//simulates the function

		
		
	}
	per_avg[id-1]=average/(double) k;//average time
	sem_wait(&sum);//average time
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
	per_avg=new double[n];
	barrier_init();
	output=fopen("New_barr_log.txt","w");
	while(i<n)
	{
		int a=pthread_create(&threads[i],NULL,test_barrier,(void*)(intptr_t)(i+1)); //thread creation
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
    avg<<"For new_barr program******************\n";
    for (int i = 0; i < n; ++i)
    {
    	avg<<"\nAverage time for thread"<<i+1<<" is ="<<1000*per_avg[i]<<"\n";//average time
    }
    avg<<"\n Average time for NEW_BARR program values for n="<<n<<" and k="<<k<<"\n";//average time
    avg<<"Average time for thread is="<<total_avg/n<<"\n\n\n";//average time
    avg.close();
	fclose(output);
	sem_destroy(&mutex);//destroys the semaphore
	sem_destroy(&waiting);//destroys the semaphore
	sem_destroy(&sum);//destroys the semaphore
	delete [] per_avg;
	return 0;
}