#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>		
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <math.h>	
#include <pthread.h>
#include <stdint.h>
#include <vector>
#include <chrono>
#include <iostream>
#include <fstream>
#include <atomic>
#include <random>
#include <time.h>
#include <thread>
using namespace std;

atomic_flag flag=ATOMIC_FLAG_INIT;
double* avg_time=NULL;//contains average time
FILE* output;

void  testCS(int a,int b,int c,int d)//function invoked by each thread
{
	int id =a,k=b;
	int csSeed=c, remSeed=d;
	for(int i=1; i<=k ;i++)
	{
		time_t my_time;
		struct tm * timeinfo; 
		time (&my_time);
		timeinfo = localtime (&my_time);
		auto start = std::chrono::system_clock::now();
		fprintf(output,"%dth CS request by Thread %d at %d:%d\n",i,id,timeinfo->tm_min,timeinfo->tm_sec);//request for cs
		while(atomic_flag_test_and_set(&flag))
		;
		timeinfo = localtime (&my_time);
		fprintf(output,"%dth CS Entry by Thread %d at %d:%d\n",i,id,timeinfo->tm_min,timeinfo->tm_sec);//entry to cs
		srand(csSeed);
		int  randCSTime = (random()%40)+1;
		usleep(randCSTime);
		timeinfo = localtime (&my_time);
		fprintf(output,"%dth CS Exit by Thread %d at %d:%d\n",i,id,timeinfo->tm_min,timeinfo->tm_sec);//entry to cs
		auto end = std::chrono::system_clock::now();
		std::chrono::duration<double> elapsed_seconds = end-start;
		avg_time[id]+=elapsed_seconds.count();
		atomic_flag_clear(&flag);
		srand(remSeed);
		int randRemTime = (random()%40)+1;
		usleep(randRemTime);
	}
}

int main()
{
	ifstream myfile;
	output = fopen("TAS_log.txt", "w+");
	int n,k,csSeed,remSeed;
	double total;
	myfile.open("a.txt");//read the input from file a.txt
	myfile>>n;
	myfile>>k;
	myfile>>csSeed;
	myfile>>remSeed;
	std::vector<std::thread> th;
	avg_time=new double[n+1];
	for(int i=1; i<=n; i++)
		th.emplace_back(testCS,i,k,csSeed,remSeed);

	for (auto& t : th) //waits for thread to join
	{
        t.join();
    }
    ofstream avg;
    avg.open("Average_time",std::ios_base::app);//average time file
    for(int i=1;i<=n;i++)
    {
    	total+=avg_time[i];
    }
    avg<<"Average time for thread TAS for n="<<n<<"\n";
    avg<<"Average time for thread TAS is=="<<(total*1000)/(n*k)<<"\n\n\n";
    avg.close();
    fclose(output);
	return 0;
}


