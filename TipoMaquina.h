#ifndef TIPOMAQUINA_H_INCLUDED
#define TIPOMAQUINA_H_INCLUDED
#define TAM 16384
#define TAMR 16
#define TOTAL 31
#define TOTALR 17
#define COL 2
#define FIL 2
#include <stdint.h> //para usar int8_t

typedef char stringg[5];

typedef struct{
	uint8_t memoria[TAM];
	int tabladesegmentos[FIL][COL];
	int error;
	int registros[TAMR];
}Componentes;

typedef struct{
	uint8_t tipo;
	int operando;
}Toperando;

typedef struct {

	int8_t OpB,OpA,CodOperacion;

}TDatos;

int DireccionFisicaValida(int ,int16_t ,Componentes );
void modificaCC(Componentes*, int );
int LeerMemoria(Componentes , int , int );
void TradLogicaFisica(int*, Componentes , int*);
void InsertaMemoria(Componentes *, int ,int, int );
void Disassembler(Componentes,TDatos,int);
void Op_AB(int8_t , Componentes , int32_t *,int*);
void Significado(int8_t , int32_t);
void setBaseCS(Componentes *, uint16_t );
void setBaseDS(Componentes *,uint16_t );
void setTamanioCS(Componentes *,uint16_t );
void setTamanioDS(Componentes *,uint16_t );
void Llamada_Disassembler(Componentes);

#endif // TIPOMAQUINA_H_INCLUDED
