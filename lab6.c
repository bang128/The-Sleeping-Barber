#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

int n, num_customers, seating_customers = 0, count = 0;
pthread_mutex_t mutex;
pthread_cond_t cond;

/* This function is to handle barber's job */
void* Barber(void* t) {
  // while loop to allow 1 barber to handle several customers
  while (1) {
    pthread_mutex_lock(&mutex);       // lock the mutex
    if (seating_customers == 0) {         // if there is no customers, barber is waiting/sleeping
      pthread_cond_wait(&cond, &mutex);   // wait for signal from customer
    }
    pthread_mutex_unlock(&mutex);     // lock the mutex

    printf("Barber is working\n\n");
    sleep(3);                         // set up 3 second for barber to handle 1 customer

    // lock and unlock mutex to decrease seating_customers
    pthread_mutex_lock(&mutex);
    seating_customers -= 1;
    pthread_mutex_unlock(&mutex);

    printf("Barber have been done with 1 customer\n");
    printf("Number of customer left: %d\n\n", seating_customers);


    count += 1;
    // if count reaches num_customers, it means that barber does nothing anymore
    if (count == num_customers) break;

  }
  return NULL;
}

/* This function is to handle customer's coming/leaving */
void* Customer(void* t) {
  // Get customers' id (pthread id)
  long my_id = (long)t;
  printf("Customer %ld comes\n", my_id);

  // if no empty seat, new customer leaves
  if (seating_customers >= n) {
    printf("Customer %ld leaves\n\n", my_id);
    // increase count no matter whether customer seats or leaves
    // to ensure barber's while loop ends after num_customers
    count += 1;
    return NULL;
  }
  else {
    // if no custmer yet, new customer wakes barber up
    if (seating_customers == 0) {
      printf("Customer %ld wakes Barber up\n", my_id);
      pthread_cond_signal(&cond);   // signal barber
    }

    // lock and unlock mutex to increase seating_customers
    pthread_mutex_lock(&mutex);
    seating_customers += 1;
    pthread_mutex_unlock(&mutex);

    printf("Customer %ld occupies 1 seat\n", my_id);
    printf("Number of current customers: %d\n\n", seating_customers);

  }
  return NULL;
}

int main(int argc, char *argv[])
{
  /* This program prompts user to input the number of seats
     and the number of customers:
        ./lab6 <number of customers> <number of seats>
     So first, it will check whether user's input is in correct form
  */
  if (argc != 3) {
    printf("Input must be in form of ./lab6 <number of customers> <number of seats>\n");
    return -1;
  }

  num_customers = atoi(argv[1]);   // assign num_customers with argv[1]
  n = atoi(argv[2]);               // assign n (number of seats) with argv[2]

  // Check to ensure num_customers and n are positive numbers.
  if (num_customers <= 0 || n <= 0) {
    printf("Number of customers and number of seats must be greater than 0\n");
    return -1;
  }

  printf("Maximum number of customers: %d\n", num_customers);
  printf("Number of seats: %d\n\n", n);

  // declare pthread variables to handle barber and customers
  pthread_t barber, customers[num_customers];

  pthread_mutex_init(&mutex, NULL);       // initialize mutex
  pthread_cond_init (&cond, NULL);        // initialize cond


  pthread_create(&barber, NULL, Barber, NULL);  // create barber's thread


  long i = 0;
  // while loop to create num_customers customers
  while (i < num_customers) {

    // use random number to decide whether a customer arrives
    srand(time(NULL));
    int r = rand();

    // if randomly-generated number is even, a customer arrives
    if (r%2 == 0) {
      //create thread for each new customer
      pthread_create(&customers[i], NULL, Customer, (void *)(i+1));
      i += 1;
    }

    sleep(1);   // slow down the process
  }

  // wait for barber thread and all customer threads to complete
  pthread_join(barber, NULL);
  for (i=0; i < num_customers; i++)
    pthread_join(customers[i], NULL);


  // clean up and exit
  pthread_mutex_destroy(&mutex);
  pthread_cond_destroy(&cond);
  pthread_exit (NULL);

  return 0;
}
