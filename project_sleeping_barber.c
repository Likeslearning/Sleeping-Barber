#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define CUSTOMER_LIMIT 10       // Customer threads limit
#define CUT_DURATION 1          // Haircut duration

// Creating semaphores
sem_t barbers;                  // Barber semaphore
sem_t customers;                // Customer semaphore
sem_t mutex;                    // Mutex semaphore

// Function prototypes
void barber_function(void* number);
void customer_function(void* number);
void to_wait();

// Variables
int barber_chair = 0;           // Barber chair variable
int customer_number = 0;        // Customer variable
int waiting_chair = 0;          // Chairs in the waiting room variable
int empty_chair = 0;            // Empty chairs in the waiting room variable
int customer_serve = 0;         // Customer identity variable
int sit_chair = 0;              // Customer's chair's identity variable
int* chair;                     // For identity trade between barber - customer

int main(int argc, char** args) { // All code works after entering the command line arguments
    printf("Enter Your Customer Number: ");
    scanf("%d", &customer_number);
    printf("Enter Your Waiting Chair Number: ");
    scanf("%d", &waiting_chair);
    printf("Enter Your Barber Number: ");
    scanf("%d", &barber_chair);

    empty_chair = waiting_chair;
    chair = (int*) malloc(sizeof(int) * waiting_chair);

    // Program is ended if the entered customer number is bigger than the customer threads limit
    if (customer_number > CUSTOMER_LIMIT) {
        printf("\nDaily customer limit is %d!\n\n", CUSTOMER_LIMIT);
        return EXIT_FAILURE;
    }

    printf("\nEntered Customer Number: %d", customer_number);
    printf("\nEntered Waiting Chair(s) Number: %d", waiting_chair);
    printf("\nEntered Barber Chair(s) Number: %d", barber_chair);

    // Creating threads
    pthread_t barber[barber_chair], customer[customer_number];

    // Initializing semaphores
    sem_init(&barbers, 0, 0);
    sem_init(&customers, 0, 0);
    sem_init(&mutex, 0, 1);

    printf("\nBarbershop has been opened.\n\n");

    // Defining barber threads
    for (int i = 0; i < barber_chair; i++) {
        pthread_create(&barber[i], NULL, (void*)barber_function, (void*)&i);
        to_wait();
    }

    // Defining customer threads
    for (int i = 0; i < customer_number; i++) {
        pthread_create(&customer[i], NULL, (void*)customer_function, (void*)&i);
        to_wait();
    }

    // To serve all available customers before the barbershop goes off
    for (int i = 0; i < customer_number; i++) {
        pthread_join(customer[i], NULL);
    }

    printf("\nServices are done. Barbershop went off for today. Barber(s) has/have left the barbershop...\n\n");

    return EXIT_SUCCESS;
}

void barber_function(void* number) {
    int c = *(int*)number + 1;
    int next_customer, customer_id;

    printf("[Barber: %d]\thas came to the barbershop.\n", c);

    while (1) {
        if (!customer_id)
        {
            printf("[Barber: %d]\twent to the sleep.\n\n", c);
        }

        sem_wait(&barbers);     // Join the tail of sleeping barbers
        sem_wait(&mutex);       // Lock the access to preserve the chair

        // Selecting the customer that will be served from the waiting customers
        customer_serve = (++customer_serve) % waiting_chair;
        next_customer = customer_serve;
        customer_id = chair[next_customer];
        chair[next_customer] = pthread_self();

        sem_post(&mutex);       // Removing the access lock of the chair
        sem_post(&customers);   // Serving the determined customer

        printf("[Barber: %d]\tis awake and %d. customer is getting the haircut.\n\n", c, customer_id);
        sleep(CUT_DURATION);
        printf("[Barber: %d]\t%d. customer's haircut is over.\n\n", c, customer_id);
    }
}

void customer_function(void* number) {
    int c = *(int*)number + 1;
    int chair_sit, barber_id;

    sem_wait(&mutex);   // Lock the access to preserve the chair

    printf("[Customer: %d]\thas came to the barbershop.\n", c);

    // If there is an empty chair in the waiting room
    if (empty_chair > 0) {
        empty_chair--;
        printf("[Customer: %d]\tcame to the waiting room.\n\n", c);

        // Choose a chair from the waiting room and sit
        sit_chair = (++sit_chair) % waiting_chair;
        chair_sit = sit_chair;
        chair[chair_sit] = c;

        sem_post(&mutex);           // Removing the access lock of the chair
        sem_post(&barbers);         // Waking up the proper barber

        sem_wait(&customers);       // Join the tail of sleeping barbers
        sem_wait(&mutex);           // Lock the access to preserve the chair

        // Passing to the barber chair
        barber_id = chair[chair_sit];
        empty_chair++;

        sem_post(&mutex);           // Removing the access lock of the chair
    }

    else {
        sem_post(&mutex);           // Removing the access lock of the chair
        printf("[Customer: %d]\tcouldn't find a chair to wait. Leaving the barbershop...\n\n", c);
    }

    pthread_exit(0);
}

void to_wait() { // A helper method for sleep operation.
    srand((unsigned int)time(NULL));
    usleep(rand() % (250000 - 50000 + 1) + 50000); // A value of ms in the interval "50000 - 250000"
}
