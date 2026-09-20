/******************************************************************************
* File Name:   main.c
*
* Description: PPCA Core 1 application for multicore blinky demo.
*              Toggles shared memory variable every 1 second (staggered 500ms
*              from Core 0) to communicate with main core. LED2 reflects this
*              core's activity.
*
* Related Document: See README.md
*
*
********************************************************************************
* (c) 2026, Infineon Technologies AG, or an affiliate of Infineon
* Technologies AG. All rights reserved.
* This software, associated documentation and materials ("Software") is
* owned by Infineon Technologies AG or one of its affiliates ("Infineon")
* and is protected by and subject to worldwide patent protection, worldwide
* copyright laws, and international treaty provisions. Therefore, you may use
* this Software only as provided in the license agreement accompanying the
* software package from which you obtained this Software. If no license
* agreement applies, then any use, reproduction, modification, translation, or
* compilation of this Software is prohibited without the express written
* permission of Infineon.
*
* Disclaimer: UNLESS OTHERWISE EXPRESSLY AGREED WITH INFINEON, THIS SOFTWARE
* IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
* INCLUDING, BUT NOT LIMITED TO, ALL WARRANTIES OF NON-INFRINGEMENT OF
* THIRD-PARTY RIGHTS AND IMPLIED WARRANTIES SUCH AS WARRANTIES OF FITNESS FOR A
* SPECIFIC USE/PURPOSE OR MERCHANTABILITY.
* Infineon reserves the right to make changes to the Software without notice.
* You are responsible for properly designing, programming, and testing the
* functionality and safety of your intended application of the Software, as
* well as complying with any legal requirements related to its use. Infineon
* does not guarantee that the Software will be free from intrusion, data theft
* or loss, or other breaches ("Security Breaches"), and Infineon shall have
* no liability arising out of any Security Breaches. Unless otherwise
* explicitly approved by Infineon, the Software may not be used in any
* application where a failure of the Product or any consequences of the use
* thereof can reasonably be expected to result in personal injury.
*******************************************************************************/

/*******************************************************************************
* Header Files
********************************************************************************/

#include "cy_pdl.h"
#include "cycfg.h"
#include <stdio.h>

/*******************************************************************************
* Macros
********************************************************************************/
/* Toggle period: 1 second */
#define LOOP_DELAY_MS 1000

/* Shared memory addresses in M4 shared memory space (0x20040000-0x20043FFF) */
/* All cores can access M4 shared memory for inter-core communication */
#define PPCA_CPU1_M4_VAR_ADDRESS 0x20040800  /* Used by this core (CPU1) */

/*******************************************************************************
* Global Variables
********************************************************************************/


/*******************************************************************************
* Function Prototypes
********************************************************************************/


/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
* PPCA Core 1 application:
*    - Toggles shared variable between 0 and 1 every 1 second
*    - Starts with 500ms offset from Core 0 for staggered LED pattern
*    - Main core monitors this variable and controls LED2
*    - LED2 state reflects this core's activity
*
* Parameters:
*  void
*
* Return:
*  int
*
*******************************************************************************/
int main(void)
{
     /* Pointer to shared memory location for inter-core communication */
     uint32_t *shared_var = (uint32_t *)PPCA_CPU1_M4_VAR_ADDRESS;
     
     /* Initialize shared variable to 0 */
     *shared_var = 0;

     /* Initial delay to stagger Core 1 toggles from Core 0 by 500ms */
     /* This creates a visually appealing alternating LED pattern */
     Cy_SysLib_Delay(500);

     /* Main loop: Toggle shared variable every 1 second */
     for(;;)
     {
          /* Wait for 1 second */
          Cy_SysLib_Delay(LOOP_DELAY_MS);
          
          /* Toggle between 0 and 1 (logical NOT converts 0->1 and 1->0) */
          *shared_var = !(*shared_var);
     }
}
