#include <stdio.h>
#include <stdlib.h>
#include "Operadores.h"
#include "TipoMaquina.h"
#include <stdint.h> //para usar int8_t

//--------------------Funciones extras--------------------
void ValorOperando(Toperando op, int *aux, Componentes *comp)
{
    int8_t segmento, tamanio;
    int pos, pos2, fl=1, des=0;

	switch(op.tipo){ //sacar el dato del operando
		case 0b01:
		    segmento = (op.operando>>2) & 0x3; //caso 01: registro
			pos = (op.operando>>4)& 0xF;
			*aux = comp->registros[pos];// no entra en ningin if: registro completo

			if(segmento == 0b01){ //toma el 4to byte AL
				*aux = (*aux & 0xFF);
				des = 24;
			}
			else if (segmento == 0b10){ //toma el 3er byte AH
				*aux = (*aux>>8)& 0xFF;
				des = 16;
			}
			else if (segmento == 0b11){ //registro de 2 bytes (dos ultimos bytes)
				*aux = *aux & 0xFFFF;
				des = 16;
			}

            if (des>0){
                *aux = (*aux<<des)>>des;
            }
			break;

		case 0b10:
		    *aux = op.operando;
		    *aux = (*aux<<16)>>16;
            break;

		case 0b11:
		    pos = (op.operando>>4)& 0xF;
            pos2 = comp->registros[pos];
			pos2 += (op.operando>>8)& 0xFFFF;
			TradLogicaFisica(&pos2, *comp,&fl);
			if (fl){
                *aux = LeerMemoria(*comp, pos2, 4);
                tamanio = op.operando & 0x3; //0 = l = 4 bytes       2 = w = 2 bytes     3 = b = 1 byte
                switch(tamanio)
                {
                    case 2: *aux &= 0xFFFF;
                    break;

                    case 3: *aux &= 0xFF;
                    break;
                }
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
    int dir, flag;
    int8_t CodReg, SecReg, tamanio;

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
                dir = (*comp).registros[CodReg]; //puntero contenido por el registro
                dir += a.operando >> 8 & 0xFFFF; //Le sumo el offset del operando
                TradLogicaFisica(&dir, *comp, &flag);
                if(flag)
                {
                    tamanio = a.operando & 0x3; //0 = l = 4 bytes       2 = w = 2 bytes     3 = b = 1 byte
                    InsertaMemoria(comp, dir+tamanio, ValorB, 4-tamanio);
                }
                else
                    (*comp).error = 3;
        break;
    }
}

void propagar_signo(int *valor, Toperando op)
{
    int8_t des, byte, SecReg;

    switch(op.tipo)
    {
        //De registro
        case 1:  SecReg = op.operando >> 2 & 0x3;

                 switch(SecReg)
                 {
                    //EAX (los 4 bytes)
                    case 0: byte = 4;
                    break;

                    //AL (4to byte)
                    case 1: byte = 1;
                    break;

                    //AH (3er byte)
                    case 2: byte = 2;
                            *valor &= 0xFFFFFF00;
                    break;

                    //AX (2 bytes)
                    case 3: byte = 2;
                    break;
                 }
        break;

        //Inmediato
        case 2: byte = 2;

        //Memoria
        case 3: byte = 3;
    }

    des = (4-byte)*8;
    *valor = (*valor << des) >> des;
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
            scanf("%d", &num);
            InsertaMemoria(comp, dir, num, tamanio);
            dir += tamanio;
        }
    else
        comp->error = 3;
}

void imprime(Componentes *comp)
{

    int ind,i, no_error,nro, cociente, nro_aux, j=-1;
    int16_t cantidad_cl, tamanio_ch,formato;
    int8_t resto;
    char nro_binario[33];

    ind = (*comp).registros[EDX] ;
    TradLogicaFisica(&ind , *comp , &no_error) ;
    if(no_error == 1 )
    {

        cantidad_cl = (*comp).registros[ECX] & 0xFF ;
        tamanio_ch =( (*comp).registros[ECX] >> 8 ) & 0xFF ;
        formato = (*comp).registros[EAX] & 0xFF ;


        for( i=0 ; i < cantidad_cl ; i++)
        {

            printf("[%04X] : ",ind);
            nro= LeerMemoria(*comp, ind , tamanio_ch);//Devuelve numero de 32 bits
            if((formato & 0x01) == 0x01 ) //Decimal

                printf("%d\t", nro);

            if((formato & 0X02) == 0x02){ //Caracteres

                if(nro<32 || nro>255)

                    printf("....\t");
                else

                    printf("%c\t", nro);
            }

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

void string_read(Componentes *comp)
{

	int tamanio, cant_carmax, dir, no_error, cant_car=0, i=0; // cant_carmax : Cantidad de caracteres máximos ; dir : Desde donde se comienza a guardar la cadena
	char cadena[60], final_final = '\0';

	tamanio = 1;
	dir = (*comp).registros[EDX];
	TradLogicaFisica(&dir, *comp, &no_error);
	cant_carmax = (*comp).registros[ECX] & 0x00FF; // CX

	if(no_error)
	{

		scanf("%s",cadena);
		while(cant_car <= cant_carmax && cadena[i]!=final_final)
		{
			InsertaMemoria(comp, dir, cadena[i], tamanio);
			dir++;
			i++;
			cant_car++;
		}
		InsertaMemoria(comp, dir, final_final, tamanio);

	}else
		(*comp).error = 3;

}

void string_write(Componentes *comp)
{

	int i=0, dir, no_error, caracter; // cant_carmax : Cantidad de caracteres máximos ; dir : Desde donde se comienza a guardar la cadena
	char cadena[60], final_final = '\0', salto='\n';

	dir = (*comp).registros[EDX]; //-------------------------------------> Donde inicia la cadena
	TradLogicaFisica(&dir, *comp, &no_error);

	if(no_error)
	{

		caracter = LeerMemoria(*comp,dir,1);
		while(caracter != final_final)
		{

			cadena[i] = caracter;
			caracter = LeerMemoria(*comp,dir,1);
			dir++;
			i++;

		}
		cadena[i]=final_final;
		printf("%s",cadena);
		dir++;
		caracter = LeerMemoria(*comp,dir,1);
		if(caracter == salto)
			printf("\n");

	}else
		(*comp).error = 3;


}

void breakpoint(Componentes *comp)
{
    char accion;

    if(comp->img.booimagen) //Si existe el .vmi
    {
        GeneraImagen(*comp);
        scanf("%c", &accion);
        switch(accion)
        {
            //g = go
            case 103: comp->sigue_breakpoint = 0;
            break;

            //q = quit
            case 113: STOP(comp);
            break;

            //Enter
            case 10: comp->sigue_breakpoint = 1;
            break;
        }
    }
}

void GeneraImagen(Componentes comp)
{
    FILE *arch;
    char ident;
    uint8_t version;
    uint16_t tamanio;
    int tabla, tamem=0;

    arch = fopen(comp.img.nombre,"wb");
    if (arch == NULL)
        printf("No se pudo abrir el archivo\n");
    else{
        ident = 'V';
        fwrite(&ident,sizeof(char),1,arch);
        ident = 'M';
        fwrite(&ident,sizeof(char),1,arch);
        ident = 'I';
        fwrite(&ident,sizeof(char),1,arch);
        ident = '2';
        fwrite(&ident,sizeof(char),1,arch);
        ident = '5';
        fwrite(&ident,sizeof(char),1,arch);

        version = 1;
        fwrite(&version,sizeof(uint8_t),1,arch);

        tamanio = comp.tamanio;
        tamanio /= 1024;
        fwrite(&tamanio,sizeof(uint16_t),1,arch);

        for(int i=0;i<16;i++){
            fwrite(&(comp.registros[i]),sizeof(int),1,arch);
        }

        for(int i=0;i<6;i++){
            tabla = comp.tabladesegmentos[i][0]<<16;
            tabla |= comp.tabladesegmentos[i][1];
            tamem += comp.tabladesegmentos[i][1];
            fwrite(&tabla,sizeof(int),1,arch);
        }
        for(int i=0;i<2;i++)
            fwrite(&tabla,sizeof(int),1,arch);

        for(int i=0;i<tamem;i++){
            fwrite(&(comp.memoria[i]),sizeof(uint8_t),1,arch);
        }

        fclose(arch);
    }
}

//---------------------Dos operandos----------------------
void MOV(Toperando a, Toperando b, Componentes *comp)
{
    int ValorB;

    ValorOperando(b, &ValorB, comp);

    if ((*comp).error == 0)
        asignaValor(a, ValorB, comp);
}

void ADD(Toperando a, Toperando b, Componentes *comp)
{
    int ValorA, ValorB, res;

    ValorOperando(a, &ValorA, comp);
    ValorOperando(b, &ValorB, comp);

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

    ValorOperando(a, &ValorA, comp);
    ValorOperando(b, &ValorB, comp);

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
        ValorOperando(a, &ValorA, comp);
        ValorOperando(b, &ValorB, comp);

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

    ValorOperando(a, &ValorA, comp);
    ValorOperando(b, &ValorB, comp);

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

    ValorOperando(a, &ValorA, comp);
    ValorOperando(b, &ValorB, comp);

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

    ValorOperando(a, &ValorA, comp);
    ValorOperando(b, &ValorB, comp);

    if ((*comp).error == 0)
    {
        res = ValorA - ValorB;
        modificaCC(comp, res);
    }
}

void SHL(Toperando a, Toperando b, Componentes *comp)
{
    int ValorA, ValorB, res;

    ValorOperando(a, &ValorA, comp);
    ValorOperando(b, &ValorB, comp);

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

    ValorOperando(a, &ValorA, comp);
    ValorOperando(b, &ValorB, comp);;

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

    ValorOperando(a, &ValorA, comp);
    ValorOperando(b, &ValorB, comp);

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

    ValorOperando(a, &ValorA, comp);
    ValorOperando(b, &ValorB, comp);

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

    ValorOperando(a, &ValorA, comp);
    ValorOperando(b, &ValorB, comp);

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

    ValorOperando(a, &ValorA, comp);
    ValorOperando(b, &ValorB, comp);

    if ((*comp).error == 0)
    {
        res = ValorA & 0xFFFF0000 | ValorB & 0xFFFF;
        asignaValor(a, res, comp);
    }
}

void LDH(Toperando a, Toperando b, Componentes *comp)
{
    int ValorA, ValorB, res;

    ValorOperando(a, &ValorA, comp);
    ValorOperando(b, &ValorB, comp);

    if ((*comp).error == 0)
    {
        res = ValorA & 0xFFFF | ValorB << 16;
        asignaValor(a, res, comp);
    }
}

void RND(Toperando a, Toperando b, Componentes *comp)
{
    int ValorA, res;

    ValorOperando(a, &ValorA, comp);

    if (comp->error == 0)
    {
        res = rand() % (ValorA + 1);
        asignaValor(a, res, comp);
    }
}

//----------------------Un operando-----------------------
void SYS(Toperando op, Componentes *comp)
{
    switch(op.operando)
    {
        case 1: leer(comp);
        break;

        case 2: imprime(comp);
        break;

        case 3: string_read(comp);
        break;

        case 4: string_write(comp);
        break;

        case 7: system("cls");
        break;

        case 15: breakpoint(comp);
        break;
    }
}

void JMP(Toperando offset, Componentes *comp)
{
    (*comp).registros[IP] =  (*comp).registros[IP] & 0xFFFF0000 | offset.operando & 0xFFFF;
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
    if(((*comp).registros[CC] >> 31 & 0x1) == 0){
        JMP(offset, comp);
    }
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

void push(Toperando op, Componentes *comp)
{
    int valor, dir, flag;

    if(comp->registros[SP] - 4 < comp->registros[SS]) //Si no esta llena
        comp->error = 6;
    else
    {
        comp->registros[SP] -= 4;
        dir = comp->registros[SP];
        TradLogicaFisica(&dir, *comp, &flag);
        if(flag)
        {
            ValorOperando(op, &valor, comp);
            propagar_signo(&valor, op);
            InsertaMemoria(comp, dir, valor, 4);
        }
        else
            comp->error = 3;
    }
}

void pop(Toperando op, Componentes *comp)
{
    int op_aux, dir, flag, tamanio;

    tamanio = 0x00050000 + comp->tabladesegmentos[5][1];

    if(comp->registros[SP] > tamanio) //Si no esta vacia
        comp->error = 7;
    else
    {
        dir = comp->registros[SP];
        TradLogicaFisica(&dir, *comp, &flag);
        if(flag)
        {
            op_aux = (*comp).memoria[dir];
            asignaValor(op, op_aux, comp);
            comp->registros[SP] += 4;
        }
        else
            comp->error = 3;
    }
}

void call(Toperando op, Componentes *comp)
{
    int dir, flag;

    if(comp->registros[SP] - 4 < comp->registros[SS]) //Si no esta llena
        comp->error = 6;
    else
    {
        comp->registros[SP] -= 4;
        dir = comp->registros[SP];
        TradLogicaFisica(&dir, *comp, &flag);
        if(flag)
        {
            InsertaMemoria(comp, dir, comp->registros[IP], 4);
            JMP(op, comp);
        }
        else
            comp->error = 3;
    }
}

//----------------------Sin operando-----------------------
void STOP(Componentes *comp)
{
    if ((*comp).error==0)
        (*comp).error = 4;
}

void ret(Componentes *comp)
{
    int dir, flag, aux=0, i, tamanio;

    tamanio = 0x00050000 + comp->tabladesegmentos[5][1];

    printf("Error ret: %d", comp->error);
    printf("SP: %X\n", comp->registros[SP]);
    if(comp->registros[SP] > tamanio) //Si no esta vacia
        comp->error = 7;
    else
    {
        dir = comp->registros[SP];
        TradLogicaFisica(&dir, *comp, &flag);
        if(flag)
        {
            for(i=0; i<4; i++)
                aux |= comp->memoria[dir + i] << 24 - i*8;

            comp->registros[IP] = aux;
            comp->registros[SP] += 4;
        }
        else
            comp->error = 3;
    }
    printf("Error ret: %d", comp->error);
}
