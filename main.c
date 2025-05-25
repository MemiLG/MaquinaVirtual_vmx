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

    Si comp.error = 1: Corta la ejecucion por el error de instruccion invalida
    Si comp.error = 2: Corta la ejecucion por el error de division por cero
    Si comp.error = 3: Corta la ejecucion por el error de caida de segmento
    Si comp.error = 4: Corta por archivo no compatible.
    Si comp.error = 5: Corta por memoria insuficiente.
    Si comp.error = 6: Corta por Stack overflow.
    Si comp.error = 7: Corta por Stack underflow.

*/
//--PROTOTIPOS--
TDatos obtener_abc(int8_t);
void EjecutarOperacion(TDatos,Toperando,Toperando,Componentes *);
void LeeArchivo(Componentes *, char argv[ARCHIVO]);
void IniciaComponentes(Componentes*);
void CargaOperando(int8_t,Toperando*,Componentes*);
void Llamada_Disassembler(Componentes );
void CargaRegistros (Componentes*);
int DarVuelta (int valor);

//--EJECUCION--


int main(int argc, char *argv[]) // argc indica la cantidad de argumentos ingresados por consola. *argv[] es una matriz de punteros a un matrices de caracteres
{

	TDatos abc ;
	Componentes comp;
	Toperando A, B, pargv, pargc, fin;
	int8_t instruccion ;
	int dirip, IP_no_caido=1, maxmemoria = 16384, boodisassembler = 0, cantparam=0, i=2, j,dir,puntparam[10]={0}, punteroparam = -1, tamcad;
	uint16_t tamparam = 0;


	IniciaComponentes(&comp);

    while (i<argc){
        if (argv[i][0] == 'm'){
            sscanf(argv[i],"m=%d",&maxmemoria);
            maxmemoria *=1024;
            comp.tamanio = maxmemoria;
        }
        else
            if (strcmp(argv[i],"-d")==0){
                boodisassembler = 1;
            }
            else
                if (strcmp(argv[i],"-p")==0){
                    i++;
                    cantparam = 0;
                    dir = 0;
                    for (;i<argc;i++){
                        j=0;
                        puntparam[cantparam] = dir;
                        cantparam++;
                        while(argv[i][j]){
                            InsertaMemoria(&comp,dir,argv[i][j],1);
                            tamparam++;
                            dir +=1;
                            j++;
                        }
                        InsertaMemoria(&comp,dir,0,1);
                        tamparam++;
                        dir +=1;
                    }
                    punteroparam = dir;
                    for (j=0;j<cantparam;j++){
                        InsertaMemoria(&comp, dir,puntparam[j],4);
                        tamparam+=4;
                        dir+=1;
                    }
                    setTamanioPS(&comp,tamparam);
                }
                else{
                    tamcad = strlen(argv[i]);
                    if (strcmp(argv[i]+(tamcad - 4),".vmi")==0){
                        comp.img.booimagen=1; //la maquina puede frenar en un breakpoint y generar la imagen en la ruta .vmi
                        strcpy(comp.img.nombre,argv[i]);
                    }

                }
        i++;
    }

	LeeArchivo(&comp, argv[1]);
    printf("PS inicio: %X\n", comp.tabladesegmentos[0][0]);
    printf("PS fin: %X\n", comp.tabladesegmentos[0][0] + comp.tabladesegmentos[0][1]);
    printf("KS inicio: %X\n", comp.tabladesegmentos[1][0]);
    printf("KS fin: %X\n", comp.tabladesegmentos[1][0] + comp.tabladesegmentos[1][1]);
    printf("CS inicio: %X\n", comp.tabladesegmentos[2][0]);
    printf("CS fin: %X\n", comp.tabladesegmentos[2][0] + comp.tabladesegmentos[2][1]);
    printf("DS inicio: %X\n", comp.tabladesegmentos[3][0]);
    printf("DS fin: %X\n", comp.tabladesegmentos[3][0] + comp.tabladesegmentos[3][1]);
    printf("ES inicio: %X\n", comp.tabladesegmentos[4][0]);
    printf("ES fin: %X\n", comp.tabladesegmentos[4][0] + comp.tabladesegmentos[4][1]);
    printf("SS inicio: %X\n", comp.tabladesegmentos[5][0]);
    printf("SS fin: %X\n", comp.tabladesegmentos[5][0] + comp.tabladesegmentos[5][1]);
	CargaRegistros(&comp);

	pargc.tipo = pargv.tipo = fin.tipo = 2;
	pargv.operando = punteroparam;
	pargc.operando = cantparam;
	fin.operando = -1;

	for (int i=0; i<6; i++){
        for(int j=0;j<2;j++)
            printf("%d\t",comp.tabladesegmentos[i][j]);
        printf("\n");
	}

    for (int i=0; i<60; i++){
        printf("%x\t",comp.memoria[0]);
    }

	push(pargv,&comp);
	push(pargc,&comp);
	push(fin,&comp); //seria el ret de la subrutina principal (ver si esta bien)

    if (boodisassembler && comp.error == 0){
        Llamada_Disassembler(comp);
    }


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
            if(comp.sigue_breakpoint == 1 && !(abc.CodOperacion==0 && B.operando==15))
                breakpoint(&comp);
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
                if (comp.error == 4)
                    printf("MV finaliza por error de archivo\n");
                else
                    if (comp.error == 5)
                        printf("MV finaliza por error de memoria insuficiente");
                    else
                        if (comp.error == 6)
                            printf("MV finaliza por error de stack overflow");
                        else
                            if (comp.error == 7)
                                printf("MV finaliza por error de Stack underflow");
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
        case 0x08 : NOT(b, comp);
            break;
        case 0x0B : push(b, comp);
            break;
        case 0x0C : pop(b, comp);
            break;
        case 0x0D : call(b, comp);
            break;
        case 0x0E: ret(comp);
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

void LeeArchivo(Componentes *comp, char argv[]){
    FILE *arch;
    Theader cab;
    str ident;
    int boo,i,tamanioseg = 0, dir, lect4, tamcad, tamtot, flag=1; //tamanioseg es un  acumulador que servira para corroborar que la memoria sea suficiente a la hora de cargar el programa
    uint8_t lect;
    uint16_t tam,aux=0,ultam=0,base=0;

    arch = fopen(argv,"rb");
    if (arch == NULL){
        printf("No se pudo leer el archivo\n");
        (*comp).error = 4;
    }
    else{

        fread(&ident,sizeof(str),1,arch);
        fread(&lect,sizeof(uint8_t),1,arch);
        fread(&tam,sizeof(uint16_t),1,arch);
        strcpy(cab.identificador,ident.ident);
        aux = (tam>>8) & 0xFF;
        tam = (tam<<8) & 0xFF00;
        cab.TamanioCodigo = 0;
        cab.TamanioCodigo = (cab.TamanioCodigo | aux) | tam; //en la opcion de vmi, esta variable guarda el tamanio de la memoria en kib
        tamanioseg += cab.TamanioCodigo;
        cab.version = lect;
        boo = ValidaEjecucion(cab.identificador,cab.version);
        if (boo){

            tamcad = strlen(argv);
            if (strcmp(argv+(tamcad - 4),".vmx")==0){

                if (cab.version == 1){
                    setBaseCS(comp,0);
                    setTamanioCS(comp,cab.TamanioCodigo);
                    if (cab.TamanioCodigo<=comp->tamanio){
                        setBaseDS(comp,cab.TamanioCodigo);
                        aux = comp->tamanio - cab.TamanioCodigo;
                        setTamanioDS(comp,aux);
                        i=0;
                        while(fread(&lect,sizeof(uint8_t),1,arch)>0){ //se supone que lee exactamente lo que dice la cabecera (por lo tanto no se cae del segmento de codigo). Preguntar si esta bien en clase
                            (*comp).memoria[i] = lect;
                            i++;
                        }
                    }
                    else
                        comp->error = 5;
                }
                else{

                    //--LECTURA DE LOS TAMANIOS DE CADA SEGMENTO--

                    tamanioseg += comp->tabladesegmentos[0][1];

                    aux = 0;
                    fread(&tam,sizeof(uint16_t),1,arch);
                    aux = (tam>>8) & 0xFF;
                    tam = (tam<<8) & 0xFF00;
                    cab.TamanioData = 0;
                    cab.TamanioData = (cab.TamanioData | aux) | tam;
                    tamanioseg += cab.TamanioData;

                    aux = 0;
                    fread(&tam,sizeof(uint16_t),1,arch);
                    aux = (tam>>8) & 0xFF;
                    tam = (tam<<8) & 0xFF00;
                    cab.TamanioExtra = 0;
                    cab.TamanioExtra = (cab.TamanioExtra | aux) | tam;
                    tamanioseg += cab.TamanioExtra;

                    aux = 0;
                    fread(&tam,sizeof(uint16_t),1,arch);
                    aux = (tam>>8) & 0xFF;
                    tam = (tam<<8) & 0xFF00;
                    cab.TamanioStack = 0;
                    cab.TamanioStack = (cab.TamanioStack | aux) | tam;
                    tamanioseg += cab.TamanioStack;

                    aux = 0;
                    fread(&tam,sizeof(uint16_t),1,arch);
                    aux = (tam>>8) & 0xFF;
                    tam = (tam<<8) & 0xFF00;
                    cab.TamanioConst = 0;
                    cab.TamanioConst = (cab.TamanioConst | aux) | tam;
                    tamanioseg += cab.TamanioConst;

                    aux = 0;
                    fread(&tam,sizeof(uint16_t),1,arch);
                    aux = (tam>>8) & 0xFF;
                    tam = (tam<<8) & 0xFF00;
                    cab.OffsetEntry = 0;
                    cab.OffsetEntry = (cab.OffsetEntry | aux) | tam;

                    //--CARGA DE LA TABLA DE SEGMENTOS COMPLETA--
                    if (tamanioseg <= comp->tamanio){

                        if (comp->tabladesegmentos[0][1]>0)
                            ultam = comp->tabladesegmentos[0][1];

                        if (cab.TamanioConst>0){
                            setBaseKS(comp,ultam);
                            setTamanioKS(comp,cab.TamanioConst);
                            ultam += cab.TamanioConst;
                        }

                        setBaseCS(comp,ultam);
                        setTamanioCS(comp,cab.TamanioCodigo);
                        ultam = cab.TamanioCodigo;

                        if(cab.TamanioData>0){
                            setBaseDS(comp,ultam);
                            setTamanioDS(comp,cab.TamanioData);
                            ultam += cab.TamanioData;
                        }

                        if (cab.TamanioExtra>0){
                            setBaseES(comp,ultam);
                            setTamanioES(comp,cab.TamanioExtra);
                            ultam += cab.TamanioExtra;
                        }
                        if (cab.TamanioStack>0){
                            setBaseSS(comp,ultam);
                            setTamanioSS(comp,cab.TamanioStack);
                        }

                        comp->registros[5] = 0x00020000 | cab.OffsetEntry;
                        comp->registros[6] = 0x00050000 | comp->tabladesegmentos[5][1]; //pone al sp al tope de la pila

                        dir =  0x00020000;
                        TradLogicaFisica(&dir,*comp,&flag);
                        while(fread(&lect,sizeof(uint8_t),1,arch)>0 && flag){
                            printf("lectura del archivo %x\n",lect);
                            comp->memoria[dir] = lect;
                            dir +=1;
                        }
                    }
                    else
                        comp->error = 5; //memoria insuficiente

                }
            }
            else{ //if el archivo es vmi
                comp->tamanio = cab.TamanioCodigo*1024;
                tamtot = 0;
                for(int j=0; j<16; j++){
                    fread(&lect4,4,1,arch);
                    comp->registros[j] = DarVuelta(lect4);
                }
                for (int j=0; j<6; j++){
                    fread(&lect4,4,1,arch);
                    tamanioseg = DarVuelta(lect4);
                    base = tamanioseg;
                    tam = tamanioseg<<16;
                    tamtot += tam;
                    comp->tabladesegmentos[j][0] = base;
                    comp->tabladesegmentos[j][1] = tam;
                }
                if(tamtot<=comp->tamanio){
                    for(int j=0;j<2;j++) //lee los 2 valores que sobran de la tabla de segmentos
                        fread(&lect4,4,1,arch);
                    i=0;
                    while(fread(&lect,sizeof(uint8_t),1,arch)>0){ //se supone que lee exactamente lo que dice la cabecera (por lo tanto no se cae del segmento de codigo). Preguntar si esta bien en clase
                        (*comp).memoria[i] = lect;
                        i++;
                    }
                }
                else
                    (*comp).error = 5;
            }
        }
        else{
            printf("No es un archivo valido\n");
            (*comp).error = 4;
        }
        fclose(arch);
    }
}

void IniciaComponentes( Componentes *comp ){

	(*comp).registros[0] = 0x00020000 ;
	(*comp).error = 0;
	(*comp).tamanio = 16384;
	comp->sigue_breakpoint = 0;
	for(int i=0; i<FIL; i++)
        for (int j=0; j<COL; j++)
            comp->tabladesegmentos[i][j] = 0;

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

    int fin, inicio , flag, cant_mueve,fin_KS ; // ------------------------> cant_mueve cantidad quese tiene que mover el IP / inicio : Es de donde empieza el IP si de KS o directamente del CS
    int8_t instruccion;
    TDatos abc;

    if (comp.tabladesegmentos[1][0] != 0)//--------------------------------> Es el KS
    {
    	inicio = comp.registros[KS]; //-------------------------------------> Comienzo desde el Constant segment
        fin_KS = comp.tabladesegmentos [1][1] + comp.tabladesegmentos[1][0];

    }else{
        inicio = comp.registros[CS]; // -------------------------------------> Comienza desde el Code Segment
        //fin_KS = comp.registros[CS];
    }
    TradLogicaFisica(&fin_KS, comp, &flag);
    TradLogicaFisica(&inicio,comp,&flag);
    fin = comp.tabladesegmentos[2][0] + comp.tabladesegmentos[2][1] + 0x00020000; //--> El final va a ser el mismo, en el CS
    TradLogicaFisica(&fin, comp, &flag);
    fin_KS = inicio;


    if(flag)
    {

        while(inicio < fin )
        {
            if(inicio >= fin_KS ) //-------------------------------------> fin_KS es el límite de KS, Entra al CS
            {
                if(inicio == fin_KS)
                    printf(">");
                instruccion = comp.memoria[inicio];
                abc = obtener_abc(instruccion);
       		}else
                abc.OpA = abc.OpB = abc.CodOperacion = 0x00; // --------> Entra al KS

            Disassembler(comp,abc,inicio,&cant_mueve);
        	inicio += cant_mueve; // -----------------------------------> cant_mueve cantidad quese tiene que mover el IP
        }

   	 }else

        comp.error = 3 ; // Error por disassembler -------------------------> SI todo va bien no debería saltar nunca; :)
/*
    while(comp.registros[5] < fin )
    {

        instruccion = comp.memoria[comp.registros[5]];
        abc = obtener_abc(instruccion);
        Disassembler(comp,abc,comp.registros[5]);
        comp.registros[5] += abc.OpA + abc.OpB +1;

    }
*/
}

void CargaRegistros(Componentes *comp){
    if (comp->tabladesegmentos[1][1]>0)
        comp->registros[4] = 0x00010000;
    else
        comp->registros[4] = -1;
    if(comp->tabladesegmentos[3][1]>0)
        comp->registros[1] = 0x00030000;
    else
        comp->registros[1] = -1;
    if(comp->tabladesegmentos[4][1]>0)
        comp->registros[2] = 0x00040000;
    else{
        comp->registros[2] = -1;
    }

        comp->registros[3] = 0x00050000;

        comp->registros[SP] = 0x00050000 + comp->tabladesegmentos[5][1];
}

int DarVuelta(int valor){
    int var1=0, var2=0, var3=0, var4=0, valorfinal=0;

    var1 = (valor & 0x000000FF)<<24;
    var2 = ((valor>>8) & 0x000000FF)<<16;
    var3 = ((valor>>16) & 0x000000FF)<<8;
    var4 = (valor>>24) & 0x000000FF;

    valorfinal = valorfinal | var1 | var2 | var3 | var4;

    return valorfinal;
}
