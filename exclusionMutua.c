#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <pthread.h>
#include <unistd.h>
#include "cola.h"

#define Tmin 0 // Tiempo mínimo de espera tras adquirir recurso
#define Tmax 2 // Tiempo máximo de espera tras adquirir recurso
#define Nmax 10 // Recursos
#define Pmax 12 // Threads


pthread_mutex_t recurso[Nmax];

cola buffer[Nmax];
int final=0; //Variable de condición para cerrar los demonios
int mensaje[Nmax];


void * trabajo(void * tid) { // Código para cada thread
  int thid, Ri;
  thid = (intptr_t) tid;
  srand(thid + time(NULL));
  Ri = (int)(Nmax) * (rand() / (RAND_MAX + 1.0)); //Numero de recursos aleatorios <=N
  int i, j, k, ya, mis_recursos[Ri];
  double x = 0;

  for (i = 0; i < Ri; i++) { // Adquiero nuevos recursos
    ya = 0;
    while (ya == 0) { // Selecciono un recurso que no tengo
      j = (int)(Nmax) * (rand() / (RAND_MAX + 1.0));
      ya = 1;
      for (k = 0; k < i; k++)
        if (mis_recursos[k] == j) ya = 0;
    }

    printf("Soy %d y quiero el recurso %d\n", thid, j);

    pthread_mutex_lock(&recurso[j]); // Adquiero el recurso
    while(insertarCola(&buffer[j],thid)==-1){ //Intento insertarlo
      sleep(3); //Si no lo consigo espero y vuelvo a intentarlo
    }
    pthread_mutex_unlock( &recurso[j]);

    mis_recursos[i] = j; // Incluyo recurso en mi lista
    for (k = 0; k < 10000; k++) x += sqrt(sqrt(k + 0.1)); // Trabajo intrascendente

    k = (int) Tmin + (Tmax - Tmin + 1) * (rand() / (RAND_MAX + 1.0)) + 1;
    sleep(k); // Espero un tiempo aleatorio

  }
  printf("************ACABE! Soy %d\n", thid);
  final++;
  pthread_exit(NULL);
}

void *trabajoDemonio (void * tid){
  //Cada demonio tiene que tener una cola de solicitud

  int thid=(intptr_t)tid;
  int solicitud;

  while(final!=Pmax || !esVaciaCola(buffer[thid])){ //Realizamos tareas con la cola hasta que acaben todos los hilos y se hayan evaluado todas las solicitudes
      while (esVaciaCola(buffer[thid]) && final!=Pmax){ //Incluimos la segunda condición debido a posibles carreras críticas
        //Espera mientras que no haya ninguna solicitud
      }

      pthread_mutex_lock( &recurso[thid]);
      //Cuando se reciba una solicitud
      if(!esVaciaCola(buffer[thid])){
        solicitud=primero(buffer[thid]);
        suprimirCola(&buffer[thid]);
          printf("Soy el demonio del recurso %d. Gestiono solicitud de %d\n",thid, solicitud);
      }
      pthread_mutex_unlock( &recurso[thid]);

  }
  pthread_exit(NULL);
}


int main() {
  int i;
  pthread_t th[Pmax],thDemonio[Nmax];
  int tiempo_ini, tiempo_fin; // Para medir tiempo

  //Creamos las colas de solicitudes
  for(int i=0;i<Nmax;i++){
    creaCola(&buffer[i]);
  }

  //Inicializamos los mutex del recurso
  for (i = 0; i < Nmax; i++) pthread_mutex_destroy( &recurso[i]);
  for (i = 0; i < Nmax; i++) pthread_mutex_init( &recurso[i], NULL);

  //Creamos Nmax demonios para cada recurso

  for (i = 0; i < Nmax; i++) pthread_create( &thDemonio[i], NULL, &trabajoDemonio, (void * ) (intptr_t) i);


  tiempo_ini = time(NULL);
  for (i = 0; i < Pmax; i++) pthread_create( &th[i], NULL, &trabajo, (void * ) (intptr_t) i);
  for (i = 0; i < Pmax; i++) pthread_join(th[i], NULL);
  tiempo_fin = time(NULL);
  printf("Acabado en %d segundos \n", tiempo_fin - tiempo_ini);
  for (i = 0; i < Nmax; i++){ //Destruimos los mutex y las colas
    pthread_mutex_destroy( &recurso[i]);
    destruyeCola(&buffer[i]);
  }
  return EXIT_SUCCESS;
}
