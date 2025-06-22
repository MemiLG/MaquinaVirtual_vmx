#include <stdio.h>
#include <stdlib.h>
#include "TipoMaquina.h"
#define TOTAL 31

/*int DireccionFisicaValida(int dir, int fila, Componentes comp)
{
    int tamanio=0;

    tamanio = comp.tabladesegmentos[fila][0] + comp.tabladesegmentos[fila][1] + (fila << 16);
    return (dir<=tamanio && dir>=comp.tabladesegmentos[fila][0]);
}*/
int DireccionFisicaValida(int dir, int fila, Componentes comp){
    int cotasup=0;

    cotasup = comp.tabladesegmentos[fila][0] + comp.tabladesegmentos[fila][1];
    return (dir<=cotasup && dir>=comp.tabladesegmentos[fila][0]);
}

void TradLogicaFisica(int *dir, Componentes comp, int *flag)
{
    int aux,i,j;
    int16_t fila;

    *flag = 1;
    fila = *dir >> 16 & 0xFFFF;
    i=j=-1;
    do{
        j++;
        if(comp.tabladesegmentos[j][1]!=0)
            i++;
    }while((comp.tabladesegmentos[j][1]==0 || i!=fila) && j<5);
    aux = (*dir & 0xFFFF);
    aux += comp.tabladesegmentos[j][0];
    if (fila<FIL && DireccionFisicaValida(aux,j,comp))
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

void InsertaMemoria(Componentes *comp, int dir, int dato, int tamanio)
{
    int i, aux=0;

    for (i=0; i<tamanio; i++)
    {
        aux = dato >> ((tamanio-1)*8 - i*8) & 0xFF;
        (*comp).memoria[dir + i] = aux;
    }
}

//---------------------- Disassembler -------------------------

void Disassembler(Componentes comp,TDatos abc, int i,int* cant_mueve)
{

    stringg Operaciones[TOTAL] = {"SYS","JMP","JZ","JP","JN","JNZ","JNP","JNN","NOT","","","PUSH","POP","CALL","RET","STOP","MOV","ADD","SUB","SWAP","MUL","DIV","CMP","SHL","SHR","AND","OR","XOR","LDL","LDH","RND"};
    int32_t auxb,auxa;
    int ind=i , terminator = 0x00, ind_cad, cant_caracteres,limite_cadena, espacios=0;
    char cad[8];

    printf("[%04X]",ind); //comp.memoria[ind]

    if(abc.OpA == 0x00 &&  abc.OpB == 0x00 && abc.CodOperacion == 0x00 )//-----------------------------> Esta en el KS
    {

        ind_cad = 0;
        cant_caracteres = 0;
        while( comp.memoria[ind] != terminator )
        {
            cant_caracteres ++;
            if(cant_caracteres == 7)
            {
                espacios += 3;
                printf(" ..");
            }

            else
                if(cant_caracteres < 7)
                {
                    espacios += 4;
                    printf(" %02X ",comp.memoria[ind]);
                }

            if(comp.memoria[ind] > 31 && comp.memoria[ind] < 255 &&  comp.memoria[ind] != 127)

                cad[ind_cad] = comp.memoria[ind];

            else

                cad[ind_cad] = '.';

            ind_cad ++;
            ind++;
        }

        if(comp.memoria[ind] == terminator && cant_caracteres <= 6)

            printf("00");

        for(int u=0; u < 44-espacios; u++)

            printf(" ");

        printf(" | ");
        cad[ind_cad] = 0x00;
        printf("''%s''\n",cad);
        *cant_mueve = cant_caracteres+1;

    }else{ // ------------------------------------------------------------------------> // Esta en el CodeSegment

    	auxa=auxb=0X0;
    	printf(" %02X ",comp.memoria[ind]);
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
        *cant_mueve = abc.OpA + abc.OpB +1;

     }

}

void Op_AB(int8_t finall, Componentes comp, int32_t *aux,int *i)
{

   int inicio=1;

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

    int8_t aux1=0,aux2=0, aux3=0;
    stringg Registross[TOTALR] = {"CS","DS","ES","SS","KS","IP","SP","BP","CC","AC","EAX","EBX","ECX","EDX","EEX","EFX"};

    switch(op){

        case 0b01:

            aux1 = (auxiliar >> 2) & 0x03;
            aux2 = (auxiliar >> 4) & 0x0F;
            if(aux1 == 0b00)//------------------------------------> Registro completo

                printf("%s",Registross[aux2]);

            else
                if (aux1 == 0b01)//-------------------------------> 1 byte de registro (XL)

                    printf("%cL",Registross[aux2][1]);

                else
                        if(aux1 == 0b10)

                            printf("%cH",Registross[aux2][1]);//--> 1 byte de registro (XH)

                        else

                            printf("%cX",Registross[aux2][1]);//--> 2 byte de registro (AX)
            break;

        case 0b10: //---------------------------------------------> Es un inmediato

            auxiliar = auxiliar << 16 >> 16;
            printf("%d",auxiliar);
            break;

        case 0b11: // --------------------------------------------> Es memoria

            aux1 = auxiliar & 0X3 ; // (0011)
            if (aux1 == 0x0)

                printf("l");

            else
                if (aux1 == 0x02)

                    printf("w");

                else

                    if (aux1 == 0x03)

                        printf("b");

            aux1 = (auxiliar >> 4) & 0X0000000F ;
            aux2 = (auxiliar >> 8);
            if(aux2 > 0)

                printf("[%s + %d]",Registross[aux1],aux2);

            else

                if(aux2 < 0)

                    printf("[%s - %d]",Registross[aux1],aux2*-1);

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
