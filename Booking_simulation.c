#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

//Global variables.
static sem_t lock;
static sem_t mutex;

//Necessary semaphors.
static sem_t connected_lock;
static sem_t operators;

//Maximum limits of resources.
static int NUM_OPERATORS=4;
static int NUM_LINES=8;
static int connected=0;

//global variables
static int next_id = 0;

// enum {FALSE, TRUE};
/*
 
 Enter critical section
 Checks if the call can be connected.
 If there is a free line : update connected 
 else : print "busy" and exit critical section.
 
*/
int check(int id){
    // Check if the lines are all busy 

    sem_wait(&connected_lock);
    if(connected == NUM_LINES){
            
        // Exit critical setion, the lines are busy 
        sem_post(&connected_lock);
        return 1;
            
    }
        
    // Update shared source.
    // There is a line available 
    connected++;
        
    // Exit critical section 
    sem_post(&connected_lock);
    
    return 0;
}


//Threads calling function 
void *phonecall(void *arg) {
    
    //ID of the thread- local variable 
    int id;
    
    //Update the ID of the thread 
    // Prevents the intervention of other threads 
    sem_wait(&lock) ;
        next_id++;
        id = next_id;
    sem_post(&lock) ;
    
    //Call has made.
    printf("[ID: %d] is attempting to connect...\n",id);
    
    // loops till the call is free.
    // Until a line gets free, Try again. 
    while(check(id)) {
        printf("[ID: %d] All the lines are busy. Please try again later\n",id);
    }
      
    // A line is free. 
    // when a connection has been made
        
    printf("[ID: %d] There is a line available. Call ringing. \n",id);
        
    //Checks for free opeartors 
    sem_wait(&operators) ;
        
    //There is  free opeartor 
    printf("[ID: %d] Medicine order is now being taken care of. It may take some time.\n",id);
    sleep(3);
    //Prepares Order
    
    printf("[ID: %d] has made an appointment for the test.\n",id);
    printf("[ID: %d] Order is now complete\n",id);
    //Order Complete.
    
    sem_wait(&mutex);
    //Update semaphor.
    sem_post(&operators);
    
    //Update shared source.
    // Exclusion is acheived through mutex
    connected--;
    sem_post(&mutex);
    
}

int main()
{
    int number_calls, i;
    char c[10];
    
    
    //loop takes care of unnecessary input or no input 
    // In case of questionable input, 
    // Inputs gets taken again 
    
    loop: 
       
        number_calls = 0;
        // Get input value for number of calls 
        printf("Enter number of calls: ");
        scanf(" %s",c);
        
        int l = strlen(c);
        
        for(i=0; i<l; i++) {
        
            // If the input string has a char
            // Take input again. 
            if(!isdigit(c[i])) {
                goto loop;
            }
            
            //update number of calls 
            else{
                number_calls = number_calls*10 + c[i]-'0';
            }
        
        }

    //Initialise all counting and binary semaphors
   
    //Binary semaphores 
    sem_init(&mutex,0,1);
    sem_init(&lock,0,1);
    sem_init(&connected_lock,0,1) ;
    
    // Counting semaphore
    sem_init(&operators,0,NUM_OPERATORS) ;
    int valid;
    
    //Create threads
    pthread_t pid[number_calls];
    for(i=0; i<number_calls; i++){
    
        // valid checks for any error in creation 
        valid = pthread_create(&pid[i], NULL, phonecall, NULL) ;
        
        // Error handling 
        if(valid !=0) {
            exit(0);
        }
        
    }
    
    //join threads so the main function DOESN'T end before threads
    for(i=0; i<number_calls; i++){
    
        //Handles error in join. 
        valid = pthread_join(pid[i], NULL) ;
       
        // Error handling 
        if(valid != 0) {
            exit(0);
        }
        
    }
    
    //Destroy the semaphors
    sem_destroy(&mutex);
    sem_destroy(&lock);
    sem_destroy(&connected_lock);
    sem_destroy(&operators);
    
    //
    return 0;
}