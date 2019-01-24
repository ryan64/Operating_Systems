#include <iostream>
#include <cstdlib>
#include <pthread.h>
#include <unistd.h>

using namespace std;

#define NUM_THREADS     64

static int maxweight;
static int bridgeLoad = 0;
static pthread_mutex_t mutex;
static pthread_cond_t empty = PTHREAD_COND_INITIALIZER;
    
struct vehicle {
    string vID;
    int arrive_delay;
    int weight;
    int cross_time;
}; 

void enterBridge(vehicle *threadarg){
    while ((bridgeLoad + threadarg->weight) > maxweight) {
        pthread_cond_wait(&empty, &mutex);
    }
    bridgeLoad += threadarg->weight;
    cout << "Vehicle " << threadarg->vID << " goes on bridge." << endl; 
    cout << "The current bridge load is " << bridgeLoad << " tons." << endl;

    if (bridgeLoad < maxweight){
    pthread_cond_signal(&empty); 
    }
}

void leaveBridge(vehicle * threadarg){
    bridgeLoad = bridgeLoad - threadarg->weight;
    pthread_cond_signal(&empty);
    cout << "Vehicle " << threadarg->vID << " leaves the bridge." << endl;
   cout << "The current bridge load is " << bridgeLoad << " tons." << endl;
}

void *thread_Run(void *threadarg) {

   struct vehicle *my_data;

   my_data = (struct vehicle *) threadarg;
   pthread_mutex_lock(&mutex);

   cout << "Vehicle " << my_data->vID << " arrives at bridge." << endl;
   cout << "The current bridge load is " << bridgeLoad << " tons." << endl;

   if (my_data->weight > maxweight){
      cout << "Vehicle " << my_data->vID << " exceeds maximum bridge load." << endl; 
      pthread_mutex_unlock(&mutex); 
      pthread_exit(NULL); 
   }

   enterBridge(my_data);
   pthread_mutex_unlock(&mutex);
   sleep(my_data->cross_time);
   pthread_mutex_lock(&mutex);
   leaveBridge(my_data);
   pthread_mutex_unlock(&mutex);

   pthread_exit(NULL);
}

int main (int argc, char *argv[]) {
  if (argc == 2){
    maxweight = atoi(argv[1]);
  }
  else {
    cout << "Please provide maxweight" << endl; 
    exit(-1); 
  }
   cout << "Maximum bridge load is " << maxweight << " tons." << endl;
   pthread_mutex_init(&mutex, NULL);

   pthread_t threads[NUM_THREADS];
   struct vehicle td[NUM_THREADS];
	
   int rc;
   int nVehicles = 0; 

   while(cin >> td[nVehicles].vID >> td[nVehicles].arrive_delay >> td[nVehicles].weight >> td[nVehicles].cross_time){
      //cout << td[nVehicles].vID << " " << td[nVehicles].arrive_delay << " " << td[nVehicles].weight << " " << td[nVehicles].cross_time << endl;
      sleep(td[nVehicles].arrive_delay);
      rc = pthread_create(&threads[nVehicles], NULL, thread_Run, (void *)&td[nVehicles]);
    
      nVehicles++; 

      if (rc){
         cout << "Error:unable to create thread," << rc << endl;
         exit(-1);
      }
   }

    /*td[0].vID = "HIOFCR";
    td[0].arrive_delay = 0;
    td[0].weight = 1;
    td[0].cross_time = 1;

    td[1].vID = "STOL3N";
    td[1].arrive_delay = 3;
    td[1].weight = 10;
    td[1].cross_time = 2;

    td[2].vID = "SHKSPR";
    td[2].arrive_delay = 8;
    td[2].weight = 2;
    td[2].cross_time = 3;*/

    int i; 
    for( i=0; i < nVehicles; i++ ){
            pthread_join(threads[i], NULL);
    }

    cout << endl;
    cout << "Total number of vehicles: " << nVehicles << endl; 
    return 0; 
   //pthread_exit(NULL);
}