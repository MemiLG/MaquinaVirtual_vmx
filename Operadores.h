#ifndef OPERADORES_H_INCLUDED
#define OPERADORES_H_INCLUDED
#include "TipoMaquina.h"

//Registros
#define CS 0
#define DS 1
#define ES 2
#define SS 3
#define KS 4
#define IP 5
#define SP 6
#define BP 7
#define CC 8
#define AC 9
#define EAX 10
#define EBX 11
#define ECX 12
#define EDX 13
#define EEX 14
#define EFX 15

//--------Funciones extras--------
void ValorOperando(Toperando ,int * ,Componentes *);
void asignaValor(Toperando , int , Componentes *);
int32_t mascara(int16_t );
void Imprime(Componentes *);
void leer(Componentes *);
void GeneraImagen(Componentes);

//--OPERADORES CON DOS OPERANDOS--
void MOV (Toperando , Toperando , Componentes *);
void ADD (Toperando , Toperando , Componentes *);
void SUB (Toperando , Toperando , Componentes *);
void MUL (Toperando , Toperando , Componentes *);
void DIV (Toperando , Toperando , Componentes *);
void SWAP (Toperando , Toperando , Componentes *);
void CMP (Toperando , Toperando , Componentes *);
void AND (Toperando , Toperando , Componentes *);
void OR (Toperando , Toperando , Componentes *);
void XOR (Toperando , Toperando , Componentes *);
void SHL (Toperando , Toperando , Componentes *);
void SHR (Toperando , Toperando , Componentes *);
void LDH (Toperando , Toperando , Componentes *);
void LDL (Toperando , Toperando , Componentes *);
void RND (Toperando , Toperando , Componentes *);

//--OPERADORES CON UN OPERANDO--
void SYS (Toperando,Componentes *);
void JMP (Toperando , Componentes *);
void JZ (Toperando , Componentes *);
void JN (Toperando , Componentes *);
void JP (Toperando , Componentes *);
void JNZ (Toperando , Componentes *);
void JNP (Toperando , Componentes *);
void JNN (Toperando , Componentes *);
void NOT (Toperando , Componentes *);
void push (Toperando , Componentes *);
void pop (Toperando , Componentes *);
void call (Toperando , Componentes *);

//--OPERADORES SIN OPERANDOS--
void STOP(Componentes *);
void ret(Componentes *);
#endif // OPERADORES_H_INCLUDED
