#include <string.h>
#include "Cabecera.h"

int ValidaEjecucion(char identificador[5], uint8_t version){
	return ((strcmp(identificador, "VMX25") && (version == 1 || version == 2)) && (strcmp(identificador,"VMI25")&& version == 1));
}
