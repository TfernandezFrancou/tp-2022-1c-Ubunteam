#include "memoria.h"
#include <sys/mman.h>
#include "string.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#ifndef INCLUDE_SWAP_H_
#define INCLUDE_SWAP_H_

//FUNCIONES


char* armarPath(int );
void crearSwap(int /*,int */);
void eliminarSwap(int );
void supender_proceso(int ) ;


#endif /* INCLUDE_SWAP_H_ */