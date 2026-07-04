/* ========================================
 * \file	analyzer.c
 * \author	Dikshant Agrawal 	Hochschule Darmstadt - dikshant.agrawal@stud.h-da.de
 * \date 	27.10.2025
 * \version	1.0
 *
 *  \brief Implementation of functions, pointers, strucs, enums for timing analyzer
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * \warning This software is a PROTOTYPE version and is not designed or intended for use in production, especially not for safety-critical applications!
 * The user represents and warrants that it will NOT use or redistribute the Software for such purposes.
 * This prototype is for research purposes only. This software is provided "AS IS," without a warranty of any kind.
 *
 * ========================================
*/

#include "project.h"
#include "analyzer.h"
#include "stdio.h"



/*****************************************************************************/
/* Function implementation - global ('extern') and local ('static')          */
/*****************************************************************************/


static Output_WriteFunc_t Output_WriteFunctions[] = {
    LED_red_Write,
    LED_yellow_Write,
    LED_green_Write,
    Logic_1_Write,
    Logic_2_Write
};

RC_t Output_Init()
{   
    for(Output_id_t i = LED_RED; i <= LOGIC_2 ; i++){
        Output_WriteFunctions[i](OUTPUT_OFF); 
    }
    return RC_SUCCESS;
}

RC_t Output_Set(Output_id_t OutputId, Output_ONOFF_t OutputOnOff){
    
    if(OutputId > LOGIC_2){
        return RC_ERROR_BAD_PARAM;
    }
    Output_WriteFunctions[OutputId](OutputOnOff); 
    return RC_SUCCESS;
}
  
/*
 * Systick Handler - used to increment the milliseconds counter each millisecond
 */
static void SysTick_Handler(void)
{
    TimingAnalyzer_SystickCounterMillis++;
}

/*
* Init function to activate the trace unit and activate the cycle counter also sets the state of counter to 0
* Led is set to OFF at start
* UART Peripheral block is started
*/
void TimingAnalyzer_Init(void){
    // Init for DWT
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;     // Activate the Trace unit
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;                // Activate the Cycle Counter
    DWT->CYCCNT = 0;                                    // Reset counter to 0
    // Init for LED Pins
    Output_Init();                                      // Initialize the Output to OFF State
    UART_Start();                                       //UART Logging Initalize
    // Init for SYS TICK
	CySysTickInit();									// Activate Systick counter
	CySysTickSetCallback(0, SysTick_Handler);           // Set ISR for Systick
    CySysTickSetReload(BCLK__BUS_CLK__HZ / 1000 - 1);   // Set 1ms cycle Time
    CySysTickEnable();	
}

/**  
* @brief This function creates the timer instance
* @param TimingAnalyser_Instance_t *pInstance takes the address of struct initalzed for this instance.
* @param TimingAnalyser_Configuration_t mode defines the mode in which you want to use the timer analyzer.
* @param LED_id_t led_id defines the LED color which needs to be assigned with the analyzer instance
* @param const pchar_t name self explantory name can be assigned to each instance for notation purpose
* @param uint32_t ID different IDs can be assigned to different analyzers
* @return RC_t (RC Codes decalred in global.h
*/
RC_t TimingAnalyzer_Create(TimingAnalyser_Instance_t *pInstance, TimingAnalyser_Configuration_t mode, Output_id_t Output_id, const pchar_t name, uint32_t ID){
    if(!pInstance || !name ){
        return RC_ERROR_NULL;    
    }
    if(mode > C_OUTPUT || Output_id > LOGIC_2){
        return RC_ERROR_BAD_PARAM;
    }
    
    strncpy((char *)pInstance->name, (const char *)name, 32 - 1);
    pInstance->name[32 - 1] = '\0';  // Ensure null termination

    pInstance->mode = mode;             
    pInstance->OutputId = Output_id;
    pInstance->state = S_IDLE;
    pInstance->analyzerID = ID;
    pInstance->validResult = FALSE;
    pInstance->startStamp = 0;
    pInstance->elapsedCycles = 0;
    pInstance->accumulatedCycles = 0;
    pInstance->Systick_start = 0;
    pInstance->Systick_elapsed = 0;
    pInstance->Systick_accumulated = 0;
   
    return RC_SUCCESS;
}

/**  
* @brief starts the instance and checks the state of timer operation if it is not in stopped or idle it will return invalid state
* @param TimingAnalyser_Instance_t *pInstance takes the address of struct initalzed for this instance.
* @return RC_SUCCESS if function was executed as exepected, other error code in case of error
*/
RC_t TimingAnalyzer_Start(TimingAnalyser_Instance_t *pInstance){
    
    uint32_t entryValue = TimingAnalyzer_CaptureEntry(pInstance);
    
    // If the name of instance is not found return RC_ERROR_NULL
    if(!pInstance){
        return RC_ERROR_NULL;
    }
    
    //If the state of instance is not running or paused, it cannot be stopped and return RC_INVALID_STATE
    if(pInstance->state != S_IDLE && pInstance->state != S_STOPPED){
        return RC_ERROR_INVALID_STATE;
    }                      
        
    //Starting the DWT Cycle Counter and storing the value in startStamp
    if(IS_DWT_MODE(pInstance->mode)){
        pInstance->startStamp = entryValue; 
        pInstance->elapsedCycles= 0;
        pInstance->accumulatedCycles = 0;
        
    }
        
    // Checking if the Mode is set to use SYSTICK cycle counter for timing analysis
    if(IS_SYSTICK_MODE(pInstance->mode)){
        pInstance->Systick_start = entryValue;
        pInstance->Systick_elapsed = 0;
        pInstance->Systick_accumulated = 0;
    }
      
    //If the mode is using OUTPUT Pin here we turn ON the PIN which marks as start of cycle
    if(IS_OUTPUT_PIN(pInstance->mode)){
        Output_Set(pInstance->OutputId, OUTPUT_ON);
    }

    //Changing the state from IDLE to Running
    pInstance->state = S_RUNNING;
    //Set the result ready flag to FALSE
    pInstance->validResult = FALSE;
    return RC_SUCCESS;
    
}

/**  
* @brief Timing analyser pause function which stores the DWT/SysTick counts in accumulatedCycle variable and Turns OFF the output
* @param TimingAnalyser_Instance_t *pInstance takes the address of struct initalzed for this instance.
* @return RC_SUCCESS if function was executed as exepected, other error code in case of error
*/
RC_t TimingAnalyzer_Pause(TimingAnalyser_Instance_t *pInstance){
        
    uint32_t entryValue = TimingAnalyzer_CaptureEntry(pInstance);
    
    // If the name of instance is not found return RC_ERROR_NULL
    if(!pInstance){
        return RC_ERROR_NULL;
    }
    //If the state of instance is not running it cannot be paused and return RC_INVALID_STATE
    if(pInstance->state != S_RUNNING){
        return RC_ERROR_INVALID_STATE;
    }

    // Checking if the Mode is set to use DWT cycle counter for timing analysis
    if(IS_DWT_MODE(pInstance->mode)){
        pInstance->accumulatedCycles += entryValue - pInstance->startStamp;
    }    
        
    // Checking if the Mode is set to use SYSTICK cycle counter for timing analysis
    if(IS_SYSTICK_MODE(pInstance->mode)){
        pInstance->Systick_accumulated += entryValue - pInstance->Systick_start;
    }
    
    //If the mode is using OUTPUT Pin here we turn off the PIN which marks as end of cycle
    if(IS_OUTPUT_PIN(pInstance->mode)){
        Output_Set(pInstance->OutputId, OUTPUT_OFF);
    }
    
        pInstance->state = S_PAUSED;
        return RC_SUCCESS;
    
}

/**  
* @brief Resume function if the timer is in paused state you can resume back from where you paused counting.
* @param TimingAnalyser_Instance_t *pInstance takes the address of struct initalzed for this instance.
* @return RC_SUCCESS if function was executed as exepected, other error code in case of error
*/
RC_t TimingAnalyzer_Resume(TimingAnalyser_Instance_t *pInstance){
    
    uint32_t entryValue = TimingAnalyzer_CaptureEntry(pInstance);
    
    // If the name of instance is not found return RC_ERROR_NULL
    if(!pInstance){
        return RC_ERROR_NULL;
    }
    
    //If the state of instance is not paused it cannot be resumed and return RC_INVALID_STATE
    if(pInstance->state != S_PAUSED){
        return RC_ERROR_INVALID_STATE;
    }

    //Starting the DWT Cycle Counter and storing the value in startStamp
    if(IS_DWT_MODE(pInstance->mode)){
        pInstance->startStamp = entryValue; 
    }
        
    // Checking if the Mode is set to use SYSTICK cycle counter for timing analysis
    if(IS_SYSTICK_MODE(pInstance->mode)){
        pInstance->Systick_start = entryValue;
    }
      
    //If the mode is using OUTPUT Pin here we turn ON the PIN which marks as start of cycle
    if(IS_OUTPUT_PIN(pInstance->mode)){
        Output_Set(pInstance->OutputId, OUTPUT_ON);
    }

    //Making the status as Running again
    pInstance->state = S_RUNNING;
    return RC_SUCCESS;
    
}

/*
* Stop the instance and store the final values to the elapsedCycle variable
* It also checks the overflowCounter and if DWT overflow it will shift the value by 32 bit and do a bitwise OR operation
* to get the current cycle count, by subtracting with the start cycle count we can have the total elapsed clock cycles
* Sets the valid result flag to TRUE so that print statement can check and log on UART
*/
RC_t TimingAnalyzer_Stop(TimingAnalyser_Instance_t *pInstance){
    
    uint32_t entryValue = TimingAnalyzer_CaptureEntry(pInstance);
    
    // If the name of instance is not found return RC_ERROR_NULL
    if(!pInstance){
        return RC_ERROR_NULL;
    }
    //If the state of instance is not running or paused, it cannot be stopped and return RC_INVALID_STATE
    if(pInstance->state != S_RUNNING && pInstance->state != S_PAUSED){
        return RC_ERROR_INVALID_STATE;
    }

    //Checking if the Mode is set to use DWT cycle counter for timing analysis
    if(IS_DWT_MODE(pInstance->mode)){
        //Final Result Calculation
        pInstance->elapsedCycles += (pInstance->accumulatedCycles + 
            (entryValue - pInstance->startStamp));
    }
            
    // Checking if the Mode is set to use SYSTICK cycle counter for timing analysis
    if(IS_SYSTICK_MODE(pInstance->mode)){
        pInstance->Systick_elapsed += pInstance->Systick_accumulated +
            (entryValue - pInstance->Systick_start); 
    }
    
    
    //If the mode is using OUTPUT Pin here we turn off the PIN which marks as end of cycle
    if(IS_OUTPUT_PIN(pInstance->mode)){
        Output_Set(pInstance->OutputId, OUTPUT_OFF);
    }
    
    //Set the result ready flag to TRUE
    pInstance->validResult = TRUE;
    
    //End of Stop Function
    pInstance->state = S_STOPPED;
    return RC_SUCCESS;
}

/*
* Print statement checks weather the instance is valid or not and result is ready to be printed or still not ready
* Based on the validResult Flag it will print the coresponding output
*/
void TimingAnalyzer_Print(const TimingAnalyser_Instance_t *pInstance){
        
    uint32_t entryValue = TimingAnalyzer_CaptureEntry(pInstance);
    char Strbuffer[256];
    
    if(!pInstance){
        return;
    }
    
    //Checking if the Mode is set to use DWT cycle counter for timing analysis
    if(IS_DWT_MODE(pInstance->mode)){
         
        if(pInstance->validResult == TRUE){
            float32_t durationMS = ((float32_t)pInstance->elapsedCycles/(float32_t)BCLK__BUS_CLK__HZ)*1000;
            
            sprintf(Strbuffer, 
                "[Final Result : %d] CPU-Frequency : %lu Hz \nTiming Analyser ID/Name : [%u/%s] Mode:%d Status:%d Elapsed cycles: %u Duration (ms): %.6f \n",
                pInstance->validResult,(unsigned long)BCLK__BUS_CLK__HZ, (unsigned int)pInstance->analyzerID, pInstance->name,pInstance->mode,
                pInstance->state, (unsigned int)pInstance->elapsedCycles, durationMS);
        }
        else{
            uint32_t elapsedCycles = (pInstance->accumulatedCycles + (entryValue - pInstance->startStamp));
            float32_t durationMS = ((float32_t)elapsedCycles/(float32_t)BCLK__BUS_CLK__HZ)*1000;
            
            sprintf(Strbuffer, 
                "[Final Result : %d] CPU-Frequency : %lu Hz \nTiming Analyser ID/Name : [%u/%s] Mode:%d Status:%d Elapsed cycles: %u Duration (ms): %.6f \n",
                pInstance->validResult,(unsigned long)BCLK__BUS_CLK__HZ, (unsigned int)pInstance->analyzerID, pInstance->name,pInstance->mode,
                pInstance->state, (unsigned int)elapsedCycles, durationMS);
        }
    }
    
    // Checking if the Mode is set to use SYSTICK cycle counter for timing analysis
    if(IS_SYSTICK_MODE(pInstance->mode)){
        if(pInstance->validResult == TRUE){
            sprintf(Strbuffer,
                "[Final Result : %d] CPU-Frequency : %lu Hz \nTiming Analyser ID/Name : [%u/%s] Mode:%d Status:%d Duration: %u \n",
                pInstance->validResult,(unsigned long)BCLK__BUS_CLK__HZ, (unsigned int)pInstance->analyzerID, pInstance->name, pInstance->mode,pInstance->state,(unsigned int)pInstance->Systick_elapsed);
        
        }
        
        else{
            uint32_t currentTotal = (TimingAnalyzer_SystickCounterMillis - pInstance->Systick_start); 
            uint32_t duration = currentTotal + pInstance->Systick_accumulated;
            sprintf(Strbuffer,
                "[Final Result : %d] CPU-Frequency : %lu Hz \nTiming Analyser ID/Name : [%u/%s] Mode:%d Status:%d Duration: %u \n",
                pInstance->validResult,(unsigned long)BCLK__BUS_CLK__HZ, (unsigned int)pInstance->analyzerID, pInstance->name, pInstance->mode,pInstance->state,(unsigned int)duration);
        }
    }
    UART_PutString(Strbuffer);
}



void TimingAnalyzer_PrintAll(const TimingAnalyser_Instance_t *pInstance, uint32_t *Active_Analyzer, uint32_t Active_Analyzer_size){
    for(uint8_t i = 0; i< Active_Analyzer_size; i++){
        TimingAnalyzer_Print(&pInstance[Active_Analyzer[i]]);
    }
};





/* [] END OF FILE */
