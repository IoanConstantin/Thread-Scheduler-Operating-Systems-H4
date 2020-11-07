#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include "so_scheduler.h"

#define THREADS_MAX 10000

/* Structura care reprezinta fiecare thread */
struct element {
	unsigned int prioritate;
	pthread_t thread_id;
	sem_t semafor;
	int pozitie;
	so_handler *handler;
	unsigned int cuanta;
	int stare;
	int stop_thread;
};

/* Un element al acestei structuri este un array care */
/* contine toate elementele cu aceeasi prioritate */
struct prio{
	void **sir;
	int dim;
}prioritati[6];

int init_counter = 0, dim_threads_array = 0, pozitii[6];
unsigned int max_cuanta, io_counter;

struct prio *io_array;

struct element *array, aux;

/* Functia care realizeaza initializarile si deschide semaforul */
/* thread-ului daca acesta este primul thread */
void initializari(unsigned int prior, so_handler *handl, pthread_t thread_id)
{
	array[dim_threads_array].prioritate = prior;
	array[dim_threads_array].handler = handl;
	array[dim_threads_array].thread_id = thread_id;
	array[dim_threads_array].stare = 0;
	sem_init(&array[dim_threads_array].semafor, 0, 0);
	int dim = prioritati[prior].dim;

	prioritati[prior].sir[dim] = (void *) dim_threads_array;
	prioritati[prior].dim++;
	if (dim_threads_array == 0) {
		aux.cuanta = max_cuanta;
		aux.pozitie = 0;
		sem_post(&array[dim_threads_array].semafor);
	}
	dim_threads_array++;
}

/* Initializari si alocari dinamice */
int so_init(unsigned int time_quantum, unsigned int io)
{
	if (time_quantum <= 0 || io > SO_MAX_NUM_EVENTS || init_counter == 1)
		return -1;

	init_counter = 1;

	io_counter = io;
	max_cuanta = time_quantum;

	array = malloc(THREADS_MAX * sizeof(struct element));

	io_array = malloc(io * sizeof(struct prio));

	for (int i = 0; i < io; i++)
		io_array[i].sir = malloc(1000 * sizeof(void *));

	for (int i = 0; i < 6; i++)
		prioritati[i].sir = malloc(1000 * sizeof(void *));

	return 0;
}

/* Aplicam intai Round Robin pentru prioritatea maxima si prioritatea */
/* curenta apoi verificam starea thread-ului si aplicam din nou */
/* Round Robin pentru prioritatea minima si prioritatea curenta */
void scheduler(void)
{
	struct element auxy = array[aux.pozitie];

	for (int i = SO_MAX_PRIO; i > auxy.prioritate; i--) {
		for (int j = pozitii[i]; j < prioritati[i].dim; j++) {
			int pozitia = (int) prioritati[i].sir[j];

			if (array[pozitia].stare == 0) {
				int old_aux_poz = aux.pozitie;

				aux.cuanta = max_cuanta;
				pozitii[array[pozitia].prioritate] = j;

				if (pozitia != aux.pozitie) {
					aux.pozitie = pozitia;

					sem_post(&array[pozitia].semafor);
if (array[old_aux_poz].stop_thread == 0)
	sem_wait(&array[old_aux_poz].semafor);
				}
				return;
			}
		}
	}

if (auxy.stare == 1 || aux.cuanta == 0) {
	if (pozitii[auxy.prioritate] < prioritati[auxy.prioritate].dim - 1)
		pozitii[auxy.prioritate]++;
	else
		pozitii[auxy.prioritate] = 0;

	for (int i = auxy.prioritate; i >= 0; i--) {
		for (int j = pozitii[i]; j < prioritati[i].dim; j++) {
			int pozitia = (int) prioritati[i].sir[j];

			if (array[pozitia].stare == 0) {
				int old_aux_poz = aux.pozitie;

				aux.cuanta = max_cuanta;
				pozitii[array[pozitia].prioritate] = j;

				if (pozitia != aux.pozitie) {
					aux.pozitie = pozitia;

					sem_post(&array[pozitia].semafor);

if (array[old_aux_poz].stop_thread == 0)
	sem_wait(&array[old_aux_poz].semafor);
				}
				return;
			}
		}
	}
}
}

/* Decrementam cuanta thread-ului curent si apelam */
/* functia de planificare */
DECL_PREFIX void so_exec(void)
{
	aux.cuanta--;
	aux.stop_thread = 0;
	scheduler();
}

/* Rutina unui thread */
void *start_thread(void *arg)
{
	sem_wait(&array[(int) arg].semafor);
	array[(int) arg].handler(array[(int) arg].prioritate);
	array[(int) arg].stop_thread = 1;
	array[(int) arg].stare = 1;
	scheduler();

	return NULL;
}

/* Functia care creeaza un thread */
tid_t so_fork(so_handler *func, unsigned int priority)
{
	if (func == 0 || priority > SO_MAX_PRIO)
		return INVALID_TID;

	pthread_t thread_id;

	pthread_create(&thread_id, NULL, start_thread,
		(void *)dim_threads_array);

	initializari(priority, func, thread_id);

	if (dim_threads_array != 1)
		so_exec();

	return thread_id;
}

/* Functia care trimite in asteptare thread-urile de pe un I/O */
DECL_PREFIX int so_wait(unsigned int io)
{
	if (io >= io_counter)
		return -1;

	array[aux.pozitie].stare = 1;

	io_array[io].sir[io_array[io].dim] = (void *) aux.pozitie;
	io_array[io].dim++;

	array[aux.pozitie].stop_thread = 0;
	scheduler();

	return 0;
}

/* Functia care trimite thread-urile de pe un I/O din nou in rulare */
DECL_PREFIX int so_signal(unsigned int io)
{
	if (io >= io_counter)
		return -1;

	aux.cuanta--;

	int dim = io_array[io].dim;

	for (int i = dim - 1; i >= 0; i--) {
		io_array[io].dim--;
		array[(int) io_array[io].sir[i]].stare = 0;
		for (int j = i; j < io_array[io].dim; j++)
			io_array[io].sir[j] = io_array[io].sir[j + 1];
	}

	aux.stop_thread = 0;
	scheduler();

	return dim;
}

/* Functia in care se elibereaza memoria alocata */
DECL_PREFIX void so_end(void)
{
	init_counter = 0;

	for (int i = 0; i < dim_threads_array; i++)
		pthread_join(array[i].thread_id, NULL);

	for (int i = 0; i < 6; i++)
		free(prioritati[i].sir);

	for (int i = 0; i < io_counter; i++)
		free(io_array[i].sir);
	free(io_array);

	free(array);
}
