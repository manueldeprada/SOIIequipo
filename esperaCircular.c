#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <pthread.h>
#include <unistd.h>

#define Tmin 0 // Tiempo mínimo de espera tras adquirir recurso
#define Tmax 2 // Tiempo máximo de espera tras adquirir recurso
#define Nmax 7// Recursos
#define Pmax 5 // Threads

pthread_mutex_t recurso[Nmax];

void * trabajo(void * tid) { // Código para cada thread
  int thid, Ri;
  thid = (intptr_t) tid;
  srand(thid + time(NULL));
  Ri = (int)(Nmax) * (rand() / (RAND_MAX + 1.0)); //Numero de recursos aleatorios <=N
  int j, k,mi_recurso=-1,rec=0;//inicializamos el recurso a -1 para provocar que se pueda asignar el primer recurso (0) al primer hilo que entra en la función trabajo
  double x = 0;

  for (int i=0;i<Ri;i++){
  	j=rec;//Asigno el valor
	rec++;
  	if (mi_recurso < j) mi_recurso = j; //Cambio el recurso del hilo en caso de que este nuevo (j) sea mayor que el que ya tenía.
    printf("Soy %d y quiero el recurso %d\n", thid, j);
    pthread_mutex_lock(& recurso[j]); // Adquiero el recurso
    printf(" Soy %d y tengo el recurso %d\n", thid, j);
    for (k = 0; k < 10000; k++) x += sqrt(sqrt(k + 0.1)); // Trabajo intrascendente
    k = (int) Tmin + (Tmax - Tmin + 1) * (rand() / (RAND_MAX + 1.0)) + 1;
    sleep(k); // Espero un tiempo aleatorio
    pthread_mutex_unlock( & recurso[j]); //Suelto el recurso
  }
  printf("************ACABE! Soy %d\n", thid);
  pthread_exit(NULL);
}
int main() {
  int i;
  pthread_t th[Pmax];
  int tiempo_ini, tiempo_fin; // Para medir tiempo
  for (i = 0; i < Nmax; i++) pthread_mutex_destroy( & recurso[i]);
  for (i = 0; i < Nmax; i++) pthread_mutex_init( & recurso[i], NULL);
  tiempo_ini = time(NULL);
  for (i = 0; i < Pmax; i++) pthread_create( & th[i], NULL, trabajo, (void * ) (intptr_t) i);
  for (i = 0; i < Pmax; i++) pthread_join(th[i], NULL);
  tiempo_fin = time(NULL);
  printf("Acabado en %d segundos \n", tiempo_fin - tiempo_ini);
  for (i = 0; i < Nmax; i++) pthread_mutex_destroy( & recurso[i]);
  return EXIT_SUCCESS;
}
