#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <pthread.h>
#include <unistd.h>

#define Tmin 0 // Tiempo mínimo de espera tras adquirir recurso
#define Tmax 2 // Tiempo máximo de espera tras adquirir recurso
#define Nmax 5

 // Recursos
#define Pmax 2 // Threads

pthread_mutex_t recurso[Nmax];

void * trabajo(void * tid) { // Código para cada thread
  int thid, i, j, k, ya;
  double x = 0;
  thid = (intptr_t) tid;
  srand(thid + time(NULL));
  int Ri= (int)(Nmax) * (rand() / (RAND_MAX + 1.0)) + 1;
  int mis_recursos[Ri];
  for (i = 0; i < Ri; i++) { // Adquiero nuevos recursos
    ya = 0;
    while (ya == 0) { // Selecciono un recurso que no tengo
      j = (int)(Nmax) * (rand() / (RAND_MAX + 1.0));
      ya = 1;
      for (k = 0; k < i; k++)
        if (mis_recursos[k] == j) ya = 0;
    }
    mis_recursos[i] = j; // Incluyo recurso en mi lista
    printf("Soy %d y quiero el recurso %d\n", thid, j);
    int r = pthread_mutex_trylock( & recurso[j]); // Intento adquirir el recurso
    if (r!=0){ //si no he podido conseguirlo
      printf("No he podido obtener el recurso %d\n", j);
      for (int ii = 0; ii < i; ii++) { //libero todos los recursos que tenia
        pthread_mutex_unlock( & recurso[mis_recursos[ii]]);
        printf("\tLiberado el recurso %d\n", mis_recursos[ii]);
      }
      sleep(rand()%5);// espero un tiempo aleatorio para que otros procesos acaben
      i=-1; //vuelvo con la i para que pase a 0 y vuelva empezar
      continue; //vuelvo al bucle
    }// si he conseguido el recurso, vuelvo al estado habitual
    printf(" Soy %d y tengo el recurso %d\n", thid, j);
    for (k = 0; k < 10000; k++) x += sqrt(sqrt(k + 0.1)); // Trabajo intrascendente
    k = (int) Tmin + (Tmax - Tmin + 1) * (rand() / (RAND_MAX + 1.0)) + 1;
    sleep(k); // Espero un tiempo aleatorio
  }
  for (i = 0; i < Ri; i++) pthread_mutex_unlock( & recurso[mis_recursos[i]]);
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
