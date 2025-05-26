#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#define MAX_MEDICINE_NUMBER 6

struct timeval program_start_time;

pthread_mutex_t queue_access_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t medicine_access_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_full_mutex = PTHREAD_COND_INITIALIZER;
pthread_cond_t consult_done_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t medicine_box_full = PTHREAD_COND_INITIALIZER;

struct PatientsQueue {
    int patient_IDs[3];
    int last;
};

struct PatientsQueue patients_queue;
int medicines_number = 0;

int get_rand(int low, int max) {
    return rand() % (max - low + 1) + low;
}

double get_current_time() {
    struct timeval current_time;
    gettimeofday(&current_time, NULL);

    return current_time.tv_sec - program_start_time.tv_sec + (current_time.tv_usec - program_start_time.tv_usec) / 1e6;
}

void* patient_func(void* data) {
    int patient_id = *(int*)data;

    int wait_to_go = get_rand(2, 5);
    printf("%f - Pacjent(%d): Ide do szpitala, bede za %d s\n", get_current_time(), patient_id, wait_to_go);
    sleep(wait_to_go);

    pthread_mutex_lock(&queue_access_mutex);
    while (patients_queue.last == 3) {
        pthread_mutex_unlock(&queue_access_mutex);

        int walk_time = get_rand(2, 5);
        printf("%f - Pacjent(%d): za dużo pacjentów, wracam później za %d s\n", get_current_time(), patient_id, walk_time);
        sleep(walk_time);
        pthread_mutex_lock(&queue_access_mutex);
    }

    patients_queue.patient_IDs[patients_queue.last++] = patient_id;

    printf("%f - Pacjent(%d): czeka %d pacjentów na lekarza\n", get_current_time(), patient_id, patients_queue.last);

    if (patients_queue.last == 3) {
        printf("%f - Pacjent(%d): budzę lekarza\n", get_current_time(), patient_id);
        pthread_cond_signal(&queue_full_mutex);
    }

    pthread_cond_wait(&consult_done_cond, &queue_access_mutex);
    
    printf("%f - Pacjent(%d): kończę wizytę\n", get_current_time(), patient_id);

    pthread_mutex_unlock(&queue_access_mutex);
    return NULL;
} 


void* doctor_func(void* data) {
    while (1) {
        pthread_mutex_lock(&queue_access_mutex);
        while (patients_queue.last != 3 || medicines_number < 3) {
            pthread_cond_wait(&queue_full_mutex, &queue_access_mutex);
        }

        if (patients_queue.last == 3 && medicines_number >= 3) {
            printf("%f - Lekarz: budzę się\n", get_current_time());
    
            int consult_time = get_rand(2, 4);
            printf("%f - Lekarz: konsultuję pacjentów %d, %d, %d\n", get_current_time(), patients_queue.patient_IDs[0], 
            patients_queue.patient_IDs[1], patients_queue.patient_IDs[2]);
            pthread_mutex_unlock(&queue_access_mutex);
            sleep(consult_time);
            pthread_mutex_lock(&queue_access_mutex);
            patients_queue.last = 0;
        
            pthread_cond_broadcast(&consult_done_cond);
            pthread_mutex_unlock(&queue_access_mutex);
            printf("%f - Lekarz: zasypiam\n", get_current_time());
        } else {

        }
        
    }
}

void* pharmacist_fun(void* data) {
    int pharmacist_id = *(int*)data;

    int time_to_arrive = get_rand(5, 15);
    printf("%f - Farmaceuta(%d): ide do szpitala, bede za %d s\n", get_current_time(), pharmacist_id, time_to_arrive);
    sleep(time_to_arrive);

    pthread_mutex_lock(&medicine_access_mutex);
    while (medicines_number == MAX_MEDICINE_NUMBER) {
        pthread_cond_wait(&medicine_box_full, &medicine_access_mutex);
    }

    printf("%f - Farmaceuta(%d): budzę lekarza\n", get_current_time(), pharmacist_id);

    pthread_mutex_lock(&queue_access_mutex);
    pthread_cond_signal(&queue_full_mutex);


}


int main(int argc, char** argv) {
    srand(time(NULL));
    gettimeofday(&program_start_time, NULL);
    patients_queue.last = 0;

    int number_of_patients = atoi(argv[1]);
    int number_of_pharmacist = atoi(argv[2]);

    int* patient_ids = malloc(sizeof(int) * number_of_patients);
    int* pharmacist_ids = malloc(sizeof(int) * number_of_pharmacist);

    for (int i = 0; i < number_of_patients; i++) {
        patient_ids[i] = i;
    }
    for (int i = 0; i < number_of_pharmacist; i++) {
        pharmacist_ids[i] = i;
    }

    pthread_t doctor;
    pthread_create(&doctor, NULL, &doctor_func, NULL);

    pthread_t* patients_pharmacists = malloc(sizeof(pthread_t) * (number_of_patients + number_of_pharmacist));
    for (int i = 0; i < number_of_patients; i++) {
        pthread_create(&patients_pharmacists[i], NULL, &patient_func, &patient_ids[i]);
    }
    for (int i = number_of_patients; i < number_of_patients + number_of_pharmacist; i++) {
        pthread_create(&patients_pharmacists[i], NULL, &pharmacist_fun, &pharmacist_ids[i]);
    }

    for (int i = 0; i < number_of_patients + number_of_pharmacist; i++) {
        pthread_join(patients_pharmacists[i], NULL);
    }

    pthread_cancel(doctor);

    free(patients_pharmacists);
    free(pharmacist_ids);
    free(patient_ids);
    return 0;
}
