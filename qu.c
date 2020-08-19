#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

#define FIFOSIZE 100

int FIFO[FIFOSIZE];
int numElements, front, added, removed;

struct termios tty;

int sPort;

pthread_mutex_t printerLock, FIFOlock;

void printerTask(char *x)
{
	printf(x);
}

void FIFOinit()
{
	added = 0;
	removed = 0;
	numElements = -1;
	front = 0;
}

void FIFOput(int data)
{
	if(numElements == -1)
		numElements = 0;
	FIFO[numElements] = data;
	numElements = (numElements+1)%FIFOSIZE;
	added+=1;
	return;
}

int FIFOget(void)
{
	int temp;
	if(added <= removed)
	{
		pthread_mutex_lock(&printerLock);
		printerTask("No new elements\n");
		pthread_mutex_unlock(&printerLock);
		return -1;
	}		
	else
	{
		temp = FIFO[front];
		front = (front+1) % FIFOSIZE;
		removed+=1;
		return temp;
	}	
}

void* firstTask(void *args)
{
	sPort = open("/dev/ttyACM0", O_RDWR);
	
	if(sPort < 0)
	{
		pthread_mutex_lock(&printerLock);
		printf("Error %i from open %s\n", errno, strerror(errno));
		pthread_mutex_unlock(&printerLock);
	}
	
	memset(&tty, 0, sizeof(tty));
	
	if(tcgetattr(sPort, &tty)!=0)
	{
		pthread_mutex_lock(&printerLock);
		printf("Error %i from tcgetattr %s\n", errno, strerror(errno));
		pthread_mutex_unlock(&printerLock);
	}

	cfsetispeed(&tty, B9600);
	cfsetospeed(&tty, B9600);
	
	if (tcsetattr(sPort, TCSANOW, &tty) != 0) 
	{
		pthread_mutex_lock(&printerLock);
		printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
		pthread_mutex_unlock(&printerLock);
	}	

	while(1)
	{
		char buffer[100];
		ssize_t length = read(sPort, &buffer, sizeof(buffer));
		if (length == -1)
		{
			pthread_mutex_lock(&printerLock);
			printf("Error reading from serial port\n");
			pthread_mutex_unlock(&printerLock);
		}
		else if (length == 0 || length == 1)
		{
			continue;
		}
		else
		{
			buffer[length] = '\0';
			pthread_mutex_lock(&printerLock);
			printf("Serial data %s\n", buffer);
			pthread_mutex_unlock(&printerLock);
			pthread_mutex_lock(&FIFOlock);
			FIFOput(atoi(buffer));
			pthread_mutex_unlock(&FIFOlock);
		}
	}		
}

void *secondTask(void *args)
{
	int temp;
	char message[100];
	while(1)
	{
		pthread_mutex_lock(&FIFOlock);
		temp = FIFOget();
		pthread_mutex_unlock(&FIFOlock);
		if(temp != -1)
		{
			sprintf(message,"Element removed : %d\n",temp);
			pthread_mutex_lock(&printerLock);
			printerTask(message);
			pthread_mutex_unlock(&printerLock);
		}
		else
		{
			sprintf(message,"All elements done for now\n");
			pthread_mutex_lock(&printerLock);
			printerTask(message);
			pthread_mutex_unlock(&printerLock);
		}
		sleep(1);
				
	}
}

int main(void)
{
	FIFOinit();
	pthread_t taskOne;
	pthread_t taskTwo;
	if (pthread_mutex_init(&printerLock, NULL) != 0) 
	{ 
        printf("\n Printer lock mutex init has failed\n"); 
        return -1; 
    }
    
    if (pthread_mutex_init(&FIFOlock, NULL) != 0) 
	{ 
        printf("\n FIFO lock mutex init has failed\n"); 
        return -1; 
    }
	
	if (pthread_create(&taskOne, NULL, &firstTask, NULL) != 0) 
    {
        printf("Creating taskOne failed\n");
        return -1;
    }
    if (pthread_create(&taskTwo, NULL, &secondTask, NULL) != 0) 
    {
        printf("Creating taskTwo failed\n");
        return -1;
    }
    
	pthread_join(taskOne, NULL);
	pthread_join(taskTwo, NULL);
	
	exit(0);
}
     
