#include <string.h>
#include "Cabecera.h"

int ValidaEjecucion(Theader header){
	return (strcmp(header.identificador, "VMX25") && header.version == 1);
}
