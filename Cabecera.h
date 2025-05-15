#ifndef CABECERA_H_INCLUDED
#define CABECERA_H_INCLUDED
#include <stdint.h>
#include <string.h>
typedef struct {

    char identificador[5];
    uint8_t version;
    uint16_t TamanioCodigo;
    uint16_t TamanioData;
    uint16_t TamanioExtra;
    uint16_t TamanioStack;
    uint16_t TamanioConst;
    uint16_t OffsetEntry;

}Theader;
int ValidaEjecucion(char [], uint8_t);

#endif // CABECERA_H_INCLUDED
