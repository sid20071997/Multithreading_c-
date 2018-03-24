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

//atomic_flag flag=ATOMIC_FLAG_INIT;
atomic<int>flag(0);
double* avg_time=NULL;  //contains average time
FILE* output;


void  testCS(int a,int b,int c,int d,int e) //function invoked by each thread
{
	int id =a,k=b,n=e;
	int csSeed=c, remSeed=d;
	bool key;
	for(int i=0; i<k ;i++)
	{
		time_t my_time;
		struct tm * timeinfo; 
		time (&my_time);
		timeinfo = localtime (&my_time);
		auto start = std::chrono::system_clock::now();
		fprintf(output,"%dth CS request by Thread %d at %d:%d\n",i+1,id,timeinfo->tm_min,timeinfo->tm_sec);//request for cs
		while(1)
		{
			int x=0,y=1;
			if(flag.compare_exchange_weak(x,y)!=0) //compare and swap function
				break;
		}
		timeinfo = localtime (&my_time);
		fprintf(output,"%dth CS Entry by Thread %d at %d:%d\n",i+1,id,timeinfo->tm_min,timeinfo->tm_sec); //entry to cs
		srand(csSeed);
		int  randCSTime = (random()%40)+1;
		usleep(randCSTime);
		timeinfo = localtime (&my_time);
		fprintf(output,"%dth CS Exit by Thread %d at %d:%d\n",i+1,id,timeinfo->tm_min,timeinfo->tm_sec);//exit the cs 
		flag=0;
		auto end = std::chrono::system_clock::now();
		std::chrono::duration<double> elapsed_seconds = end-start;
		avg_time[id-1]+=elapsed_seconds.count();
		srand(remSeed);
		int randRemTime = (random()%40)+1;
		usleep(randRemTime);
	}
}

int main()
{
	ifstream myfile;
	output = fopen("CAS_log.txt", "w+"); //inputs a file 
	int n,k,csSeed,remSeed;
	double total;
	myfile.open("a.txt");//read the input from file a.txt
	myfile>>n;
	myfile>>k;
	myfile>>csSeed;
	myfile>>remSeed;
	std::vector<std::thread> th;
	//waiting=new bool[n+1];
	avg_time=new double[n+1];
	for(int i=0; i<n; i++)
		th.emplace_back(testCS,i+1,k,csSeed,remSeed,n);

	for (auto& t : th) //waits for thread to join
	{
        t.join();
    }
    ofstream avg;
    avg.open("Average_time",std::ios_base::app);//average time file
    for(int i=0;i<=n;i++)
    {
    	total+=avg_time[i];
    }
    avg<<"Average time for thread CAS for n="<<n<<"\n";
    avg<<"Average time for thread CAS is=="<<(total*1000)/(n*k)<<"\n\n\n";
    avg.close();
    fclose(output);
	return 0;
}


