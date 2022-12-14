#include "planificacionUtils.h"

//--------------------------------------------------- FUNCIONES PLANIFICACION -------------------------------

void inciar_planificacion(){
	//iniciar_timer();
	iniciar_planificador_largo_plazo();
	log_info(kernel_logger_info, "Estructuras largo plazo creadas \n");
	iniciar_planificador_corto_plazo();
	log_info(kernel_logger_info, "Estructuras corto plazo creadas \n");
	iniciar_planificador_mediano_plazo();
	log_info(kernel_logger_info, "Estructuras mediano plazo creadas \n");
}



// --------------------------------------------------- FUNCIONES LARGO PLAZO ----------------------------------------------------------

// AVISOS y RECIBOS de MEMORIA - CONSOLA

 void avisarAModulo(int socket_fd,op_code codigo){
	 enviar_datos(socket_fd, &codigo, sizeof(op_code));

 }

 void enviar_pcb_a_memoria(pcb *pcb, int socket_memoria, op_code codigo) {
	 t_paquete* paquete = crear_paquete_con_codigo_de_operacion(codigo);
	 armarPaquete(paquete,pcb);
	 enviar_paquete(paquete, socket_memoria);
	 eliminar_paquete(paquete);
 }


pcb* obtener_entrada_tabla_de_pagina(int socket_fd,pcb* pcb) {
	t_paquete *paquete = crear_paquete_con_codigo_de_operacion(INICIALIZAR_ESTRUCTURAS);
	armarPaquete(paquete,pcb);
	enviar_paquete(paquete, socket_fd);
	log_info(kernel_logger_info, "Envio mensaje a memoria para inicializar estructuras del proceso \n");
	eliminar_paquete(paquete);
	uint32_t valorTP1;
	recibir_datos(socket_fd,&valorTP1,sizeof(uint32_t));
	pcb->valor_tabla_paginas = valorTP1;
	log_info(kernel_logger_info,"valor tabla recibido  %"PRIu32" \n",pcb->valor_tabla_paginas);
	log_info(kernel_logger_info, "Recibi paquete PCB de memoria \n ");
	return pcb;

}
op_code esperar_respuesta_memoria(int socket_memoria) {
 	op_code codigo;
 	recibir_datos(socket_memoria,&codigo,sizeof(op_code));
 	return codigo;
 }



// ---------------------------------------------------- TIMER ----------------------------------------
 void iniciar_timer() {
 	pthread_mutex_init(&mutex_timer, NULL);
 	pthread_create(&th_timer, NULL, (void *)timer, NULL);
 	pthread_detach(th_timer);
 }

 void timer(void *data) {
	tiempo = 0;
 	while(1) {
 		usleep(1000);
 		pthread_mutex_lock(&mutex_timer);
 		tiempo++;
 		pthread_mutex_unlock(&mutex_timer);
 	}
 }

/*uint32_t get_time() {
	pthread_mutex_lock(&mutex_timer);
 	uint32_t tiempo_actual = tiempo;
 	pthread_mutex_unlock(&mutex_timer);
 	return tiempo_actual;
}*/

uint32_t get_time() {
    struct timeval te;
    gettimeofday(&te, NULL); // get current time
    uint32_t milliseconds = te.tv_sec*1000LL + te.tv_usec/1000; // calculate milliseconds
    // printf("milliseconds: %lld\n", milliseconds);
    return milliseconds;
}

// --------------------------------------------------- FUNCIONES CORTO PLAZO ----------------------------------------------------------


// CONFIG_ALGORITMO

algoritmo obtener_algoritmo(){

 	 algoritmo switcher;
 	 char* algoritmo = config_valores_kernel.algoritmo_planificacion;
 	    //FIFO
 	 if (strcmp(algoritmo,"FIFO") == 0)
 	 {
 		 switcher = FIFO;
 		log_info(kernel_logger_info, "El algoritmo de planificacion elegido es FIFO \n");
 	 }

 	    //SFJ CON DESALOJO
 	 if (strcmp(algoritmo,"SRT") == 0)
 	 {
 		 switcher = SRT;
 		log_info(kernel_logger_info, "El algoritmo de planificacion elegido es SRT \n");
 	 }
 	 return switcher;
}

proceso* obtenerSiguienteReady(){
	proceso* procesoSeleccionado;
	int tamanioReady;

	pthread_mutex_lock(&mutex_ready);
 	tamanioReady = list_size(colaReady);
	pthread_mutex_unlock(&mutex_ready);

 	int gradoMultiprogramacion = config_valores_kernel.grado_multiprogramacion;
 	algoritmo algoritmo = obtener_algoritmo();
 	pthread_mutex_lock(&mutex_exec);
	int ejecutando = list_size(colaExec);
	pthread_mutex_unlock(&mutex_exec);
 	chequear_lista_pcbs(colaReady);

 	if (tamanioReady > 0 && ejecutando < gradoMultiprogramacion){
 		switch(algoritmo){
 		case FIFO:
 			procesoSeleccionado = obtenerSiguienteFIFO();
 			break;
 		case SRT:
 			procesoSeleccionado = obtenerSiguienteSRT();
 			break;
 		default:break ;
 		}

 	}
 	return procesoSeleccionado;
 }


proceso* obtenerSiguienteFIFO(){

	log_info(kernel_logger_info, "Inicio la planificacion FIFO");
 	chequear_lista_pcbs(colaReady);
	pthread_mutex_lock(&mutex_ready);
 	proceso* procesoSeleccionado = list_remove(colaReady,0);
	pthread_mutex_unlock(&mutex_ready);
 	log_info(kernel_logger_info, "PID[%d] sale de READY para plan FIFO \n", procesoSeleccionado->pcb->id_proceso);
	return procesoSeleccionado;

}

proceso* obtenerSiguienteSRT(){

	log_info(kernel_logger_info, "Inicio la planificacion SRT \n");
 	proceso* procesoElegido = elegirElDeMenorEstimacion();
 	log_info(kernel_logger_info, "PID[%d] con menor estimacion sale de READY para plan SRT \n", procesoElegido->pcb->id_proceso);
 	return procesoElegido;
 }

proceso* elegirElDeMenorEstimacion(){

	pthread_mutex_lock(&mutex_ready);
	int tamanioReady = list_size(colaReady);
	pthread_mutex_unlock(&mutex_ready);
	for(int i = 0; i < tamanioReady; i++){
		pthread_mutex_lock(&mutex_ready);
		proceso* procesoAux = list_get(colaReady,i);
		pthread_mutex_unlock(&mutex_ready);
		log_info(kernel_logger_info,"PID[%d] con estimacion: %f \n", procesoAux->pcb->id_proceso, procesoAux->pcb->estimacion_rafaga);
	}

	proceso* procesoSeleccionado;
	pthread_mutex_lock(&mutex_ready);
	proceso* procesoAux = list_get(colaReady,0);
	pthread_mutex_unlock(&mutex_ready);
	int indiceElegido = 0;
	float procesoMasCorto;
	procesoMasCorto = procesoAux->pcb->estimacion_rafaga;
	for(int i = 0; i < tamanioReady; i++){
		pthread_mutex_lock(&mutex_ready);
		proceso* procesoAux = list_get(colaReady,i);
		pthread_mutex_unlock(&mutex_ready);
		if(procesoMasCorto > procesoAux->pcb->estimacion_rafaga){
			procesoMasCorto = procesoAux->pcb->estimacion_rafaga;
			indiceElegido = i;
		}
	}

	pthread_mutex_lock(&mutex_ready);
	procesoSeleccionado = list_remove(colaReady,indiceElegido);
	pthread_mutex_unlock(&mutex_ready);

	return procesoSeleccionado;
}


void calculoEstimacionProceso(proceso *proceso){
	float alfa = config_valores_kernel.alfa;
	float estimacionInicial = proceso->pcb->estimacion_rafaga;
	float realAnterior = proceso->pcb->rafaga_anterior;
	float nuevaEstimacion = alfa * realAnterior + (1 - alfa) * estimacionInicial;
	proceso->pcb->estimacion_rafaga = nuevaEstimacion;
	log_info(kernel_logger_info,"Rafaga anterior de la PCB: %d \n",proceso->pcb->rafaga_anterior);
	log_info(kernel_logger_info, "Nueva estimacion del pid[%d]: %f",proceso->pcb->id_proceso, proceso->pcb->estimacion_rafaga);

}

void interrumpir_cpu(){
	op_code codigo = MENSAJE;
	enviar_datos(socket_interrupt, &codigo, sizeof(codigo));
	log_info(kernel_logger_info, "Se envia mensaje de interrupcion a cpu \n");

}

void ejecutarIO(uint32_t tiempoIO){
	usleep(tiempoIO * 1000);
}

void transicion_interrupcion(){
	algoritmo algo = obtener_algoritmo();
	
	pthread_mutex_lock(&mutex_exec);
	log_info(kernel_logger_info," Cant Procesos Ejecutando %d \n ",list_size(colaExec));
	if(algo == SRT && !list_is_empty(colaExec)){
		pthread_mutex_unlock(&mutex_exec);
		pthread_mutex_lock(&mutex_interrupcion);
		interrupcion = 1;
		pthread_mutex_unlock(&mutex_interrupcion);
		interrumpir_cpu();
	}
	else {
		pthread_mutex_unlock(&mutex_exec);
	}
}



//..................................... DESTRUCCIONES............................................................................

void finalizar_kernel(){
	 log_info(kernel_logger_info,"Finalizando el modulo Kernel");
	 log_destroy(kernel_logger_info);
	 destruir_semaforos();
	 destruir_listas();
	 liberar_conexion(server_fd);
	 liberar_conexion(socket_memoria);
	 liberar_conexion(socket_dispatch);
	 liberar_conexion(socket_interrupt);

}

// SEMAFOROS


 void destruir_semaforos(void){

 	pthread_mutex_destroy(&mutex_new);
 	pthread_mutex_destroy(&mutex_ready);
 	pthread_mutex_destroy(&mutex_blocked);
 	pthread_mutex_destroy(&mutex_exec);
 	pthread_mutex_destroy(&mutex_exit);
 	pthread_mutex_destroy(&mutex_suspended_blocked);
 	pthread_mutex_destroy(&mutex_suspended_ready);
 	pthread_mutex_destroy(&mutex_generador_id);
	pthread_mutex_destroy(&mutex_interrupcion);
	pthread_mutex_destroy(&mutex_timer);
 	sem_destroy(&sem_ready);
 	sem_destroy(&sem_blocked);
 	sem_destroy(&sem_desalojo);
 	sem_destroy(&sem_suspended_ready);
 	sem_destroy(&sem_admitir);
 	sem_destroy(&sem_exec);
 	sem_destroy(&sem_grado_multiprogramacion);
 	sem_destroy(&sem_exit);

 }


// LISTAS

 void destruir_listas(void){

 	list_destroy_and_destroy_elements(colaNew, free);
 	list_destroy_and_destroy_elements(colaReady, free);
 	list_destroy_and_destroy_elements(colaExec, free);
 	list_destroy_and_destroy_elements(colaBlocked, free);
	list_destroy_and_destroy_elements(colaSuspendedBlocked, free);
	list_destroy_and_destroy_elements(colaSuspendedReady, free);
 	list_destroy_and_destroy_elements(colaExit, free);

 }
