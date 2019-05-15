#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/types.h>
#include <fcntl.h>

#define Tmin 0 // Tiempo mínimo de espera tras adquirir recurso
#define Tmax 2 // Tiempo máximo de espera tras adquirir recurso
#define Nmax 4 // Recursos
#define Pmax 3 // Threads

/*En el grafo los Pmax primeros elementos de la matriz son los threads y los
* Nmax elementos siguientes son los recursos.
*/
int G[Nmax + Pmax][Nmax + Pmax], visited[Nmax + Pmax], fin;
int visitados = 0;
int filaModif; //Almaceno el último hilo que solicitó un recurso
int colModif; //Almaceno el último recurso solicitado por un hilo

void deshacerCiclo(int i, int j);
int hayCiclo();
int busquedaIguales(int *nodosVisitados);
void DFS(int i, int *nodosVisitados);

sem_t *comprobador;
pthread_mutex_t recurso[Nmax], accesoGrafo;

void * trabajo(void * tid) { // Código para cada thread
  int thid, i, j, k, ya, mis_recursos[Nmax];
  double x = 0;
  thid = (intptr_t) tid;
  srand(thid + time(NULL));
  comprobador = sem_open("Comprobador", O_CREAT, S_IRWXU, 0);

  for (i = 0; i < Nmax; i++) { // Adquiero nuevos recursos
    ya = 0;

    while (ya == 0) { // Selecciono un recurso que no tengo
      j = (int)(Nmax) * (rand() / (RAND_MAX + 1.0));
      ya = 1;
      for (k = 0; k < i; k++)
        if (mis_recursos[k] == j) ya = 0;
    }

    mis_recursos[i] = j; // Incluyo recurso en mi lista
    printf("Soy %d y quiero el recurso %d\n", thid, j);
    pthread_mutex_lock(&accesoGrafo);
    G[thid][j + Nmax - 1] = 1; //Se añade una arista desde el hilo thid hasta el recurso j
    filaModif = thid;
    colModif = j + Nmax - 1;
    sem_post(comprobador);
    pthread_mutex_unlock(&accesoGrafo);

    pthread_mutex_lock( & recurso[j]); // Adquiero el recurso
    printf(" Soy %d y tengo el recurso %d\n", thid, j);
    pthread_mutex_lock(&accesoGrafo);
    G[j + Nmax - 1][thid] = 1; //Se añade una arista desde el  recurso j hasta el hilo thid
    pthread_mutex_unlock(&accesoGrafo);

    for (k = 0; k < 10000; k++) x += sqrt(sqrt(k + 0.1)); // Trabajo intrascendente
    k = (int) Tmin + (Tmax - Tmin + 1) * (rand() / (RAND_MAX + 1.0)) + 1;
    sleep(k); // Espero un tiempo aleatorio
  }
  for (i = 0; i < Nmax; i++) pthread_mutex_unlock( & recurso[mis_recursos[i]]);
  printf("************ACABE! Soy %d\n", thid);
  pthread_exit(NULL);
}

void *hiloComprobador(void *tid){
  comprobador = sem_open("Comprobador", O_CREAT, S_IRWXU, 0);

  while(!fin){
    sem_wait(comprobador);
    pthread_mutex_lock(&accesoGrafo);
    if(hayCiclo())
      deshacerCiclo(filaModif, colModif);
    pthread_mutex_unlock(&accesoGrafo);
  }
  pthread_exit(NULL);
}

int main() {
  int i;
  pthread_t th[Pmax], fio;
  int tiempo_ini, tiempo_fin; // Para medir tiempo

  for(i=0; i< Nmax + Pmax; i++){
    for (int j = 0; j < Nmax + Pmax; j++) {
      G[i][j]=0;
    }
    visited[i]=0;
  }

  sem_unlink("Comprobador");

  fin = 0; //Variable para que el hilo comprobador sepa cuando terminar

  for (i = 0; i < Nmax; i++) pthread_mutex_destroy( & recurso[i]);
  pthread_mutex_destroy(&accesoGrafo);
  for (i = 0; i < Nmax; i++) pthread_mutex_init( & recurso[i], NULL);
  pthread_mutex_init(&accesoGrafo, NULL);

  tiempo_ini = time(NULL);
  pthread_create(&fio, NULL, hiloComprobador, (void *) (intptr_t) i);
  for (i = 0; i < Pmax; i++) pthread_create( & th[i], NULL, trabajo, (void * ) (intptr_t) i);
  for (i = 0; i < Pmax; i++) pthread_join(th[i], NULL);
  tiempo_fin = time(NULL);

  fin=1;

  pthread_join(fio, NULL);

  printf("Acabado en %d segundos \n", tiempo_fin - tiempo_ini);
  for (i = 0; i < Nmax; i++) pthread_mutex_destroy( & recurso[i]);

  return EXIT_SUCCESS;
}

void deshacerCiclo(int i, int j){
  printf("Libero el recurso %d \n", j);
  pthread_mutex_unlock(&recurso[j]);
  G[j][i] = 0;
}

int hayCiclo(){
  int *nodosVisitados;
  nodosVisitados = (int *)malloc((Nmax + Pmax)*sizeof(int));

  DFS(filaModif, nodosVisitados);
  printf("filaModif %d, nodosVisitados: ", filaModif);
  for (int i = 0; i < Nmax + Pmax; i++)
    printf("%d ", nodosVisitados[i]);
  printf("\n");

  if(busquedaIguales(nodosVisitados)){
    free(nodosVisitados);
    return 1;
  }
  free(nodosVisitados);
  return 0;
}

int busquedaIguales(int *nodosVisitados){
  for(int i=0; i < visitados; i++){
    for (int j = i+1; j < visitados; j++) {
        if(nodosVisitados[i] == nodosVisitados[j])
          return 1;
    }
  }
  return 0;
}

void DFS(int i, int *nodosVisitados){
  int j;
  visited[i]=1;
	for(j=0;j<Nmax + Pmax;j++){
    if(G[i][j]==1){
      nodosVisitados[visitados] = j; //Almaceno los nodos a los que se puede ir desde i
      visitados++; //Mantengo un registro de cuantos nodos he podido visitar
      if(!visited[j])
        DFS(j, nodosVisitados);
    }
  }
}