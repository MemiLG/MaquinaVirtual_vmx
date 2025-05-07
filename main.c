#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "Operadores.h"
#include "TipoMaquina.h"
#include "Cabecera.h"
#define ARCHIVO 3
typedef struct{
    char ident[5];
}str;
/*
    CORTES DE EJECUCION DE LA MV:

    Si comp.error = 1: Corto la ejecucion por el error de instruccion invalida
    Si comp.error = 2: Corto la ejecucion por el error de division por cero
    Si comp.error = 3: Corto la ejecucion por el error de caida de segmento
    Si comp.error = 4: La ejecucion finalizo exitosamente con 0 errores.
    Si comp.error = 5: Corto por lectura incorrecta del archivo
*/
//--PROTOTIPOS--
TDatos obtener_abc(int8_t);
void EjecutarOperacion(TDatos,Toperando,Toperando,Componentes *);
void LeeArchivo(Componentes *); //, char argv[ARCHIVO]
void IniciaRegistros(Componentes*);
void CargaOperando(int8_t,Toperando*,Componentes*);
void Llamada_Disassembler(Componentes );

//--EJECUCION--
int main(int argc, char *argv[]) // argc indica la cantidad de argumentos ingresados por consola. *argv[] es una matriz de punteros a un matrices de caracteres
{

	TDatos abc ;
	Componentes comp;
	Toperando A, B;
	int8_t instruccion ;
	int dirip, IP_no_caido=1;

	IniciaRegistros(&comp);
	LeeArchivo(&comp); //, argv[1]

    //if (strcmp(argv[2],"-d") && comp.error!=5)
    if (comp.error!=5)
        Llamada_Disassembler(comp);


	while (comp.error == 0 && IP_no_caido)
    {

        dirip = comp.registros[5]; //pone en una variable int la direccion logica de donde apunta ip
        TradLogicaFisica(&dirip,comp,&IP_no_caido);
        if (IP_no_caido){

            instruccion = comp.memoria[dirip];
            abc = obtener_abc(instruccion);
            comp.registros[5] += 0x00000001; //Mueve el puntero de IP a la proxima instruccion (le suma 1 al offset);

            if(abc.OpB!=0)
                CargaOperando(abc.OpB,&B,&comp);//Carga el valor del operando b y mueve IP

            if(abc.OpA!=0)
                CargaOperando(abc.OpA,&A,&comp);//Carga el valor del operando a y mueve IP

            EjecutarOperacion(abc,A,B,&comp);
            TradLogicaFisica(&dirip,comp,&IP_no_caido);
        }

	}


	if (comp.error == 1 )
        printf("MV finaliza por error de instruccion invalida\n");
    else
        if (comp.error == 2)
            printf("MV finaliza por error de division por 0\n");
        else
            if(comp.error == 3 ^ dirip > comp.tabladesegmentos[0][1])
                printf("MV finaliza por error de caida de segmento\n");
            else
                if (comp.error == 5)
                    printf("MV finaliza por error de archivo\n");
                else
                    printf("MV finaliza exitosamente con 0 errores\n");
    return 0;
}

//--IMPLEMENTACION--

TDatos obtener_abc(int8_t instruccion){

	TDatos aux;

	aux.OpB = (instruccion >> 6 ) & 0x03 ;
	aux.OpA = (instruccion >> 4 ) & 0x03 ;
	aux.CodOperacion = instruccion & 0x1F ;

	return aux;

}

void EjecutarOperacion(TDatos abc, Toperando a, Toperando b, Componentes *comp){

    switch (abc.CodOperacion){

        case 0x00 : SYS(b, comp);
            break;
        case 0x01 : JMP(b, comp);
            break;
        case 0x02 : JZ(b, comp);
            break;
        case 0x03 : JP(b, comp);
            break;
        case 0x04 : JN(b, comp);
            break;
        case 0x05 : JNZ(b, comp);
            break;
        case 0x06 : JNP(b, comp);
            break;
        case 0x07 : JNN(b, comp);
            break;
        case 0x08 : NOT(b,comp);
            break;
        case 0x0F : STOP(comp);
            break;
        case 0x10 : MOV(a,b,comp);
            break;
        case 0x11 : ADD(a,b,comp);
            break;
        case 0x12 : SUB(a,b,comp);
            break;
        case 0x13 : SWAP(a,b,comp);
            break;
        case 0x14 : MUL(a,b,comp);
            break;
        case 0x15 : DIV(a,b,comp);
            break;
        case 0x16 : CMP(a,b,comp);
            break;
        case 0x17 : SHL(a,b,comp);
            break;
        case 0x18 : SHR(a,b,comp);
            break;
        case 0x19 : AND(a,b,comp);
            break;
        case 0x1A : OR(a,b,comp);
            break;
        case 0x1B : XOR(a,b,comp);
            break;
        case 0x1C : LDL(a,b,comp);
            break;
        case 0x1D : LDH(a,b,comp);
            break;
        case 0x1E : RND(a,b,comp);
            break;
        default: comp->error = 1; //Error por instruccion invalida
    }
}

void LeeArchivo(Componentes *comp){ //, char argv[]
    FILE *arch;
    Theader cab;
    str ident;
    int boo,i;
    uint8_t lect;
    uint16_t tam,aux=0;

    arch = fopen("sample.vmx","rb"); //argv
    if (arch == NULL){
        printf("No se pudo leer el archivo\n");
        (*comp).error = 5;
    }
    else{

        fread(&ident,sizeof(str),1,arch);
        fread(&lect,sizeof(uint8_t),1,arch);;
        fread(&tam,sizeof(uint16_t),1,arch);
        strcpy(cab.identificador,ident.ident);
        aux = (tam>>8) & 0xFF;
        tam = (tam<<8) & 0xFF00;
        cab.TamanioCodigo = 0;
        cab.TamanioCodigo = (cab.TamanioCodigo | aux) | tam;
        cab.version = lect;
        boo = ValidaEjecucion(cab);
        if (boo){
            setBaseCS(comp,cab.TamanioCodigo);
            setTamanioCS(comp,cab.TamanioCodigo);
            setBaseDS(comp,cab.TamanioCodigo);
            setTamanioDS(comp,cab.TamanioCodigo);
            i=0;
            while(fread(&lect,sizeof(uint8_t),1,arch)>0){ //se supone que lee exactamente lo que dice la cabecera (por lo tanto no se cae del segmento de codigo). Preguntar si esta bien en clase
                (*comp).memoria[i] = lect;
                i++;
            }
        }
        else{
            printf("No es un archivo valido\n");
            (*comp).error = 5;
        }
        fclose(arch);
    }
}

void IniciaRegistros( Componentes *comp )
{

	(*comp).registros[0] = 0 ;
	(*comp).registros[1] = 0x00010000 ;
	(*comp).registros[5] = (*comp).registros[0];
	(*comp).error = 0;

}

void CargaOperando(int8_t tipo, Toperando *a, Componentes *comp){
    int dir, bytes=0, flag;

    a->tipo = tipo;
    dir = (*comp).registros[5];//direccion logica de lo que apunta IP
    TradLogicaFisica(&dir, *comp, &flag); //Traducion a direccion fisica de la posicion apuntada por IP
    if(flag){
        switch(tipo){
        case 0x01: //registro
            bytes = 1;
            break;
        case 0x02: //inmediato
            bytes = 2;
            break;
        case 0x03: //memoria
            bytes = 3;
            break;
        case 0x00: //ninguno
            bytes = 0;
            break;
        }
        (*a).operando = LeerMemoria(*comp,dir,bytes);
        (*comp).registros[5] += bytes; //suma la cantidad de bytes que se movio al offset (mueve IP)
        }
    else
        (*comp).error = 3;
}

void Llamada_Disassembler(Componentes comp){

    int fin ;
    int8_t instruccion;
    TDatos abc;


    comp.registros[5] = comp.registros[0];
    fin = (comp.tabladesegmentos[0][1] + comp.tabladesegmentos[0][0]);


    while(comp.registros[5] < fin )
    {

        instruccion = comp.memoria[comp.registros[5]];
        abc = obtener_abc(instruccion);
        Disassembler(comp,abc,comp.registros[5]);
        comp.registros[5] += abc.OpA + abc.OpB +1;

    }

}
