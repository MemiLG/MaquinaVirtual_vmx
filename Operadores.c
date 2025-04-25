#include <stdio.h>
#include <stdlib.h>
#include "Operadores.h"
#include "TipoMaquina.h"
#include <stdint.h> //para usar int8_t

//Registros
#define CS 0
#define DS 1
#define IP 5
#define CC 8
#define AC 9
#define EAX 10
#define EBX 11
#define ECX 12
#define EDX 13
#define EEX 14
#define EFX 15

//--------------------Funciones extras--------------------
void ValorOperando(Toperando op, int *aux, Componentes *comp)
{
    int8_t segmento;
    int pos, pos2, fl=1;

	switch(op.tipo){ //sacar el dato del operando
		case 0b01:
		    segmento = (op.operando>>2) & 0x3; //caso 01: registro
			pos = (op.operando>>4)& 0xF;
			*aux = comp->registros[pos];// no entra en ningin if: registro completo

			if(segmento == 0b01) //toma el 4to byte AL
				*aux = (*aux & 0xFF);
			else if (segmento == 0b10) //toma el 3er byte AH
				*aux = (*aux>>8)& 0xFF;
			else if (segmento == 0b11) //registro de 2 bytes (dos ultimos bytes)
				*aux = *aux & 0xFFFF;
			break;

		case 0b10:
		    *aux = op.operando;
            break;

		case 0b11:
		    pos = (op.operando>>4)& 0xF;
            pos2 = comp->registros[pos];
			pos2 += (op.operando>>8)& 0xFFFF;
			TradLogicaFisica(&pos2, *comp,&fl);
			if (fl){
                *aux = LeerMemoria(*comp, pos2, 4);
			}
            else{
                comp->error=3; //valor de corte por la flag. Caida de segmento
                *aux = 0;
            }
			break;
	}
}

void asignaValor(Toperando a, int ValorB, Componentes *comp)
{
    int dir, flag=1;
    int8_t CodReg, SecReg;

    switch(a.tipo)
    {
        //De registro
        case 1:  CodReg = a.operando >> 4 & 0xF;
                 SecReg = a.operando >> 2 & 0x3;

                 switch(SecReg)
                 {
                    //EAX (los 4 bytes)
                    case 0: (*comp).registros[CodReg] = ValorB;
                    break;

                    //AL (4to byte)
                    case 1: (*comp).registros[CodReg] = (*comp).registros[CodReg] & 0xFFFFFF00 ^ ValorB & 0xFF;
                    break;

                    //AH (3er byte)
                    case 2: (*comp).registros[CodReg] = (*comp).registros[CodReg] & 0xFFFF00FF ^ (ValorB & 0xFF)<<8;
                    break;

                    //AX (2 bytes)
                    case 3: (*comp).registros[CodReg] = (*comp).registros[CodReg] & 0xFFFF0000 ^ ValorB & 0xFFFF;
                    break;
                 }
        break;

        //Memoria
        case 3: CodReg = a.operando >> 4 & 0xF;
                dir = (*comp).registros[CodReg]; //offset del registro
                dir += a.operando >> 16 & 0xFFFF; //Le sumo el offset del operando
                TradLogicaFisica(&dir, *comp, &flag);
                if(flag)
                    InsertaMemoria(comp, dir, ValorB, 4);
        break;
    }
}

void leer(Componentes *comp)
{
    int i, cant_celdas, num, dir, tamanio, no_error;

    cant_celdas = (*comp).registros[ECX] & 0xFF; //CL
    tamanio = (*comp).registros[ECX] >> 8 & 0xFF; //CH
    dir = (*comp).registros[EDX];
    TradLogicaFisica(&dir, *comp, &no_error);

    if(no_error)
        for(i=0; i<cant_celdas; i++)
        {
            scanf("%d\n", &num);
            InsertaMemoria(comp, dir, num, tamanio);
            dir += tamanio;
        }
    else
        comp->error = 3;
}

void Imprime(Componentes *comp)
{

    int ind,i, no_error,nro, cociente, nro_aux, j=-1;
    int16_t cantidad_cl, tamanio_ch,formato;
    int8_t resto;
    char nro_binario[33];

    ind = (*comp).registros[EDX] ;
    printf("Indice : %x\n",ind);
    TradLogicaFisica(&ind , *comp , &no_error) ;
    if(no_error == 1 )
    {

        cantidad_cl = (*comp).registros[ECX] & 0xFF ;
        tamanio_ch =( (*comp).registros[ECX] >> 8 ) & 0xFF ;
        formato = (*comp).registros[EAX] & 0xFF ;

        printf("[%04x] : ",ind);

        for( i=0 ; i < cantidad_cl ; i++)
        {

            nro= LeerMemoria(*comp, ind , tamanio_ch);//Devuelve numero de 32 bits
            if((formato & 0x01) == 0x01 ) //Decimal

                printf("%d\t", nro);

            if((formato & 0X02) == 0x02) //Caracteres

                printf("%c\t", nro);

            if((formato & 0x04) == 0x04) //Octal

                printf("%o\t", nro);

            if((formato & 0x08) == 0x08) //Hexadecimal

                printf("%X\t", nro);

            if ((formato & 0x10) == 0x10) //Binario
            {
                cociente = nro / 2;
                resto = nro & 2;

                while(cociente != 1)
                {
                    if(resto)
                        nro_binario[++j] = '1';
                    else
                        nro_binario[++j] = '0';
                    nro_aux = cociente;
                    cociente = nro_aux / 2;
                    resto = nro_aux & 2;
                }

                if(resto)
                    nro_binario[++j] = '1';
                else
                    nro_binario[++j] = '0';

                printf("%s\t", nro_binario);
            }

            ind += tamanio_ch;
            printf("\n");
        }
    }
    else

        (*comp).error = 3;

}

//---------------------Dos operandos----------------------
void MOV(Toperando a, Toperando b, Componentes *comp)
{
    int ValorB;

    printf("MOV POS 0 DE MEMORIA: %x\n",comp->memoria[0]);
    ValorOperando(b, &ValorB, comp);

    printf("\t\tVALOR B: %x\n",ValorB);

    if ((*comp).error == 0)
        asignaValor(a, ValorB, comp);
    printf("MOV POS 0 DE MEMORIA DESPUES: %x\n",comp->memoria[0]);
}

void ADD(Toperando a, Toperando b, Componentes *comp)
{
    int ValorA, ValorB, res;

    ValorOperando(a, &ValorB, comp);
    ValorOperando(b, &ValorA, comp);

    if ((*comp).error == 0)
    {
        res = ValorA + ValorB;
        asignaValor(a, res, comp);
        modificaCC(comp, res);
    }
}

void SUB(Toperando a, Toperando b, Componentes *comp)
{
    int ValorA, ValorB, res;

    ValorOperando(a, &ValorB, comp);
    ValorOperando(b, &ValorA, comp);

    if ((*comp).error == 0)
    {
        res = ValorA - ValorB;
        asignaValor(a, res, comp);
        modificaCC(comp, res);
    }
}

void SWAP(Toperando a, Toperando b, Componentes *comp)
{
    int ValorA, ValorB;

    if((a.tipo==1 && b.tipo==1) || (a.tipo==3 && b.tipo==3)) //Ambos operandos de registro o memoria
    {
        ValorOperando(a, &ValorB, comp);
        ValorOperando(b, &ValorA, comp);

        if ((*comp).error == 0)
        {
            asignaValor(a, ValorB, comp);
            asignaValor(b, ValorA, comp);
        }
    }
}

void MUL(Toperando a, Toperando b, Componentes *comp)
{
    int ValorA, ValorB, res;

    ValorOperando(a, &ValorB, comp);
    ValorOperando(b, &ValorA, comp);

    if ((*comp).error == 0)
    {
        res = ValorA * ValorB;
        asignaValor(a, res, comp);
        modificaCC(comp, res);
    }
}

void DIV(Toperando a, Toperando b, Componentes *comp)
{
    int ValorA, ValorB, res;

    ValorOperando(a, &ValorB, comp);
    ValorOperando(b, &ValorA, comp);

    if ((*comp).error == 0){
        if (ValorB == 0)
            (*comp).error = 2;
        else
        {
            (*comp).registros[9] = ValorA % ValorB; //Guarda el resto en AC
            res = ValorA / ValorB;
            asignaValor(a, res, comp);
            modificaCC(comp, res);
        }
    }
}

void CMP(Toperando a, Toperando b, Componentes *comp)
{
    int ValorA, ValorB, res;

    ValorOperando(a, &ValorB, comp);
    ValorOperando(b, &ValorA, comp);

    if ((*comp).error == 0)
    {
        res = ValorA - ValorB;
        modificaCC(comp, res);
    }
}

void SHL(Toperando a, Toperando b, Componentes *comp)
{
    int ValorA, ValorB, res;

    ValorOperando(a, &ValorB, comp);
    ValorOperando(b, &ValorA, comp);

    if ((*comp).error == 0)
    {
        res = ValorA << ValorB;
        asignaValor(a, res, comp);
        modificaCC(comp, res);
    }
}

void SHR(Toperando a, Toperando b, Componentes *comp)
{
    int ValorA, ValorB, res;

    ValorOperando(a, &ValorB, comp);
    ValorOperando(b, &ValorA, comp);

    if ((*comp).error == 0)
    {
        res = ValorA >> ValorB;
        asignaValor(a, res, comp);
        modificaCC(comp, res);
    }
}

void AND(Toperando a, Toperando b, Componentes *comp)
{
    int ValorA, ValorB, res;

    ValorOperando(a, &ValorB, comp);
    ValorOperando(b, &ValorA, comp);

    if (comp->error == 0)
    {
        res = ValorA & ValorB;
        asignaValor(a, res, comp);
        modificaCC(comp, res);
    }
}

void OR(Toperando a, Toperando b, Componentes *comp)
{
    int ValorA, ValorB, res;

    ValorOperando(a, &ValorB, comp);
    ValorOperando(b, &ValorA, comp);

    if ((*comp).error == 0)
    {
        res = ValorA | ValorB;
        asignaValor(a, res, comp);
        modificaCC(comp, res);
    }
}

void XOR(Toperando a, Toperando b, Componentes *comp)
{
    int ValorA, ValorB, res;

    ValorOperando(a, &ValorB, comp);
    ValorOperando(b, &ValorA, comp);

    if ((*comp).error == 0)
    {
        res = ValorA ^ ValorB;
        asignaValor(a, res, comp);
        modificaCC(comp, res);
    }
}

void LDL(Toperando a, Toperando b, Componentes *comp)
{
    int ValorA, ValorB, res;

    ValorOperando(a, &ValorB, comp);
    ValorOperando(b, &ValorA, comp);

    if ((*comp).error == 0)
    {
        res = ValorA & 0xFFFF0000 | ValorB & 0xFFFF;
        asignaValor(a, res, comp);
    }
}

void LDH(Toperando a, Toperando b, Componentes *comp)
{
    int ValorA, ValorB, res;

    ValorOperando(a, &ValorB, comp);
    ValorOperando(b, &ValorA, comp);

    if ((*comp).error == 0)
    {
        res = ValorA & 0xFFFF | ValorB << 16;
        asignaValor(a, res, comp);
    }
}

void RND(Toperando a, Toperando b, Componentes *comp)
{
    int ValorB, res;

    ValorOperando(a, &ValorB, comp);

    if (comp->error == 0)
    {
        res = rand() % (ValorB + 1);
        asignaValor(a, res, comp);
    }
}

//----------------------Un operando-----------------------
void SYS(Toperando op, Componentes *comp)
{
    if(op.operando==1)
        leer(comp);
    else //Escribe
        Imprime(comp);
}

void JMP(Toperando offset, Componentes *comp)
{
    (*comp).registros[IP] = offset.operando & 0xFFFF;
}

void JZ(Toperando offset, Componentes *comp)
{
    if((*comp).registros[CC] >> 30 & 0x1)
        JMP(offset, comp);
}

void JP(Toperando offset, Componentes *comp)
{
    if(((*comp).registros[CC] >> 30 & 0x3) == 0)
        JMP(offset, comp);
}

void JN(Toperando offset, Componentes *comp)
{
    if((*comp).registros[CC] >> 31 & 0x1)
        JMP(offset, comp);
}

void JNZ(Toperando offset, Componentes *comp)
{
    if(((*comp).registros[CC] >> 30 & 0x1) == 0)
        JMP(offset, comp);
}

void JNP(Toperando offset, Componentes *comp)
{
    if((*comp).registros[CC] >> 31 & 0x1)
        JMP(offset, comp);
}

void JNN(Toperando offset, Componentes *comp)
{
    if(((*comp).registros[CC] >> 31 & 0x1) == 0)
        JMP(offset, comp);
}

void NOT(Toperando a, Componentes *comp)
{
    int valor;

    ValorOperando(a, &valor, comp);

    if ((*comp).error == 0)
    {
        valor = ~valor;
        asignaValor(a, valor, comp);
        modificaCC(comp, valor);
    }
}

void STOP(Componentes *comp)
{
    if ((*comp).error==0)
        (*comp).error = 4;
}

