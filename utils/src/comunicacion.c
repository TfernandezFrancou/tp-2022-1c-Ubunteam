#include "comunicacion.h"

// NO TOCAR FUNCIONES TP0 - SOLO SE AGREGAN NUEVAS FUNCIONES EN CASO DE SER NECESARIO




//----------------------------------ENVIO - RECIBO DE  PAQUETE/BUFFER/MENSAJE/OPERACION------------------------------------

// OPERACION

int recibir_operacion(int socket_cliente) {
   int cod_op;
    if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) != 0)
        return cod_op;
    else
    {
        close(socket_cliente);
        return -1;
    }
}

// MENSAJE

void enviar_mensaje(char *mensaje, int socket_cliente)
{
    t_paquete *paquete = malloc(sizeof(t_paquete));

    paquete->codigo_operacion = MENSAJE;
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->stream_size = strlen(mensaje) + 1;
    paquete->buffer->stream = malloc(paquete->buffer->stream_size);
    memcpy(paquete->buffer->stream, mensaje, paquete->buffer->stream_size);

    int bytes = paquete->buffer->stream_size + 2 * sizeof(int);

    void *a_enviar = serializar_paquete_con_bytes(paquete,bytes);

    for(int i = 0 ; i<bytes; i++){
    	printf("%02x ",((char*) a_enviar)[i]);
    }
    send(socket_cliente, a_enviar, bytes, 0);

    free(a_enviar);
    eliminar_paquete(paquete);
}

void recibir_mensaje(int socket_cliente,t_log* logger) {
    int size;
    char* buffer = recibir_buffer(&size, socket_cliente);
    log_info(logger, "Me llego el mensaje %s", buffer);
    free(buffer);
}


 // BUFFER

void crear_buffer(t_paquete *paquete)
{
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->stream_size = 0;
    paquete->buffer->stream = NULL;
}

void *recibir_stream(int *size, int socket_cliente)
{
    void *stream;

    recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
    stream = malloc(*size);
    recv(socket_cliente, stream, *size, MSG_WAITALL);

    return stream;
}


// recibir buffer comun sin tamnio proceso

 void* recibir_buffer(int* size, int socket_cliente) {
     void * buffer;

     recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
     buffer = malloc(*size);
     recv(socket_cliente, buffer, *size, MSG_WAITALL);

     return buffer;
 }


// recibir buffer para pcb

t_buffer *recibir_buffer_proceso(int socket_cliente) // deserializar paquete instrucciones y tamanio proceso
{
	t_buffer* buffer = malloc(sizeof(t_buffer)) ;
    int size;
    void*stream ;

//    recv(socket_cliente, &(buffer->tamanio_proceso), sizeof(int), MSG_WAITALL);
    recv(socket_cliente, &size, sizeof(int), MSG_WAITALL);
    stream = malloc(size);
    recv(socket_cliente, stream, size, MSG_WAITALL);

    buffer->stream = stream ;


    return buffer ;
}

void agregar_a_buffer(t_buffer *buffer, void *src, uint32_t size) {
	buffer->stream = realloc(buffer->stream, buffer->stream_size + size);
	memcpy(buffer->stream + buffer->stream_size, src, size);
	buffer->stream_size+=size;
}

t_buffer *inicializar_buffer_con_parametros(uint32_t size, void *stream) {
	t_buffer *buffer = (t_buffer *)malloc(sizeof(t_buffer));
	buffer->stream_size = size;
	buffer->stream = stream;
	return buffer;
}

// PAQUETE


void* serializar_paquete_con_bytes(t_paquete* paquete, int bytes)
{
	void * magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->stream_size), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->stream_size);
	desplazamiento+= paquete->buffer->stream_size;

	return magic;
}

t_buffer* serializar_paquete(t_paquete* paquete) {
    t_buffer * magic = inicializar_buffer_con_parametros(0, NULL);

    agregar_a_buffer(magic, &(paquete->codigo_operacion), sizeof(uint8_t));
    agregar_a_buffer(magic, &(paquete->buffer->stream_size), sizeof(uint32_t));
    agregar_a_buffer(magic, paquete->buffer->stream, paquete->buffer->stream_size);

    return magic;
}

// paquete con codigo de operacion solo paquete

t_paquete *crear_paquete(void){
    t_paquete *paquete = malloc(sizeof(t_paquete));

    paquete->codigo_operacion = PAQUETE;
    crear_buffer(paquete);
    return paquete;
}

//paquete con cualquier codigo de operacion

t_paquete *crear_paquete_con_codigo_de_operacion(op_code codigo){
    t_paquete *paquete = malloc(sizeof(t_paquete));

    paquete->codigo_operacion = codigo;
    crear_buffer(paquete);
    return paquete;
}


void agregar_a_paquete(t_paquete *paquete, void *valor, uint32_t tamanio_valor) {
    agregar_a_buffer(paquete->buffer, &tamanio_valor, sizeof(uint32_t));
    agregar_a_buffer(paquete->buffer, valor, tamanio_valor);
}


void agregar_entero_a_paquete(t_paquete *paquete, int tamanio_proceso) // Agregar un entero a un paquete (ya creado)
{
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->stream_size + sizeof(int));
    memcpy(paquete->buffer->stream , &tamanio_proceso, sizeof(int));
    paquete->buffer->stream_size += tamanio_proceso + sizeof(int);
}

void agregar_datos_consola(t_paquete* paquete,void *valor, int tamanio_valor,int tamanio_proceso) {

	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->stream_size + tamanio_valor + sizeof(int));

	memcpy(paquete->buffer->stream , &tamanio_proceso, sizeof(int));
    memcpy(paquete->buffer->stream + paquete->buffer->stream_size, &tamanio_valor, sizeof(int));
    memcpy(paquete->buffer->stream + paquete->buffer->stream_size + sizeof(int), valor, tamanio_valor);

    paquete->buffer->stream_size += tamanio_proceso + sizeof(int);
    paquete->buffer->stream_size += tamanio_valor + sizeof(int);


}
void enviar_paquete(t_paquete *paquete, int socket_cliente)
{
    t_buffer *a_enviar = serializar_paquete(paquete);
    char* stream = (char*)(a_enviar->stream);
    printf("enviar paquete \n");
    for(int i=0 ; i<a_enviar->stream_size;i++){
    	printf("%02X ",stream[i]);
    }
    send(socket_cliente, a_enviar->stream, a_enviar->stream_size, 0);
    free(a_enviar->stream);
    free(a_enviar);
}

t_list *recibir_paquete(int socket_cliente)
{
    int size;
    int desplazamiento = 0;
    void *stream;
    t_list *valores = list_create();
    int tamanio;

    stream = recibir_stream(&size, socket_cliente);

    while (desplazamiento < size)
    {
        memcpy(&tamanio, stream + desplazamiento, sizeof(int));
        desplazamiento += sizeof(int);
        char *valor = malloc(tamanio);
        memcpy(valor, stream + desplazamiento, tamanio);
        desplazamiento += tamanio;
        list_add(valores, valor);
    }
    free(stream);
    return valores;
}

void eliminar_paquete(t_paquete* paquete) {
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
}

//----------------------------------SERIALIZAR INSTRUCCIONES------------------------------------

t_paquete *serializar_instrucciones(t_list *instrucciones, op_code codigo) {
	t_paquete *paquete = crear_paquete_con_codigo_de_operacion(codigo);

	for(int i=0; i<list_size(instrucciones); i++) {
		instruccion *instr = (instruccion *)list_get(instrucciones, i);
		agregar_entero_a_paquete(paquete, instr->codigo);
		agregar_entero_a_paquete(paquete, instr->parametro1);
		agregar_entero_a_paquete(paquete, instr->parametro2);
	}
	return paquete;

}



//----------------------------------ENVIO/RECIBO DE PCBS----------------------------------

void enviarPcb(pcb* proceso,int socket_cliente){
	op_code codigo=PCB;
	t_buffer* buffer=serializar_pcb(proceso);
	int bytes=sizeof(int)*2+buffer->stream_size;
	void* a_enviar=malloc(bytes);
	memcpy(a_enviar,&(codigo),sizeof(int));
	memcpy(a_enviar+sizeof(int),&buffer->stream_size,sizeof(int));
	memcpy(a_enviar+sizeof(int)*2,&buffer->stream,buffer->stream_size);
	send(socket_cliente,a_enviar,bytes,0);
	free(buffer);
}

pcb* recibir_pcb(int socket_cliente){
	int size;
	void *stream;
	pcb* recibido;
	stream= recibir_stream(&size, socket_cliente);
	recibido=deserializar_pcb(stream);
	free(stream);
	return recibido;
}




//----------------------------------SERIALIZAR_PCB---------------------------------------

void *serializar_pcb(pcb* pcb)
{
    t_buffer* buffer = malloc(sizeof(t_buffer));
    buffer->stream_size = sizeof(uint32_t) * 4 // Para los unint32
             + sizeof(1) * 1 // Para los int ;
             + sizeof(float) * 1 // Para los float
             + sizeof(uint8_t) * 2 // Para los uint_8_t
             + list_size(pcb->instrucciones) * sizeof(instruccion);

    void* stream = malloc(buffer->stream_size);
    int offset = 0; // Desplazamiento

    // Serializar los campos int , float y double

    memcpy(stream + offset, &pcb->id_proceso, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(stream + offset, &pcb->tamanio_proceso, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(stream + offset, &pcb->valor_tabla_paginas, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(stream + offset, &pcb->program_counter, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(stream + offset, &pcb->estimacion_rafaga, sizeof(float));
    offset += sizeof(float);
    memcpy(stream + offset, &pcb->tiempo_de_bloqueo,sizeof(double));
    offset += sizeof(double);
    memcpy(stream + offset, &pcb->suspendido, sizeof(uint8_t));
    offset += sizeof(uint8_t);
    memcpy(stream + offset, &pcb->rafaga_anterior, sizeof(uint8_t));
    offset += sizeof(uint8_t);

    //Serializar los campos enum

    memcpy(stream + offset, &pcb->estado_proceso, sizeof(estado));
    offset += sizeof(estado);


    //Serializar lista instrucciones

    int cantidad_instrucciones = list_size(pcb->instrucciones) ;

    memcpy(stream + offset, &cantidad_instrucciones, sizeof(1)); // primero se copia el tamnio de la lista para despues ir agregando los otros campos de la lista
    offset += sizeof(1);

    for (int i = 0; i < cantidad_instrucciones; i++){ // hacemos un char* a la vez

        memcpy(stream + offset, list_get(pcb->instrucciones,i), sizeof(instruccion));
        offset += sizeof(instruccion);
    }

    buffer->stream = stream;


    list_destroy(pcb->instrucciones);


    return buffer ;

}


///----------------------------------DESERIALIZAR PCB ----------------------------------

pcb* deserializar_pcb(void* stream) {

	pcb* pcb=malloc(sizeof(pcb));
	//Deserializar los campos int, double  y float

    memcpy(&(pcb->id_proceso), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(&(pcb->tamanio_proceso), stream ,sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(&(pcb->valor_tabla_paginas), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(&(pcb->program_counter), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(&(pcb->estimacion_rafaga), stream, sizeof(float));
    stream += sizeof(float);
    memcpy(&(pcb->tiempo_de_bloqueo), stream, sizeof(double));
    stream += sizeof(double);
    memcpy(&(pcb->suspendido), stream, sizeof(uint8_t));
    stream += sizeof(uint8_t);

    //Deserializar los campos enum*

    memcpy(&(pcb->estado_proceso), stream, sizeof(estado));
    stream += sizeof(estado);

    //Deserializar lista instrucciones

    pcb->instrucciones = list_create();

    int cant_instrucciones ;
    memcpy(&(cant_instrucciones), stream, sizeof(1));  // primero se deserializa el tamanio de la lista para despues ir deserializando los otros campos de la lista
    stream += sizeof(1);

    for(int i = 0 ; i<cant_instrucciones; i++) {
    	instruccion* instruccion_deserializar = malloc(sizeof(instruccion));
        memcpy(&(instruccion_deserializar->codigo), stream, sizeof(codigo_instrucciones));
        stream += sizeof(codigo_instrucciones);
        memcpy(&(instruccion_deserializar->parametro1), stream, sizeof(1));
        stream += sizeof(1);
        memcpy(&(instruccion_deserializar->parametro2), stream, sizeof(1));
        stream += sizeof(1);

        list_add(pcb->instrucciones,instruccion_deserializar);
        free(instruccion_deserializar);
    }
    return pcb;
}


//HANDSHAKE
void pedir_handshake(int socket_memoria){
	t_paquete *paquete = malloc(sizeof(t_paquete));
	char* mensaje_handshake="HANDSHAKE";

	   paquete->codigo_operacion = HANDSHAKE;
	    paquete->buffer = malloc(sizeof(t_buffer));
	    paquete->buffer->stream_size = strlen(mensaje_handshake) + 1;
	    paquete->buffer->stream = malloc(paquete->buffer->stream_size);
	    memcpy(paquete->buffer->stream, mensaje_handshake, paquete->buffer->stream_size);

	    int bytes = paquete->buffer->stream_size + 2 * sizeof(int);

	    void *a_enviar = serializar_paquete(paquete);

	    send(socket_memoria, a_enviar, bytes, 0);

	    free(a_enviar);
	    eliminar_paquete(paquete);
}
t_handshake* recibir_handshake(int socket_memoria){
	t_handshake* han=malloc(sizeof(t_handshake));
	int size;
	void* stream=recibir_stream(&size,socket_memoria);
	memcpy(&(han->tam_pagina),stream,sizeof(int));
	memcpy(&(han->entradas),stream+sizeof(int),sizeof(int));
	free(stream);
	return han;
}
//TABLA DE PAGINA
void pedir_tabla_pagina(int socket,uint32_t tabla,uint32_t entrada){
	t_paquete* paquete=crear_paquete();
	paquete->codigo_operacion=TABLA;
	agregar_entero_a_paquete(paquete,tabla);
	agregar_entero_a_paquete(paquete,entrada);
	enviar_paquete(paquete,socket);
}
//MARCO
void pedir_marco(int socket,uint32_t tabla,uint32_t entrada){
	t_paquete* paquete=crear_paquete();
	paquete->codigo_operacion=MARCO;
	agregar_entero_a_paquete(paquete,tabla);
	agregar_entero_a_paquete(paquete,entrada);
	enviar_paquete(paquete,socket);
}
