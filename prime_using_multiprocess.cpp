#include <iostream>
#include <string>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <chrono>
#include <fstream>
using namespace std;
int k;
int primer(int n) //process to find whether the number is prime or not
{   
	int i, flag = 0;
	for(i=2; i<=sqrt(n); ++i)//checks till squareroot
	{
		if(n%i==0)
		{
			flag=1;
			break;
		}
	}

	return flag;
}


int main()
{
	int n;
	pid_t pid;
	cout<<"\nparent process executing=>\n\n";
	do
	{
		cout<<"Enter a postive integer(LESS THAN 8 DIGITS) N:";//asks the user for n
		cin>>n;
	}while(n<=0);
	do
	{
		cout<<"Enter a postive integer K:";//asks the user for k
		cin>>k;
	}while(k<=2 && k>32);
	const int SIZE=(n/k)*sizeof(int); // size of one shared memory
	auto start = std::chrono::system_clock::now();//time measurment starts
	for(int i=1;i<=k;i++)
	{
		
		pid=fork();//child creation
		if(pid==0)
		{
			char* name=new char[2];//child process execution
			sprintf(name,"%d",i);
			int shm_id;
			int *ptr;
			shm_id=shm_open(name, O_CREAT | O_RDWR, 0666);
			ftruncate(shm_id,SIZE+3);
			ptr = (int *)mmap(0,SIZE+3, PROT_WRITE, MAP_SHARED,shm_id, 0);//shared memory for each chiild 
			int p=0;
			for(int j=i;j<n;j=j+k)
			{
				if(primer(j)==0 && j!=1)//storing the primes number
				{
					ptr[p]=j;
					p++;
				}
			}
			ptr[p]=n;
			return 0;
		}

	}
	ofstream output("osprocess.txt",ios::out);//file creation
	if(!output)
	{
		cerr<<"File cannot be opened\n";
		return 0;
	}
	for(int i=1;i<=k;i++)//waits till child completes
		wait(NULL);
	for(int i=1;i<=k;i++)
	{
		
		if(pid>0)
		{
			char* name=new char[2];
			sprintf(name,"%d",i);
			int shm_id;
			int *ptr;
			shm_id=shm_open(name,O_RDONLY, 0666);//opens the memory
			ftruncate(shm_id, SIZE+3);
			int l=0;
			ptr = (int *)mmap(0, SIZE+3, PROT_READ, MAP_SHARED,shm_id, 0); //shared memory reads for each child
			output<<"FOR CHILD=="<<i<<"\n";
			while(ptr[l]!=n)
			{
				output<<ptr[l]<<"\n";
				l++;
			}
			output<<"\n";
			shm_unlink(name);//unlink the memory
		} 
		
	}
	auto end = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_seconds = end-start;//clocks stops
	output<<"\n\nTIME TAKEN===\n";
	output<<elapsed_seconds.count();//measured time
	output.close();
	return 0;
}