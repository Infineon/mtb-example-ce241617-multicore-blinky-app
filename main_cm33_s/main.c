/*****************************************************************************
* File Name        : main.c
*
* Description      : Main CM33 secure core application that boots PPCA cores,
*                    monitors shared memory for inter-core communication, and
*                    controls LEDs based on PPCA core activity.
*
* Related Document : See README.md
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


/******************************************************************************
 * Header Files
 *****************************************************************************/

#include "cy_pdl.h"
#include "cybsp.h"
#include "cy_retarget_io.h"

/*******************************************************************************
* Macros
*******************************************************************************/

/* Flash memory addresses where PPCA core images are stored */
#define CORE0_IMAGE_ADDRESS   CYMEM_CM33_0_S_m33s_ppca0_nvm_C_S_START
#define CORE1_IMAGE_ADDRESS   CYMEM_CM33_0_S_m33s_ppca1_nvm_C_S_START

#define PPCA0_IMAGE_SIZE      CYMEM_CM33_0_S_ppca0_code_SIZE
#define PPCA1_IMAGE_SIZE      CYMEM_CM33_0_S_ppca1_code_SIZE

/* Shared memory addresses for inter-core communication */
/* PPCA cores write to these locations, main core reads from them */
/* Variables located in M4 shared memory space (16KB at 0x20040000-0x20043FFF from PPCA view) */
/* Main core accesses PPCA memory through PPCA peripheral base with memory windows: */
/* M1 (CPU0 data): 0x53020000, M3 (CPU1 data): 0x53040000, M4 (shared): 0x53050000 */
#define PPCA_CPU0_M4_VAR_ADDRESS   0x53050400  /* Written by PPCA Core 0 */
#define PPCA_CPU1_M4_VAR_ADDRESS   0x53050800  /* Written by PPCA Core 1 */

/* Timing configuration */
#define POLLING_DELAY_MS      10     /* Main loop polling interval */
#define LED3_PERIOD_MS        500    /* LED3 heartbeat period */
#define LED3_COUNTER_MAX      (LED3_PERIOD_MS / POLLING_DELAY_MS)

/*******************************************************************************
* Global Variables
*******************************************************************************/

/* Debug UART variables */
static cy_stc_scb_uart_context_t    DEBUG_UART_context; /* DEBUG_UART context */
static mtb_hal_uart_t               DEBUG_UART_hal_obj; /* Debug DEBUG_UART HAL object */

/*******************************************************************************
* Function Prototype
*******************************************************************************/


/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
* Main CM33 Core application:
*    1. Boots PPCA Core 0 and Core 1
*    2. Monitors shared memory variables from both PPCA cores
*    3. Controls LEDs based on PPCA core activity:
*       - LED1: Toggles when Core 0 changes its variable (1 sec period)
*       - LED2: Toggles when Core 1 changes its variable (1 sec, 500ms offset)
*       - LED3: Heartbeat showing main core is alive (500ms period)
*    4. Prints activity messages to UART terminal
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
    cy_rslt_t result;
    
    /* Initialize the device and board peripherals */
    result = cybsp_init();

    /* Board init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Initialize UART hardware for debug output */
    Cy_SCB_UART_Init(DEBUG_UART_HW, &DEBUG_UART_config, &DEBUG_UART_context);
    Cy_SCB_UART_Enable(DEBUG_UART_HW);

    /* Setup the HAL DEBUG_UART */
    result = mtb_hal_uart_setup(&DEBUG_UART_hal_obj, &DEBUG_UART_hal_config,
                                &DEBUG_UART_context, NULL);

    /* HAL DEBUG_UART init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Initialize redirecting of low level IO (printf) to UART */
    result = cy_retarget_io_init(&DEBUG_UART_hal_obj);

    /* retarget IO init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Transmit header to the terminal */
    /* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */
    printf("\x1b[2J\x1b[;H");
    printf("************************************************************\r\n");
    printf("PSOC Control C3M/P8: Multicore blinky application\r\n");
    printf("************************************************************\r\n\n");

    /* enable interrupts */
    __enable_irq();

    // Initialize and enable PPCA configuration
    Cy_PPCA_CNFG_Init(PPCA_CNFG_HW, &PPCA_CNFG_config);
    Cy_PPCA_Enable(PPCA_CNFG_HW);

    /* Boot PPCA Core 0 and Core 1 from flash memory */
    Cy_System_Init_CPU0((void*)CORE0_IMAGE_ADDRESS, PPCA0_IMAGE_SIZE);
    Cy_System_Init_CPU1((void*)CORE1_IMAGE_ADDRESS, PPCA1_IMAGE_SIZE);

    /* Display multicore application banner */
    printf("PPCA CM33 Cores Boot Complete!\r\n\r\n");
    printf("========================================\r\n");
    printf("  Multicore Blinky LED Pattern\r\n");
    printf("========================================\r\n");
    #if !defined(APP_KIT_PSC3M8_CC1)
    printf("  LED1: PPCA Core0 (1 sec toggle)\r\n");
    printf("  LED2: PPCA Core1 (1 sec toggle, staggered)\r\n");
    #endif
    printf("  LED3: Main Core  (500ms heartbeat)\r\n");
    printf("========================================\r\n\r\n");

    /* Pointers to shared memory locations for inter-core communication */
    uint32_t *ppca_core0_var = (uint32_t *)PPCA_CPU0_M4_VAR_ADDRESS;
    uint32_t *ppca_core1_var = (uint32_t *)PPCA_CPU1_M4_VAR_ADDRESS;

    /* Counter for LED3 heartbeat timing (500ms period with 10ms polling) */
    uint32_t led3_counter = 0;
    
    /* Previous values to detect changes in shared variables */
    uint32_t prev_core0_val = *ppca_core0_var;
    uint32_t prev_core1_val = *ppca_core1_var;

    printf("Monitoring PPCA cores activity...\r\n");

    for (;;)
    {
         /* Monitor PPCA Core 0 activity */
         /* Check if Core 0 has changed its shared variable */
         if(*ppca_core0_var != prev_core0_val)
         {
             #if !defined(APP_KIT_PSC3M8_CC1)
                /* Toggle LED1 to indicate Core 0 activity (EVK kit only) */
                Cy_GPIO_Inv(CYBSP_USER_LED1_PORT, CYBSP_USER_LED1_PIN);
                /* Print status update with LED state */
                printf("\r\n[PPCA Core0] Value: %ld | LED1: %s", 
                    (long)*ppca_core0_var, 
                    (const char*)((*ppca_core0_var == 1) ? "ON " : "OFF"));
             #else
                /* Print status update without LED state (CC1 kit) */
                printf("\r\n[PPCA Core0] Value: %ld", (long)*ppca_core0_var);
             #endif
             
             /* Update previous value to detect next change */
             prev_core0_val = *ppca_core0_var;
         }

         /* Monitor PPCA Core 1 activity */
         /* Check if Core 1 has changed its shared variable */
         if(*ppca_core1_var != prev_core1_val)
         {
             #if !defined(APP_KIT_PSC3M8_CC1)
                /* Toggle LED2 to indicate Core 1 activity (EVK kit only) */
                Cy_GPIO_Inv(CYBSP_USER_LED2_PORT, CYBSP_USER_LED2_PIN);
                /* Print status update with LED state */
                printf("\r\n[PPCA Core1] Value: %ld | LED2: %s", 
                        (long)*ppca_core1_var,
                        (const char*)((*ppca_core1_var == 1) ? "ON " : "OFF"));
             #else
                /* Print status update without LED state (CC1 kit) */
                printf("\r\n[PPCA Core1] Value: %ld", (long)*ppca_core1_var);
             #endif
             
             /* Update previous value to detect next change */
             prev_core1_val = *ppca_core1_var;
         }

         /* Main core heartbeat - LED3 toggles every 500ms */
         /* Counter increments every 10ms, so LED3_COUNTER_MAX counts = 500ms */
         if(led3_counter >= LED3_COUNTER_MAX)
         {
             Cy_GPIO_Inv(CYBSP_USER_LED3_PORT, CYBSP_USER_LED3_PIN);
             led3_counter = 0;
         }
         led3_counter++;

         /* Poll shared memory every 10ms for responsive LED updates */
         Cy_SysLib_Delay(POLLING_DELAY_MS);  
    }
}