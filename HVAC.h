 // FileName:        HVAC.h
 // Dependencies:    None.
 // Processor:       MSP432
 // Board:           MSP432P401R
 // Program version: CCS V8.3 TI
 // Company:         Texas Instruments
 // Description:     Incluye librerías, define ciertas macros y significados así como llevar un control de versiones.
 // Authors:         José Luis Chacón M. y Jesús Alejandro Navarro Acosta.
 // Updated:         11/2018

#ifndef _hvac_h_
#define _hvac_h_

#pragma once

#define __MSP432P401R__
#define  __SYSTEM_CLOCK    48000000 // Frecuencias funcionales recomendadas: 12, 24 y 48 Mhz.
#define GIT "Hola Git"
/* Archivos de cabecera importantes. */
#include <unistd.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Archivos de cabecera POSIX. */
#include <pthread.h>
#include <semaphore.h>
#include <ti/posix/tirtos/_pthread.h>
#include <ti/sysbios/knl/Task.h>

/* Archivos de cabecera para RTOS. */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Event.h>

/* Board Support Package. */
#include "Drivers/BSP.h"

//Variables para las opciones del menú

enum MENU        //Se asignan valores del 0 al 3 para las variables
{
    DEFAULT,
    P1_SELECTED,
    P2_SELECTED,
    SL_SELECTED,
};

enum UP_DOWN{  //Variables para las persianas y la secuencia de LEDs
    off ,
    on,
};

struct Estado_PSL
{
    uint8_t  Estado;
}   Persiana1,Persiana2,SecuenciaLED;

uint8_t Select_Menu;

// Definiciones Básicas.
#define ENTRADA 1
#define SALIDA 0

// Re-definición de los bits y puertos de entrada a utilizar.
#define ON_OFF B1
#define MENU_BTN B4
#define UP_BTN B3
#define DOWN_BTN B4

#define ON_OFF_PORT 1
#define MENU_PORT 1
#define UP_DOWN_PORT 2

//Potenciometros
#define POT1 CH8
#define POT2 CH9
#define POT3 CH10

// Re-definición de los bits y puertos de salida a utilizar.
#define LED_Rojo BSP_LED2
#define LED_Verde BSP_LED3
#define LED_Azul BSP_LED4
#define LED_RGB_PORT BSP_LED3_PORT

// Definiciones del estado 'normal' de los botones externos a la tarjeta (solo hay dos botones).
#define GND 0
#define VCC 1
#define NORMAL_STATE_EXTRA_BUTTONS GND  // Aqui se coloca GND o VCC.

// Definiciones del sistema.
#define MAX_MSG_SIZE 64
#define MAX_ADC_VALUE 16383             // (2 ^14 bits) es la resolución default.
#define MAIN_UART (uint32_t)(EUSCI_A0)
#define DELAY 20000
#define ITERATIONS_TO_PRINT 49

//Encendido y apagado del sistema
uint8_t Enc_Apg;
uint32_t contadorApg;
#define ENCENDIDO 1
#define APAGADO 0

// Definición para el RTOS.
#define THREADSTACKSIZE1 1500

/* Funciones. */

/* Función de interrupción para botones de setpoint. */
extern void INT_SWI(void);
extern void INT_UP_DOWN(void);

/* Funciones de inicialización. */
extern void HVAC_InicialiceIO   (void);
extern void HVAC_InicialiceADC  (void);
extern void HVAC_InicialiceUART (void);
extern void System_InicialiceTIMER (void); // ESTO LO AÑADI PARA UTILIZAR EL "TIMER32"

/* Funciones principales. */
extern void HVAC_ActualizarEntradas(void);
extern void HVAC_PrintState(void);
extern void HVAC_Enc_Apg_Check(void);
extern void HVAC_Heartbeat(void);

// Funciones para controlar el sistema
extern void HVAC_Enc_Apag_Ctrl(void);
extern void HVAC_Menu(void);

#endif
