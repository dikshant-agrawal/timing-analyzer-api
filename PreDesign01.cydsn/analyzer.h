/* ========================================
 * \file	analyzer.h
 * \author	Dikshant Agrawal 	Hochschule Darmstadt - dikshant.agrawal@stud.h-da.de
 * \date 	27.10.2025
 * \version	1.0
 *
 * \brief API for Timing Analysis using various methods such as DWT, SYSTICK and OUTPUT Pin
 * 
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

#ifndef ANALYZER_H_
#define ANALYZER_H_
   
/*Including Pre-defined drivers*/    
#include "global.h"
#include "project.h"


/*****************************************************************************/
/* Global pre-processor symbols/macros and type declarations                 */
/*****************************************************************************/
  
    
volatile uint32_t TimingAnalyzer_SystickCounterMillis;
    
// Check if the mode is any of the DWT modes
#define IS_DWT_MODE(mode) ((mode) == C_DWT || (mode) == C_DWT_OUTPUT)

// Check if the mode is any of the SysTick modes
#define IS_SYSTICK_MODE(mode) ((mode) == C_SYSTICK || (mode) == C_SYSTICK_OUTPUT)
    
// Check if the mode is any of the OUTPUT modes
#define IS_OUTPUT_PIN(mode) ((mode) == C_DWT_OUTPUT || (mode) == C_SYSTICK_OUTPUT || (mode) == C_OUTPUT)

    
/**
* @enum TimingAnalyzer_Configuration_t    
* @brief Configuration Modes for Timing Analyzer.
*/
typedef enum {
    C_DWT = 0,          ///<Configuration 0 = Timing Analysis using DWT Cycle Counter only.
    C_DWT_OUTPUT,       ///<Configuration 1 = Timing Analysis using DWT Cycle Counter with OUTPUT Toggling to be analyzed using Logic Analyzer.
    C_SYSTICK,          ///<Configuration 2 = Timing Analysis using SYS Tick Counter only.
    C_SYSTICK_OUTPUT,   ///<Configuration 3 = Timing Analysis using SYS Tick Counter with OUTPUT Toggling.
    C_OUTPUT            ///<Configuration 4 = Timing Analysis using OUTPUT toggling to be analyzed using Logic Analyzer.
} TimingAnalyser_Configuration_t;

/**
* @enum TimingAnalyzer_State_t    
* @brief Current State of Timing Analyzer Object.
*/
typedef enum {
    S_UNCONFIGURED = 0, ///<State 0 = Unconfigured Timing Analyzer Object.
    S_IDLE,             ///<State 1 = Timing Analyzer configured but not started.
    S_RUNNING,          ///<State 2 = Timing Analyzer running i.e. Measurement in progress.
    S_PAUSED,           ///<State 3 = Timing Analyzer paused i.e. Measurement Interrupted by user.
    S_STOPPED,          ///<State 4 = Timing Analyzer stopped i.e.e Measurement Ended Result is Final.
} TimingAnalyzer_State_t;

/**
* @enum Output_id_t
* \brief Enumeration identifying the OUTPUT to be set
*/
typedef enum {
  LED_RED,              ///< Selection of red LED
  LED_YELLOW,           ///< Selection of yellow LED 
  LED_GREEN,            ///< Selection of green LED
  LOGIC_1,              ///< Selection of LOGIC 1
  LOGIC_2               ///< Selection of LOGIC 2
}Output_id_t;         

/**
* @enum Output_ONOFF_t
* \brief Enumeration identifying the OUTPUT to be set
*/
typedef enum {
    OUTPUT_OFF = 0,        ///< Turn the Output OFF
    OUTPUT_ON  = 1         ///< Turn the Output ON
}Output_ONOFF_t;

typedef void (*Output_WriteFunc_t)(uint8_t);


/**
* @struct TimingAnalyser_Instance_t    
* @brief Object instance for Timing Analyzer.
*/
typedef struct {
    TimingAnalyser_Configuration_t mode;    ///<Selection of Analyzer Configuration Mode.
    Output_id_t OutputId;                   ///<Passing OutputId to trigger specific LED from Red, Green and Yellow.
    char_t name[32];                        ///<Name of the Timing Analyzer Instance.
    TimingAnalyzer_State_t state;           ///<Current State of Timing Analyzer Instance.
    uint32_t startStamp;                    ///<Signature when DWT Cycle Count Starts.
    uint32_t accumulatedCycles;             ///<Total number of cycles stored from start to Pause/Stop.
    uint32_t elapsedCycles;                 ///<Total number of cycles passed from start to current stamp.
    boolean_t validResult;                  ///<To check if the result is valid or not.
    uint32_t Systick_start;
    uint32_t Systick_elapsed;
    uint32_t Systick_accumulated;
    uint32_t analyzerID;
}TimingAnalyser_Instance_t;                


/*****************************************************************************/
/* API functions                                                             */
/*****************************************************************************/

/** 
* @brief Static inline for getting the SYSTICK and DWT Cycle count at the very start of the function
* @param  const TimingAnalyser_Instance_t *pInstance to check instance is running in which mode
*/
static inline uint32_t TimingAnalyzer_CaptureEntry(const TimingAnalyser_Instance_t *pInstance)
{
    return (IS_DWT_MODE(pInstance->mode)) ? DWT->CYCCNT : TimingAnalyzer_SystickCounterMillis;
}
 
/*
* @brief Activate the trace unit and the cycle counter, sets the state of counter to 0
* \Led is set to OFF at start
* \UART Peripheral block is started
*/
void TimingAnalyzer_Init(void);

/**  
* @brief This function creates the timer instance
* @param TimingAnalyser_Instance_t *pInstance takes the address of struct initalzed for this instance.
* @param TimingAnalyser_Configuration_t mode defines the mode in which you want to use the timer analyzer.
* @param LED_id_t led_id defines the LED color which needs to be assigned with the analyzer instance
* @param const pchar_t name self explantory name can be assigned to each instance for notation purpose
* @param uint32_t ID different IDs can be assigned to different analyzers
* @return RC_t (RC Codes decalred in global.h
*/
RC_t TimingAnalyzer_Create(TimingAnalyser_Instance_t *pInstance, 
    TimingAnalyser_Configuration_t mode, Output_id_t Output_id, 
    const pchar_t name, uint32_t ID);


/**  
* @brief starts the instance and checks the state of timer operation if it is not in stopped or idle it will return invalid state
* @param TimingAnalyser_Instance_t *pInstance takes the address of struct initalzed for this instance.
* @return RC_SUCCESS if function was executed as exepected, other error code in case of error
*/
RC_t TimingAnalyzer_Start(TimingAnalyser_Instance_t *pInstance);

/**  
* @brief Timing analyser pause function which stores the DWT/SysTick counts in accumulatedCycle variable and Turns OFF the output
* @param TimingAnalyser_Instance_t *pInstance takes the address of struct initalzed for this instance.
* @return RC_SUCCESS if function was executed as exepected, other error code in case of error
*/
RC_t TimingAnalyzer_Pause(TimingAnalyser_Instance_t *pInstance);

/**  
* @brief Resume function if the timer is in paused state you can resume back from where you paused counting.
* @param TimingAnalyser_Instance_t *pInstance takes the address of struct initalzed for this instance.
* @return RC_SUCCESS if function was executed as exepected, other error code in case of error
*/
RC_t TimingAnalyzer_Resume(TimingAnalyser_Instance_t *pInstance);


/**  
* @brief Stop the instance and store the final values to the elapsedCycle variable
* \It also checks the overflowCounter and if DWT overflow it will shift the value by 32 bit and do a bitwise OR operation
* \to get the current cycle count, by subtracting with the start cycle count we can have the total elapsed clock cycles
* \Sets the valid result flag to TRUE so that print statement can check and log on UART
* @param TimingAnalyser_Instance_t *pInstance takes the address of struct initalzed for this instance.
* @return RC_SUCCESS if function was executed as exepected, other error code in case of error
*/
RC_t TimingAnalyzer_Stop(TimingAnalyser_Instance_t *pInstance);

/**  
* @brief 
* @param TimingAnalyser_Instance_t *pInstance takes the address of struct initalzed for this instance.
*/
void TimingAnalyzer_Print(const TimingAnalyser_Instance_t *pInstance);

/**  
* @brief 
*/
void TimingAnalyzer_PrintAll(const TimingAnalyser_Instance_t *pInstance, uint32_t *Active_Analyzer, uint32_t Active_Analyzer_size);

/**  
* @brief 
*/
RC_t Output_Init();


/**  
* @brief 
* 
*/
RC_t Output_Set(Output_id_t OutputId, Output_ONOFF_t OutputOnOff);


/*****************************************************************************/
/* Extern global variables                                                   */
/*****************************************************************************/


extern uint32_t Active_Analyzer[];
extern uint32_t Active_Analyzer_size;
extern TimingAnalyser_Instance_t analyzers[];


#endif  // TIMING_ANALYZER_H_
/* [] END OF FILE */
