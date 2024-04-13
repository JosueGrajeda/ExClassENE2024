 // FileName:        HVAC_IO.c
 // Dependencies:    HVAC.h
 // Processor:       MSP432
 // Board:           MSP432P401R
 // Program version: CCS V8.3 TI
 // Company:         Texas Instruments
 // Description:     Funciones de control de HW a través de estados.
 // Authors:         José Luis Chacón M. y Jesús Alejandro Navarro Acosta.
 // Updated:         11/2018

#include "HVAC.h"

char state[MAX_MSG_SIZE];      // Cadena a imprimir.
bool toggle = 0;               // Toggle para el heartbeat.
uint32_t delay;                // Delay

//Variables para controlar el estado del sistema
bool Enc_Apg_Push = FALSE;
bool Menu_Push = FALSE;
bool UP_DOWN_Push = FALSE;

/* **** SE DECLARARON LAS VARIABLES Y FUNCIONES PARA REALIZAR EL DALAY CON EL TIMER ******** */
extern void Timer32_INT1 (void); // Función de interrupción.
extern void Delay_ms (uint32_t time); // Función de delay.
float pot[3];

/*FUNCTION******************************************************************************
*
* Function Name    : System_InicialiceTIMER
* Returned Value   : None.
* Comments         :
*    Controla los preparativos para poder usar TIMER32
*
*END***********************************************************************************/
void System_InicialiceTIMER (void)
{
    T32_Init1();
    Int_registerInterrupt(INT_T32_INT1, Timer32_INT1);
    Int_enableInterrupt(INT_T32_INT1);

}

/*******************************************************************************************/


/*FUNCTION***************************************************************************
*
* Function Name : HVAC_Enc_Apg_Ctrl
* Returned Value : None.
* Comments : Maneja el encendido y apagado del sistema.
*
*END*********************************************************************************/
void HVAC_Enc_Apg_Ctrl (void)
{
    //Si se pulsa el boton con el sistema apagado...
    if(Enc_Apg == APAGADO){
        Enc_Apg = ENCENDIDO; //Se enciende el sistema
        UART_putsf(MAIN_UART,"Sistema encendido\n\r"); //Se manda el mensaje a la terminal
        GPIO_setOutput(BSP_LED1_PORT, BSP_LED1, 1); //Enciende LED rojo
    }
    else if (Enc_Apg == ENCENDIDO){
        contadorApg = contadorApg + 1; //Al pulsar el botón se suma 1 al contador


        if(contadorApg == 0x02){ //Si el contador llega a 2, el sistema se apaga
            Enc_Apg = APAGADO; //Se apaga el sistema
            UART_putsf(MAIN_UART,"SISTEMA APAGADO\n\r");
            GPIO_setOutput(BSP_LED1_PORT, BSP_LED1, 0); //Apaga LED rojo
        }

        //Se manda mensaje para dar indicaciones si se desea apagar el sistema
        else if(contadorApg < 0x02){ //Esto sucede tras pulsar por primera ocasión el botón de apagado/encendido tras el inicio del sistema
            UART_putsf(MAIN_UART,"Presiona nuevamente para apagar el sistema\n\r");
        }
    }
    Enc_Apg_Push = TRUE; //Controla los segundos de "delay" de cada estado (ON/OFF)
}

/**********************************************************************************
 * Function: INT_SWI
 * Preconditions: Interrupción habilitada, registrada e inicialización de módulos.
 * Overview: Función que es llamada cuando se genera
 *           la interrupción del botón SW1 o SW2.
 * Input: None.
 * Output: None.
 **********************************************************************************/

void INT_SWI(void)
{
    GPIO_clear_interrupt_flag(P1,B1); // Limpia la bandera de la interrupcion.
    GPIO_clear_interrupt_flag(P1,B4); // Limpia la bandera de la interrupcion.

    if(!GPIO_getInputPinValue(ON_OFF_PORT,BIT(ON_OFF))) // Llama a la función de encendido y apagado al pulsar
                                                        //el botón de encendido/apagado
        HVAC_Enc_Apg_Ctrl();

    //Despliega el menú al pulsar el botón
    if(!GPIO_getInputPinValue(MENU_PORT,BIT(MENU_BTN))){

        UART_putsf(MAIN_UART, "0. DEFAULT\r\n");
        UART_putsf(MAIN_UART, "1. P1_SELECTED  STATUS   \r\n");
        UART_putsf(MAIN_UART, "2. P2_SELECTED STATUS  \r\n");
        UART_putsf(MAIN_UART, "3. SL_SELECTED STATUS \r\n");

        //Se aumenta en 1 la cuenta de iteraciones (inicia en 0)
        Select_Menu += 0x01; // Cambia la seleccion

    if(Select_Menu > 0x03) //Reinicia la cuenta al sobrepasar las 3 cuentas
        Select_Menu = 0x00;

    //Hace un bucle hasta que se suelta el botón
    while(!GPIO_getInputPinValue(MENU_PORT,BIT(MENU_BTN)));
        HVAC_Menu(); //Llama a la función
    }
    return;
}

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_InicialiceIO
* Returned Value   : None.
* Comments         :
*    Controla los preparativos para poder usar las entradas y salidas GPIO.
*
*END***********************************************************************************/
void HVAC_InicialiceIO(void)
{
    // Para entradas y salidas ya definidas en la tarjeta.
    GPIO_init_board();

    // Modo de interrupcion de los botones principales.
    GPIO_interruptEdgeSelect(ON_OFF_PORT,BIT(ON_OFF), GPIO_HIGH_TO_LOW_TRANSITION);
    GPIO_interruptEdgeSelect(MENU_PORT,BIT(MENU_BTN), GPIO_HIGH_TO_LOW_TRANSITION);
    GPIO_interruptEdgeSelect(UP_DOWN_PORT,BIT(UP_BTN), GPIO_LOW_TO_HIGH_TRANSITION);
    GPIO_interruptEdgeSelect(UP_DOWN_PORT,BIT(DOWN_BTN),GPIO_HIGH_TO_LOW_TRANSITION);

    // Preparativos de interrupcion.
    GPIO_clear_interrupt_flag(P1,B1);
    GPIO_clear_interrupt_flag(P1,B4);
    GPIO_clear_interrupt_flag(P2,B3);
    GPIO_clear_interrupt_flag(P2,B4);

    GPIO_enable_bit_interrupt(P1,B1);
    GPIO_enable_bit_interrupt(P1,B4);
    GPIO_enable_bit_interrupt(P2,B3);
    GPIO_enable_bit_interrupt(P2,B4);

    //Habilita el registro de interrupciones
    Int_registerInterrupt(INT_PORT1, INT_SWI);
    Int_registerInterrupt(INT_PORT2, INT_UP_DOWN);

    //Habilita el módulo interrupt para las interrupciones
    Int_enableInterrupt(INT_PORT1);
    Int_enableInterrupt(INT_PORT2);

}

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_InicialiceADC
* Returned Value   : None.
* Comments         :
*    Inicializa las configuraciones deseadas para
*    el módulo general ADC y canales.
*
*END***********************************************************************************/
void HVAC_InicialiceADC(void)
{
    // Iniciando ADC y canales a utilizar
    ADC_Initialize(ADC_14bitResolution, ADC_CLKDiv8);
    ADC_SetConvertionMode(ADC_SequenceOfChannels);
    ADC_ConfigurePinChannel(POT1, AN8, ADC_VCC_VSS); //Se toma en cuenta el pin 4.5 para iniciar la lectura
    ADC_ConfigurePinChannel(POT2, AN9 , ADC_VCC_VSS);
    ADC_ConfigurePinChannel(POT3, AN10 , ADC_VCC_VSS);
    ADC_SetStartOfSequenceChannel(AN8); //Lectura inicia en analogo 8
    ADC_SetEndOfSequenceChannel(AN10); //Lectura termina en analogo 9
}

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_InicialiceUART
* Returned Value   : None.
* Comments         :
*    Inicializa las configuraciones deseadas para
*    configurar el modulo UART (comunicación asíncrona).
*
*END***********************************************************************************/
void HVAC_InicialiceUART (void)
{
    UART_init();
}

/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_ActualizarEntradas
* Returned Value   : None.
* Comments         :
*    Actualiza variables indicadores de las entradas
*
*
*END***********************************************************************************/
void HVAC_ActualizarEntradas(void)
{
    ADC_trigger(); //Inicia la conversión analógico-digital
    while (ADC_is_busy()); //Bucle que espera a que la conversión esté completa
    //utilizando la fórmula (valor del ADC / Valor máximo del ADC) * valor máximo deseado
    pot[0] = (ADC_result(POT1) * 10) / MAX_ADC_VALUE;

    ADC_trigger();
    while (ADC_is_busy());
    pot[1] = (ADC_result(POT2) * 10) / MAX_ADC_VALUE;

    ADC_trigger();
    while (ADC_is_busy());
    pot[2] = (ADC_result(POT3) * 10) / MAX_ADC_VALUE;
}


/*FUNCTION******************************************************************************
* Function: INT_UP_DOWN
* Preconditions: Interrupcion habilitada, registrada e inicializacion de modulos.
* Overview: Funcion que es llamada cuando se genera la interrupcion del
* boton que controla las persianas y secuencia de LEDs
****************************************************************************************/
void INT_UP_DOWN (void)
{
    // Limpia las banderas de interrupción
    GPIO_clear_interrupt_flag(P2, B3);
    GPIO_clear_interrupt_flag(P2, B4);

    UART_putsf(MAIN_UART,"\n\r Interrupcion INTUPDOWN \n\r");


    // Verifica si se ha presionado el botón UP o el botón DOWN
    if (((GPIO_getInputPinValue(UP_DOWN_PORT, BIT(UP_BTN)) != NORMAL_STATE_EXTRA_BUTTONS))
        ||((GPIO_getInputPinValue(UP_DOWN_PORT, BIT(DOWN_BTN)) != NORMAL_STATE_EXTRA_BUTTONS))){
        //Utiliza la variable Select_Menu para determinar que acción realizar
        switch (Select_Menu) {

            //Actualiza el estado de la persiana 1 a "UP"
            case P1_SELECTED:
                if(GPIO_getInputPinValue(UP_DOWN_PORT, BIT(UP_BTN)) == 1){ //Verifica si se pulsó el botón
                    UART_putsf(MAIN_UART,"\n\r Opening P1... \n\r"); //Manda mensaje por terminal
                    Persiana1.Estado = on; //Actualiza el estado
                }
                else if (GPIO_getInputPinValue(UP_DOWN_PORT, BIT(DOWN_BTN)) == 1){ //Verifica si se pulsó el botón
                    Persiana1.Estado = off;
                    UART_putsf(MAIN_UART,"\n\r Closing P1... \n\r");
                    }
                break;


            case P2_SELECTED:
                if(GPIO_getInputPinValue(UP_DOWN_PORT, BIT(UP_BTN)) == 1){
                    UART_putsf(MAIN_UART,"\n\r Opening P2... \n\r");
                    Persiana2.Estado = on;
                }
                else if (GPIO_getInputPinValue(UP_DOWN_PORT, BIT(DOWN_BTN)) == 1){
                    Persiana2.Estado = off;
                    UART_putsf(MAIN_UART,"\n\r Closing P2... \n\r");
                    }
                break;

            case SL_SELECTED:
                if(GPIO_getInputPinValue(UP_DOWN_PORT, BIT(UP_BTN)) == 1){
                    UART_putsf(MAIN_UART,"\n\r Encendiendo secuencia... \n\r");
                    SecuenciaLED.Estado = on;
                }
                else if(GPIO_getInputPinValue(UP_DOWN_PORT, BIT(DOWN_BTN)) == 1){
                    SecuenciaLED.Estado = off;
                    UART_putsf(MAIN_UART,"\n\r Apagando secuencia... \n\r");
                    }
                break;

            default:
                UART_putsf(MAIN_UART,"\n\r Selecciona una opcion con el boton MENU \n\r");
        }
    }



    UP_DOWN_Push = TRUE;

    return;
}


/*FUNCTION******************************************************************************
*
* Function Name    : HVAC_PrintState
* Returned Value   : None.
* Comments         :
*    Imprime via UART la situación actual del sistema en términos de temperaturas
*    actual y deseada, estado del abanico, del sistema y estado de las entradas.
*    Imprime cada cierto número de iteraciones y justo despues de recibir un cambio
*    en las entradas, produciéndose un inicio en las iteraciones.
*END***********************************************************************************/
void HVAC_PrintState(void)
{
    static char iterations = 0;

    iterations++;
    if(iterations >= ITERATIONS_TO_PRINT) //Se toma en cuenta las 50 iteraciones
    {
        sprintf(state,"LED_1: %f LED_2: %f LED_3: %f \n\r",pot[0], pot[1], pot[2]);
        UART_putsf(MAIN_UART,state);

        sprintf (state, " P1: %s, P2: %s, SL: %s \n \r \n \r",
        ( Persiana1.Estado == on? " Up":"Down"),
        ( Persiana2.Estado == on? " Up":"Down"),
        ( SecuenciaLED.Estado == on? " ON":"OFF"));
        UART_putsf(MAIN_UART,state);
        iterations = 0;
        //Si se activa la secuencia...
        if(SecuenciaLED.Estado == on){
            //Realiza la secuencia de rojo y azul
            toggle ^= 1;
            GPIO_setOutput(LED_RGB_PORT, LED_Rojo, toggle);
            GPIO_setOutput(LED_RGB_PORT, LED_Azul, !toggle);
        }
        //Se mantienen apagados los LED si no se activa
        else{
            GPIO_setOutput(LED_RGB_PORT, LED_Rojo, 0);
            GPIO_setOutput(LED_RGB_PORT, LED_Azul, 0);
        }
    }
        usleep (DELAY); //Retraso
}


/*FUNCTION***************************************************************************
*
* Function Name : HVAC_Enc_Apg_Check
* Returned Value : None.
* Comments : Verifica el estado de la pulsacion del boton ON/OFF y controla
* el delay para el encendido y apagado del programa.
* Tambien controla el delay del menu
*
*END*********************************************************************************/
void HVAC_Enc_Apg_Check (void)
{
    //Se pulsa el botón de encendido/apagado o el de menú
    if (Enc_Apg_Push == TRUE || Menu_Push == TRUE){

        //Se inicia el contador en 0
        if(contadorApg == 0x00)
            Delay_ms(1000); //retardo 1 segundo

        //Ocurre cuando el sistema está encendido
        else if(contadorApg > 0x00)
            Delay_ms(5000); // retardo 5 segundos
    }
    else if (UP_DOWN_Push == TRUE){ //Se ejecuta el codigo si el botón de subir/bajar se presiona

        //Si la seleccion del menú no es la predeterminada o la correspondiente a SL
        //Se manda un mensaje de esperar y se da una espera de 5 segundos
        if (Select_Menu != DEFAULT && Select_Menu != 0x03){
            UART_putsf(MAIN_UART, "\r\n Espere 5 segundos ...");
            Delay_ms(4000);
        }
        Delay_ms(1000);
        HVAC_Menu(); //Actualiza el menú
    }
    // se restablecen a falso para indicar que no se ha producido ninguna pulsación de botón.
    UP_DOWN_Push = FALSE;
    Menu_Push = FALSE;
    Enc_Apg_Push = FALSE;
}


/*FUNCTION***************************************************************************
*
* Function Name : HVAC_Menu
* Returned Value : None.
* Comments : Imprime la seleccion actual del menu y su estado.
*
*END*********************************************************************************/
void HVAC_Menu (void){

    if (Enc_Apg != APAGADO){ //Condicional para no imprimir si el sistema está apagado
        switch (Select_Menu){ //Switch para cambiar en base a las iteraciones del botón menú

            case P1_SELECTED : sprintf (state," \n \rP1: %s \n \r \n \r",( Persiana1.Estado == on? "UP":"DOWN"));
                UART_putsf(MAIN_UART, state);
            break;

            case P2_SELECTED : sprintf (state, "\n \rP2: %s \n \r \n \r",(Persiana2.Estado == on? " UP":"DOWN"));
                UART_putsf(MAIN_UART, state);
            break;

            case SL_SELECTED : sprintf (state, "\n \rSL: %s \n \r \n \r",(SecuenciaLED.Estado == on? "ON":"OFF"));
                UART_putsf(MAIN_UART, state);
            break;

            default :  sprintf(state,"LED_1: %f LED_2: %f LED_3: %f \n\r",pot[0], pot[1], pot[2]);
            UART_putsf(MAIN_UART,state);
            sprintf (state, " P1: %s, P2: %s, SL: %s \n \r \n \r",
            ( Persiana1.Estado == on? " Up":"Down"),
            ( Persiana2.Estado == on? " Up":"Down"),
            ( SecuenciaLED.Estado == on? " ON":"OFF"));
            UART_putsf(MAIN_UART,state);
            break;

        }

        Menu_Push = TRUE;
    }

}


