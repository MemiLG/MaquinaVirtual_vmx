#ifndef CABECERA_H_INCLUDED
#define CABECERA_H_INCLUDED
#include <stdint.h>
#include <string.h>
typedef struct {

    char identificador[5];
    uint8_t version;
    uint16_t TamanioCodigo;

}Theader;
int ValidaEjecucion(Theader );

#endif // CABECERA_H_INCLUDED
