#include <stdio.h>
#include <stdlib.h>
#include "TipoMaquina.h"
#define TOTAL 31

int DireccionFisicaValida(int dir, int16_t fila, Componentes comp)
{
    return (dir<=comp.tabladesegmentos[fila][1]-1 && dir>=comp.tabladesegmentos[fila][0]);
}

void TradLogicaFisica(int *dir, Componentes comp, int *flag)
{
    int aux;
    int16_t fila;

    *flag = 1;
    fila = *dir >> 16 & 0xFFFF;
    aux = (*dir & 0xFFFF);
    aux = comp.tabladesegmentos[fila][0] + aux;
    if (fila<FIL && DireccionFisicaValida(aux,fila,comp))
        *dir = aux;
    else
        *flag = 0;
}

int LeerMemoria(Componentes comp, int dir, int bytes)
{
    int result=0, i, aux=0, des;

    for(i=0; i<bytes; i++)
    {
        result = result<<8;
        aux = comp.memoria[dir+i];
        result |= aux;
    }

    if (comp.registros[5]>>16){ //Entra si esta en el ds (cambiar en la segunda parte de la mv)
        des = (4-bytes)*8;
        result = (result << des) >> des;
    }
    return result;
}

void modificaCC(Componentes *comp, int num)
{
    (*comp).registros[8] = (*comp).registros[8] & 0x3FFFFFFF;
    if(num < 0)
        (*comp).registros[8] = (*comp).registros[8] | 0x80000000;
    else
        if(num == 0)
            (*comp).registros[8] = (*comp).registros[8] | 0x40000000;
}

void InsertaMemoria(Componentes *comp,int dir,int dato, int byte)
{
int i, aux = 0;
    for (i=0;i<byte;i++){
        aux = (dato>>(24-i*8)) & 0xFF;
        (*comp).memoria[dir+i] = aux;
    }
}

//-------------------- Disassembler --------------------

void Disassembler(Componentes comp,TDatos abc, int i)
{

    stringg Operaciones[TOTAL] = {"SYS","JMP","JZ","JP","JN","JNZ","JNP","JNN","NOT","","","","","","","STOP","MOV","ADD","SUB","SWAP","MUL","DIV","CMP","SHL","SHR","AND","OR","XOR","LDL","LDH","RND"};
    int32_t auxb,auxa;
    int ind=i;

    printf("[%04X] %02X ",i,comp.memoria[i]);
    auxa=auxb=0X0;
    Op_AB(abc.OpB,comp,&auxb,&ind);
    Op_AB(abc.OpA,comp,&auxa,&ind);

    for(int u =0;u< 10 - (abc.OpA+abc.OpB) ;u++)

        printf("    ");

    printf(" | ");

    printf(" %s ",Operaciones[abc.CodOperacion]);

    if(abc.OpA != 0x0)
    {
        Significado(abc.OpA,auxa);
        printf(", ");
    }

    Significado(abc.OpB,auxb);
    printf("\n");

}

void Op_AB(int8_t Op, Componentes comp, int32_t *aux,int *i)
{

   int finall,inicio=1;

   switch(Op){

        case 0b00 :

            finall = 0; // No entra nunca al ciclo for
            break;

        case 0b01 :     // Es registro

            finall = 1;
            break;

        case 0b10 :     // Es inmediato

            finall = 2;
            break;

        case 0b11 :     // Es memoria

            finall = 3;
            break;

   }

    while ( inicio <= finall ){

        (*i)++;
        *aux = *aux << 8;
        printf(" %02X ",comp.memoria[*i]);
        *aux =(*aux) | comp.memoria[*i];
        inicio ++;

   }

}

void Significado(int8_t op, int32_t auxiliar)
{

    int8_t aux1=0,aux2=0;
    stringg Registross[TOTALR] = {"CS","DS"," "," "," ","IP"," "," ","CC","AC","EAX","EBX","ECX","EDX","EEX","EFX"};

    switch(op){

        case 0b01:

            aux1 = (auxiliar >> 2) & 0x03;
            aux2 = (auxiliar >> 4) & 0x0F;
            if(aux1 == 0b00)                                        // Registro completo

                printf("%s",Registross[aux2]);

            else
                if (aux1 == 0b01)                                  // 1 byte de registro (XL)

                    printf("%cL",Registross[aux2][1]);

                else
                        if(aux1 == 0b10)

                            printf("%cH",Registross[aux2][1]);     // 1 byte de registro (XH)

                        else

                            printf("%cX",Registross[aux2][1]);     // 2 byte de registro (AX)
            break;

        case 0b10:                                                // Es un inmediato

            printf("%d",auxiliar);
            break;

        case 0b11:                                                // Es memoria

            aux1 = (auxiliar >> 4) & 0X0000000F ;
            aux2 = (auxiliar >> 8);
            if(aux2 > 0)

                printf("[%s + %d]",Registross[aux1],aux2);

            else

                printf("[%s]",Registross[aux1]);
            break;
    }

}

//-------------------- Tabla de segmentos --------------------

void setBasePS(Componentes *comp, uint16_t valor){
    (*comp).tabladesegmentos[0][0] = valor;
}
void setTamanioPS(Componentes *comp,uint16_t valor){
    (*comp).tabladesegmentos[0][1] = valor;
}
void setBaseKS(Componentes *comp, uint16_t valor){
    (*comp).tabladesegmentos[1][0] = valor;
}
void setTamanioKS(Componentes *comp,uint16_t valor){
    (*comp).tabladesegmentos[1][1] = valor;
}
void setBaseCS(Componentes *comp, uint16_t valor){
    (*comp).tabladesegmentos[2][0] = valor;
}
void setTamanioCS(Componentes *comp,uint16_t valor){
    (*comp).tabladesegmentos[2][1] = valor;
}
void setBaseDS (Componentes *comp, uint16_t valor){
    (*comp).tabladesegmentos[3][0] = valor;
}
void setTamanioDS(Componentes *comp, uint16_t valor){
    (*comp).tabladesegmentos[3][1] = valor;
}
void setBaseES(Componentes *comp, uint16_t valor){
    (*comp).tabladesegmentos[4][0] = valor;
}
void setTamanioES(Componentes *comp,uint16_t valor){
    (*comp).tabladesegmentos[4][1] = valor;
}
void setBaseSS(Componentes *comp, uint16_t valor){
    (*comp).tabladesegmentos[5][0] = valor;
}
void setTamanioSS(Componentes *comp,uint16_t valor){
    (*comp).tabladesegmentos[5][1] = valor;
}
