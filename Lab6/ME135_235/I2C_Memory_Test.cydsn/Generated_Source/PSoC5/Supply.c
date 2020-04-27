/*******************************************************************************
* File Name: Supply.c  
* Version 1.90
*
* Description:
*  This file provides the source code to the API for the 8-bit Voltage DAC 
*  (VDAC8) User Module.
*
* Note:
*  Any unusual or non-standard behavior should be noted here. Other-
*  wise, this section should remain blank.
*
********************************************************************************
* Copyright 2008-2012, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#include "cytypes.h"
#include "Supply.h"

#if (CY_PSOC5A)
#include <CyLib.h>
#endif /* CY_PSOC5A */

uint8 Supply_initVar = 0u;

#if (CY_PSOC5A)
    static uint8 Supply_restoreVal = 0u;
#endif /* CY_PSOC5A */

#if (CY_PSOC5A)
    static Supply_backupStruct Supply_backup;
#endif /* CY_PSOC5A */


/*******************************************************************************
* Function Name: Supply_Init
********************************************************************************
* Summary:
*  Initialize to the schematic state.
* 
* Parameters:
*  void:
*
* Return:
*  void
*
* Theory:
*
* Side Effects:
*
*******************************************************************************/
void Supply_Init(void) 
{
    Supply_CR0 = (Supply_MODE_V );

    /* Set default data source */
    #if(Supply_DEFAULT_DATA_SRC != 0 )
        Supply_CR1 = (Supply_DEFAULT_CNTL | Supply_DACBUS_ENABLE) ;
    #else
        Supply_CR1 = (Supply_DEFAULT_CNTL | Supply_DACBUS_DISABLE) ;
    #endif /* (Supply_DEFAULT_DATA_SRC != 0 ) */

    /* Set default strobe mode */
    #if(Supply_DEFAULT_STRB != 0)
        Supply_Strobe |= Supply_STRB_EN ;
    #endif/* (Supply_DEFAULT_STRB != 0) */

    /* Set default range */
    Supply_SetRange(Supply_DEFAULT_RANGE); 

    /* Set default speed */
    Supply_SetSpeed(Supply_DEFAULT_SPEED);
}


/*******************************************************************************
* Function Name: Supply_Enable
********************************************************************************
* Summary:
*  Enable the VDAC8
* 
* Parameters:
*  void
*
* Return:
*  void
*
* Theory:
*
* Side Effects:
*
*******************************************************************************/
void Supply_Enable(void) 
{
    Supply_PWRMGR |= Supply_ACT_PWR_EN;
    Supply_STBY_PWRMGR |= Supply_STBY_PWR_EN;

    /*This is to restore the value of register CR0 ,
    which is modified  in Stop API , this prevents misbehaviour of VDAC */
    #if (CY_PSOC5A)
        if(Supply_restoreVal == 1u) 
        {
             Supply_CR0 = Supply_backup.data_value;
             Supply_restoreVal = 0u;
        }
    #endif /* CY_PSOC5A */
}


/*******************************************************************************
* Function Name: Supply_Start
********************************************************************************
*
* Summary:
*  The start function initializes the voltage DAC with the default values, 
*  and sets the power to the given level.  A power level of 0, is the same as
*  executing the stop function.
*
* Parameters:
*  Power: Sets power level between off (0) and (3) high power
*
* Return:
*  void 
*
* Global variables:
*  Supply_initVar: Is modified when this function is called for the 
*  first time. Is used to ensure that initialization happens only once.
*
*******************************************************************************/
void Supply_Start(void)  
{
    /* Hardware initiazation only needs to occure the first time */
    if(Supply_initVar == 0u)
    { 
        Supply_Init();
        Supply_initVar = 1u;
    }

    /* Enable power to DAC */
    Supply_Enable();

    /* Set default value */
    Supply_SetValue(Supply_DEFAULT_DATA); 
}


/*******************************************************************************
* Function Name: Supply_Stop
********************************************************************************
*
* Summary:
*  Powers down DAC to lowest power state.
*
* Parameters:
*  void
*
* Return:
*  void
*
* Theory:
*
* Side Effects:
*
*******************************************************************************/
void Supply_Stop(void) 
{
    /* Disble power to DAC */
    Supply_PWRMGR &= (uint8)(~Supply_ACT_PWR_EN);
    Supply_STBY_PWRMGR &= (uint8)(~Supply_STBY_PWR_EN);

    /* This is a work around for PSoC5A  ,
    this sets VDAC to current mode with output off */
    #if (CY_PSOC5A)
        Supply_backup.data_value = Supply_CR0;
        Supply_CR0 = Supply_CUR_MODE_OUT_OFF;
        Supply_restoreVal = 1u;
    #endif /* CY_PSOC5A */
}


/*******************************************************************************
* Function Name: Supply_SetSpeed
********************************************************************************
*
* Summary:
*  Set DAC speed
*
* Parameters:
*  power: Sets speed value
*
* Return:
*  void
*
* Theory:
*
* Side Effects:
*
*******************************************************************************/
void Supply_SetSpeed(uint8 speed) 
{
    /* Clear power mask then write in new value */
    Supply_CR0 &= (uint8)(~Supply_HS_MASK);
    Supply_CR0 |=  (speed & Supply_HS_MASK);
}


/*******************************************************************************
* Function Name: Supply_SetRange
********************************************************************************
*
* Summary:
*  Set one of three current ranges.
*
* Parameters:
*  Range: Sets one of Three valid ranges.
*
* Return:
*  void 
*
* Theory:
*
* Side Effects:
*
*******************************************************************************/
void Supply_SetRange(uint8 range) 
{
    Supply_CR0 &= (uint8)(~Supply_RANGE_MASK);      /* Clear existing mode */
    Supply_CR0 |= (range & Supply_RANGE_MASK);      /*  Set Range  */
    Supply_DacTrim();
}


/*******************************************************************************
* Function Name: Supply_SetValue
********************************************************************************
*
* Summary:
*  Set 8-bit DAC value
*
* Parameters:  
*  value:  Sets DAC value between 0 and 255.
*
* Return: 
*  void 
*
* Theory: 
*
* Side Effects:
*
*******************************************************************************/
void Supply_SetValue(uint8 value) 
{
    #if (CY_PSOC5A)
        uint8 Supply_intrStatus = CyEnterCriticalSection();
    #endif /* CY_PSOC5A */

    Supply_Data = value;                /*  Set Value  */

    /* PSOC5A requires a double write */
    /* Exit Critical Section */
    #if (CY_PSOC5A)
        Supply_Data = value;
        CyExitCriticalSection(Supply_intrStatus);
    #endif /* CY_PSOC5A */
}


/*******************************************************************************
* Function Name: Supply_DacTrim
********************************************************************************
*
* Summary:
*  Set the trim value for the given range.
*
* Parameters:
*  range:  1V or 4V range.  See constants.
*
* Return:
*  void
*
* Theory: 
*
* Side Effects:
*
*******************************************************************************/
void Supply_DacTrim(void) 
{
    uint8 mode;

    mode = (uint8)((Supply_CR0 & Supply_RANGE_MASK) >> 2) + Supply_TRIM_M7_1V_RNG_OFFSET;
    Supply_TR = CY_GET_XTND_REG8((uint8 *)(Supply_DAC_TRIM_BASE + mode));
}


/* [] END OF FILE */
