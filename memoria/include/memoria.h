#ifndef MEMORIA_H_
#define MEMORIA_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <pthread.h>
#include "comunicacion.h"
#include "log_config.h"
#include "sockets.h"
#include "manejodetabla.h"
#include "swap.h"
#include <math.h>
#include "inttypes.h"


// STRUCTS/ENUMS

 typedef struct  // archivo de configuracion memoria
 {
 	char* ip_memoria;
 	char* puerto_escucha;
 	int tam_memoria;
 	int tam_pagina;
 	int entradas_por_tabla;
 	int retardo_memoria;
 	char* algoritmo_reemplazo;
 	int marcos_por_proceso;
 	int retardo_swap;
 	char* path_swap;
 } arch_config;

 typedef struct  // archivo de configuracion memoria
  {
  	uint32_t contadorPF;
  	uint32_t contadorAccesoSwap;

  } contador_por_pid;

t_list* lista_tabla_de_pagina_1_nivel;
t_log* memoria_logger ;

arch_config config_valores_memoria;

//VARIABLES
int socket_memoria;
void* memoria_usuario;
int cantidad_de_marcos;
const char* pathSwap;
algoritmo algoritmo_memoria;
t_list* marcos;
t_list* contador_pid;


//SEMAFOROS

pthread_mutex_t mutex_comparador_pid;
pthread_mutex_t mutex_comparador_indice;
pthread_mutex_t mutex_comparador;
pthread_mutex_t mutex_marcos;
pthread_mutex_t mutex_memoria_usuario;
pthread_mutex_t mutex_lista_archivo;
pthread_mutex_t mutex_comparador_archivo_pid;
pthread_mutex_t mutex_archivo_swap;
pthread_mutex_t mutex_numero_tabla_2p;
pthread_mutex_t mutex_tabla_pagina_primer_nivel;
pthread_mutex_t mutex_tabla_pagina_segundo_nivel;
pthread_mutex_t mutex_contador_pid;

//FUNCIONES

void cargar_configuracion();
t_paquete* preparar_paquete_para_handshake();
void manejo_instrucciones(t_list*,int);
void traducir_operandos(void*,uint32_t*,uint32_t*);
void inicializar_memoria();
void manejo_conexiones(int);
int atender_clientes_memoria(int);
void suspender_proceso(int);


void liberarMarco(uint32_t);
void liberarPag(uint32_t );
void liberarTodosLosMarcos(uint32_t);
int ocuparMarcoLibre(uint32_t );
bool estaLibre(marquito* );
bool igualPid(marquito*);
int cantidadUsadaMarcos(uint32_t );
void inicializar_marcos();
double marcosTotales();
t_list* marcosPid(uint32_t );

int pags_proceso(uint32_t ,int );
int tp2_proceso(int ,int );

uint32_t leer_de_memoria(uint32_t);
int escribirEn(uint32_t , uint32_t);
uint32_t escribirModificaciones(uint32_t ,uint32_t );
void escribirPagEnMemoria(void* ,uint32_t);
int get_marco(int);
op_code codigoEscritura(int);

void cambiarPdePagina(uint32_t,uint32_t ,bool );
void cambiarUdePagina(uint32_t ,uint32_t ,bool );
void cambiarMdePagina(uint32_t ,uint32_t ,bool);
void cambiarPunterodePagina(uint32_t ,uint32_t ,bool );
void cambiarMarcoModificadoAUno(uint32_t );
void cambiarMarcoUsoAUno(uint32_t );
void liberarMemoriaUsuario(uint32_t );
void limpiarMarco(uint32_t);
void finalizar_memoria();
void sighandler(int);




#endif
