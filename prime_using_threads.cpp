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
using namespace std;

int n=0,k=0;

vector <int> combination[33];

int primer(int k){ //prime checker function
	if(k == 1 || k == 0)
		return 0;
	for(int i=2; i<=sqrt(k); i++)//calculates whether the number is prime or not
	{
		if(k%i == 0)
			return 0;
	}
	return 1;
}

void * prime_finder(void *ptr)//prime calculator and stroes the result in 2d vector
{
	int l=(intptr_t)(ptr);

	for(int i=l; i<=n; i+=k)
	{
		if(primer(i) == 1)
			combination[l].push_back(i);//store the prime number in the vector
	}

	pthread_exit(0);
}



int main(void)
{	
	cout<<"\nparent process executing=>\n\n";
	do
	{
		cout<<"Enter a postive integer(LESS THAN 8 DIGITS) N:";//asks for n
		cin>>n;
	}while(n<=0);
	do
	{
		cout<<"Enter a postive integer K:";//asks for k
		cin>>k;
	}while(k<=2 && k>32);

	ofstream output("osthread.txt",ios::out);//opening of file
	if(!output)
	{
		cerr<<"File cannot be opened\n";
		return 0;
	}
	auto start = std::chrono::system_clock::now();//clocks starts 
	pthread_t tid[33];

	for(int i=1; i<=k; i++)
		pthread_create(&tid[i],NULL,prime_finder,(void*)(intptr_t)(i));	//thread creation	

	for(int i=1; i<=k; i++)
		pthread_join(tid[i],NULL);//combine all the threads 

	for(int i=1; i<=k; i++)
	{
		output<<"Thread number"<<i<<"\n";
		for(int j=0; j<combination[i].size(); j++)
		{
			output<<combination[i][j]<<"\n";//outputs the results in file
		}
		output<<"\n"<<"\n";
	}
	auto end = std::chrono::system_clock::now();//clocks stops
	std::chrono::duration<double> elapsed_seconds = end-start;//time calculation
	output<<"\n\nTIME TAKEN===\n";
	output<<elapsed_seconds.count();//output of time
	output.close();
	return 0;
}