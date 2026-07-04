/* ========================================
 * \file	main.c
 * \author	Dikshant Agrawal 	Hochschule Darmstadt - dikshant.agrawal@stud.h-da.de
 * \date 	27.10.2025
 * \version	1.0
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include "project.h"
#include "analyzer.h"
#include "global.h"
#include "stdio.h"
#include <math.h>

#define TestCase2

/*Just to check entry value (for personal use)*/
#ifdef TestCase0

    int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */
    int entryValue2 = 0;
    int entryValue1 = 0;

    TimingAnalyzer_Init(); //Initalize the analyzer and pheripherals
       
    TimingAnalyser_Instance_t analyzers[1];
    
    //Create struct Instance for Timing Analyzer for DWT with OUTPUT [Timer DWT_OUTPUT]
    TimingAnalyzer_Create(&analyzers[0], C_DWT_OUTPUT, LED_GREEN, "Timer DWT_OUTPUT", 111);
    char temp[100];
    entryValue1= TimingAnalyzer_CaptureEntry(&analyzers[0]);
    CyDelay(1);
    entryValue2 = TimingAnalyzer_CaptureEntry(&analyzers[0]);
    sprintf(temp, "entry1=%u, entry2=%u\n", entryValue1, entryValue2);
    UART_PutString(temp);

for(;;);
}

#endif

/*Basic Test case written for Lab 1A/1B */
#ifdef TestCase1
    
int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */
 
    TimingAnalyzer_Init(); //Initalize the analyzer and pheripherals
       
    TimingAnalyser_Instance_t analyzers[3];
    uint32_t Active_Analyzer[] = {0,1,2};
    uint32_t Active_Analyzer_size = sizeof(Active_Analyzer) / sizeof(Active_Analyzer[0]);;
    
    
    //Create struct Instance for Timing Analyzer for DWT with OUTPUT [Timer DWT_OUTPUT]
    TimingAnalyzer_Create(&analyzers[0], C_DWT_OUTPUT, LED_GREEN, "Timer DWT_OUTPUT", 111);
    TimingAnalyzer_Create(&analyzers[1], C_SYSTICK_OUTPUT, LED_RED, "Timer SYS_OUTPUT", 123);
    TimingAnalyzer_Create(&analyzers[2], C_OUTPUT, LED_YELLOW, "Timer OUTPUT", 223);
    
    TimingAnalyzer_Start(&analyzers[0]);
    //CyDelay(1000);
    TimingAnalyzer_Stop(&analyzers[0]);
    TimingAnalyzer_Print(&analyzers[0]);
    TimingAnalyzer_Start(&analyzers[1]);
    TimingAnalyzer_Stop(&analyzers[1]);
    TimingAnalyzer_PrintAll(analyzers, Active_Analyzer, Active_Analyzer_size);
    
    
    for(;;){}

}
#endif

/*Test Case 2.1 Basic Functionality start / stop and print works properly 
* pause / resume works as expected 
* multiple timers can be used in parallel without interfering 
*/
#ifdef TestCase2

int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */
    char temp[32];
    uint32_t Active_Analyzer[] = {0,1};
    uint32_t Active_Analyzer_size = sizeof(Active_Analyzer)/sizeof(Active_Analyzer[0]);
    TimingAnalyser_Instance_t analyzers[2];
    
    TimingAnalyzer_Init(); //Initalize the analyzer and pheripherals
    
    
    //Create struct Instance for Timing Analyzer for DWT with OUTPUT [Timer DWT_OUTPUT]
    RC_t rc = TimingAnalyzer_Create(&analyzers[0], C_DWT_OUTPUT, LED_GREEN, "Timer DWT_OUTPUT", 111);
    RC_t rc1 = TimingAnalyzer_Create(&analyzers[1], C_SYSTICK_OUTPUT, LED_RED, "Timer SYSTICK_OUTPUT", 222);
    //If Create is not initialized properly it should skip the timing analysis object
    //Can add more logics for error handeling using RC Codes in Main.c
    
    if(rc != RC_SUCCESS || rc1 != RC_SUCCESS){
        sprintf(temp, "Error initalizing");
        UART_PutString(temp);
    }

    /*
    * Test Case 2.1 Basic START STOP PAUSE RESUME
    * Write code in the main.c with CyDelay() to prove that: 
    * start / stop and print works properly
    * pause / resume works as expected 
    * multiple timers can be used in parallel without interfering
	*/
    
    TimingAnalyzer_Start(&analyzers[1]);
    TimingAnalyzer_Start(&analyzers[0]);
    CyDelay(3000);
    TimingAnalyzer_Stop(&analyzers[1]);
    TimingAnalyzer_Stop(&analyzers[0]);
    TimingAnalyzer_PrintAll(analyzers, Active_Analyzer, Active_Analyzer_size );
    
    
    TimingAnalyzer_Start(&analyzers[1]);
    TimingAnalyzer_Start(&analyzers[0]);
    CyDelay(1000);
    TimingAnalyzer_Pause(&analyzers[1]);
    TimingAnalyzer_Pause(&analyzers[0]);
    CyDelay(1000);
    TimingAnalyzer_Resume(&analyzers[1]);
    TimingAnalyzer_Resume(&analyzers[0]);
    CyDelay(2000);
    TimingAnalyzer_Stop(&analyzers[1]);
    TimingAnalyzer_Stop(&analyzers[0]);
    TimingAnalyzer_PrintAll(analyzers, Active_Analyzer, Active_Analyzer_size );
    
    
    
    for(;;){}

}

#endif


/*Test Case for 2.2 Mathematical Functions It will print line by line*/
#ifdef TestCase3

    int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */


    TimingAnalyzer_Init(); //Initalize the analyzer and pheripherals
       
    TimingAnalyser_Instance_t analyzers[1];
    
    //Create struct Instance for Timing Analyzer for DWT with OUTPUT [Timer DWT_OUTPUT]
    TimingAnalyzer_Create(&analyzers[0], C_DWT_OUTPUT, LED_GREEN, "Timer DWT_OUTPUT", 111);
    char temp[100];
        
    
    /*
    * the durations of the following calculations: 
    * 1000 integer additions 
    */
    
    CyDelay(1000);


    volatile int a = 1;
    volatile int a1 = 0;
    TimingAnalyzer_Start(&analyzers[0]);
    for(int i = 0 ; i<=1000 ; i++){
        a1 = a + i;
    }
    TimingAnalyzer_Stop(&analyzers[0]);
    TimingAnalyzer_Print(&analyzers[0]);
    sprintf(temp, "Value of Result of Addition: %d\n", a1);
    UART_PutString(temp);
    
    /*
    * the durations of the following calculations: 
    * 1000 integer multiplications
    */
    
    volatile int bm = 4;
    volatile int b1 = 0;
    TimingAnalyzer_Start(&analyzers[0]);
    for(int i = 0 ; i<=1000 ; i++){
        b1 = i * bm;
    }
    TimingAnalyzer_Stop(&analyzers[0]);
    TimingAnalyzer_Print(&analyzers[0]);
    sprintf(temp, "Value of Result of Multiplication: %d\n", b1);
    UART_PutString(temp);
    
    
    /*
    * the durations of thye following calculations: 
    * 1000 integer divisons
    */
    
    volatile int bd = 2;
    volatile int ad = 4;
    volatile int c1 = 0;
    TimingAnalyzer_Start(&analyzers[0]);
    for(int i = 0 ; i<=1000 ; i++){
        c1 = ad/ bd;
    }
    TimingAnalyzer_Stop(&analyzers[0]);
    TimingAnalyzer_Print(&analyzers[0]);
    sprintf(temp, "Value of Result of Division: %d\n", c1);
    UART_PutString(temp);
    
        
    /*
    * the durations of the following calculations:
    * 1000 float additions
    */
    
    volatile float32_t fd = 0;
    volatile float32_t af = 2.3123;
    volatile float32_t bf = 1.31231;
    TimingAnalyzer_Start(&analyzers[0]);
    for(int i = 0 ; i<=1000 ; i++){
        fd = af + bf;
    }
    TimingAnalyzer_Stop(&analyzers[0]);
    TimingAnalyzer_Print(&analyzers[0]);
    sprintf(temp, "Value of Result of Float Addition: %f\n", fd);
    UART_PutString(temp);
    
    /*
    * the durations of the following calculations:
    * 1000 float multiplications
    */
        
    volatile float32_t fe = 0;
    volatile float32_t afm = 2.3123;
    volatile float32_t bfm = 1.31231;
    TimingAnalyzer_Start(&analyzers[0]);
    for(int i = 0 ; i<=1000 ; i++){
        fe = afm * bfm;
    }
    TimingAnalyzer_Stop(&analyzers[0]);
    TimingAnalyzer_Print(&analyzers[0]);
    sprintf(temp, "Value of Result of Float Multiplication: %f\n", fe);
    UART_PutString(temp);
    
    /*
    * the durations of the following calculations:
    * 1000 float divisons
    */
    
    volatile float32_t ff = 0;
    volatile float32_t afd = 2.3123;
    volatile float32_t bfd = 1.31231;
    TimingAnalyzer_Start(&analyzers[0]);
    for(int i = 0 ; i<=1000 ; i++){
        ff = afd / bfd;
    }
    TimingAnalyzer_Stop(&analyzers[0]);
    TimingAnalyzer_Print(&analyzers[0]);
    sprintf(temp, "Value of Result of Float Division: %f\n", ff);
    UART_PutString(temp);
        
    /*
    * the durations of the following calculations:
    * 1000 Square root calculations
    */
    volatile float32_t fg = 0;
    volatile float32_t afsq = 2.00;
    TimingAnalyzer_Start(&analyzers[0]);
    for(int i = 0 ; i<=1000 ; i++){
        fg = sqrt(afsq);
    }
    TimingAnalyzer_Stop(&analyzers[0]);
    TimingAnalyzer_Print(&analyzers[0]);
    sprintf(temp, "Value of Result of SQRT: %f\n", fg);
    UART_PutString(temp);
    
        
    /*
    * the durations of the following calculations:
    * 1000 sin()
    */
    
    volatile float64_t fh = 0;
    volatile float64_t sina = 3.14159265/2.00;
    TimingAnalyzer_Start(&analyzers[0]);
    for(int i = 0 ; i<=1000 ; i++){
        fh = sin(sina);
    }
    TimingAnalyzer_Stop(&analyzers[0]);
    TimingAnalyzer_Print(&analyzers[0]);
    sprintf(temp, "Value of Result of SIN: %f\n", fh);
    UART_PutString(temp);
    
        
    /*
    * the durations of the following calculations:
    * 1000 sinf() 
    */
    
    volatile float64_t fi = 0;
    volatile float64_t afsf = 3.14;
    TimingAnalyzer_Start(&analyzers[0]);
    for(int i = 0 ; i<=1000 ; i++){
        fi = sinf(afsf);
    }
    TimingAnalyzer_Stop(&analyzers[0]);
    TimingAnalyzer_Print(&analyzers[0]);
    sprintf(temp, "Value of Result of SINF: %f\n", fi);
    UART_PutString(temp);

for(;;);
}

#endif


/**/
#ifdef TestCase4

CY_ISR_PROTO(Timer_1ms);
CY_ISR_PROTO(Timer_2s);

int time1_var_t = 0;
int time1_var_t2 = 0;
TimingAnalyser_Instance_t analyzers[2];
int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */
    Timer_1_Start();
    Timer_2_Start();
   
    isr_timer_1_StartEx(Timer_1ms);
    isr_timer_2_StartEx(Timer_2s);

    char temp[32];
    uint32_t Active_Analyzer[] = {0,1};
    uint32_t Active_Analyzer_size = sizeof(Active_Analyzer)/sizeof(Active_Analyzer[0]);
    
    
    TimingAnalyzer_Init(); //Initalize the analyzer and pheripherals
    
    RC_t rc = TimingAnalyzer_Create(&analyzers[0], C_SYSTICK, LED_GREEN, "Timer DWT_OUTPUT", 111);
    RC_t rc1 = TimingAnalyzer_Create(&analyzers[1], C_SYSTICK, LED_RED, "Timer DWT", 222);

    if(rc != RC_SUCCESS || rc1 != RC_SUCCESS){
        sprintf(temp, "Error initalizing");
        UART_PutString(temp);
    }
    

    
    for(;;){
    
        if(time1_var_t == 2000){
            TimingAnalyzer_Print(&analyzers[1]);
            CyDelay(10);
            
        }
        
        if(time1_var_t2 == 20){
            TimingAnalyzer_Print(&analyzers[1]);
            CyDelay(10);
        }

    }

}

CY_ISR(Timer_1ms){
    TimingAnalyzer_Start(&analyzers[1]);
    Timer_1_ReadStatusRegister();   // Clear interrupt flag
    isr_timer_1_ClearPending();
    TimingAnalyzer_Stop(&analyzers[1]);
    time1_var_t++;
    if(time1_var_t == 2000){
        TimingAnalyzer_Print(&analyzers[1]);
        CyDelay(10);
    }
    return;

}
CY_ISR(Timer_2s){

    Timer_2_ReadStatusRegister();   // Clear interrupt flag
    isr_timer_2_ClearPending();
    TimingAnalyzer_Start(&analyzers[0]);
    CyDelay(1010);
    TimingAnalyzer_Stop(&analyzers[0]);
    TimingAnalyzer_Print(&analyzers[0]);
    return;

}

#endif

#ifdef TestCase5

int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */

    char temp[32];
    uint32_t Active_Analyzer[] = {0,1};
    uint32_t Active_Analyzer_size = sizeof(Active_Analyzer)/sizeof(Active_Analyzer[0]);
    TimingAnalyser_Instance_t analyzers[2];
    
    TimingAnalyzer_Init(); //Initalize the analyzer and pheripherals
    
    RC_t rc = TimingAnalyzer_Create(&analyzers[0], C_DWT_OUTPUT, LED_GREEN, "Timer DWT_OUTPUT", 111);
    RC_t rc1 = TimingAnalyzer_Create(&analyzers[1], C_SYSTICK, LED_RED, "Timer SYSTICK_OUTPUT", 222);

    if(rc != RC_SUCCESS || rc1 != RC_SUCCESS){
        sprintf(temp, "Error initalizing");
        UART_PutString(temp);
    }

    TimingAnalyzer_Start(&analyzers[0]);
    TimingAnalyzer_Stop(&analyzers[0]);
    TimingAnalyzer_Print(&analyzers[0]);
    for(;;){}

}



#endif


/* [] END OF FILE */
