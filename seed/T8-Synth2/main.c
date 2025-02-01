#include "cy_pdl.h"
#include "cybsp.h"
#include "cycfg.h"
#include "cycfg_capsense.h"
#include "cy_scb_ezi2c.h"

/*******************************************************************************
 * User configurable Macros/functions for RTT
 ********************************************************************************/
#include "SEGGER_RTT/RTT/SEGGER_RTT.h"
#include <stdio.h>

#define RTT_TUNER_CHANNEL 1
#define RTT_USE_FAST_RTT  1

#define RTT_TX_HEADER0      0x0Du
#define RTT_TX_HEADER1      0x0Au

#define RTT_TX_TAIL0        0x00u
#define RTT_TX_TAIL1        0xFFu
#define RTT_TX_TAIL2        0xFFu

typedef struct {
    uint8_t header[2];
    uint8_t tuner_data[sizeof(cy_capsense_tuner)];
    uint8_t tail[3];
} rtt_tuner_data_t;

static void rtt_tuner_send(void * context);
static void rtt_tuner_receive(uint8_t ** packet, uint8_t ** tuner_packet, void * context);
static uint8_t tuner_down_buf[32];

static rtt_tuner_data_t tuner_up_buf = {
    .header = {RTT_TX_HEADER0, RTT_TX_HEADER1},
    .tuner_data = {0},
    .tail = {RTT_TX_TAIL0, RTT_TX_TAIL1, RTT_TX_TAIL2}
};


/*******************************************************************************
 * Macros
 *******************************************************************************/
#define CAPSENSE_INTR_PRIORITY           (3u)
#define CY_ASSERT_FAILED                 (0u)

/*******************************************************************************
 * Function Prototypes
 *******************************************************************************/
static void initialize_capsense(void);
static void capsense_isr(void);
static void initialize_capsense_tuner(void);
static void ezi2c_isr(void);

/* User configurable Macros/functions for RTT */
#define RTT_TUNER_CHANNEL   1   // Канал для данных тюнера

/* EZI2C buffer variables */
#define CY_CAPSENSE_TUNER_CMD_PACKET_SIZE     (16u)
#define CAPSENSE_TUNER_CMD_RESERVED_LENGTH    (0u)
#define CAPSENSE_TUNER_CMD_SIZE              (CY_CAPSENSE_TUNER_CMD_PACKET_SIZE + CAPSENSE_TUNER_CMD_RESERVED_LENGTH)

/* Variables for EZI2C buffer */
static uint8_t i2c_buffer[2]; /* Буфер для позиции (0-100) и статуса касания */
static uint8_t tuner_cmd_buffer[CAPSENSE_TUNER_CMD_SIZE];

/* EZI2C context variable */
cy_stc_scb_ezi2c_context_t EZI2C_context;

/*******************************************************************************
 * Function Name: main
 ********************************************************************************
 * Summary:
 *  System entrance point. This function performs
 *  - initial setup of device
 *  - initialize CAPSENSE
 *  - initialize tuner communication
 *  - scan touch input continuously
 *
 * Return:
 *  int
 *
 *******************************************************************************/
int main(void)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;

    /* Initializes the RTT Control Block */
    SEGGER_RTT_Init();
    /* Configure or add an up buffer by specifying its name, size and flags */
    SEGGER_RTT_ConfigUpBuffer(RTT_TUNER_CHANNEL, "tuner", &tuner_up_buf, sizeof(tuner_up_buf) + 1, SEGGER_RTT_MODE_NO_BLOCK_TRIM);
    /* Configure or add a down buffer by specifying its name, size and flags */
    SEGGER_RTT_ConfigDownBuffer(RTT_TUNER_CHANNEL, "tuner", tuner_down_buf, sizeof(tuner_down_buf), SEGGER_RTT_MODE_BLOCK_IF_FIFO_FULL);

    /* Initialize the device and board peripherals */
    result = cybsp_init();

    /* Board init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(CY_ASSERT_FAILED);
    }

    /* Enable global interrupts */
    __enable_irq();

    /* Initialize CAPSENSE */
    initialize_capsense();

    /* Initialize CAPSENSE Tuner */
    initialize_capsense_tuner();

    /* Initialize EZI2C */
    Cy_SCB_EZI2C_Init(EZI2C_HW, &EZI2C_config, &EZI2C_context);

    /* Configure EZI2C interrupt */
    cy_stc_sysint_t ezi2c_intr_config = {
        .intrSrc = EZI2C_IRQ,
        .intrPriority = 3u,
    };
    Cy_SysInt_Init(&ezi2c_intr_config, ezi2c_isr);
    NVIC_EnableIRQ(ezi2c_intr_config.intrSrc);

    /* Start EZI2C operation */
    Cy_SCB_EZI2C_Enable(EZI2C_HW);

    /* Set up communication data buffer for EZI2C */
    Cy_SCB_EZI2C_SetBuffer1(EZI2C_HW, i2c_buffer, sizeof(i2c_buffer), sizeof(tuner_cmd_buffer), &EZI2C_context);

    /* Start the first scan */
    Cy_CapSense_ScanAllWidgets(&cy_capsense_context);

    for (;;)
    {
        if(CY_CAPSENSE_NOT_BUSY == Cy_CapSense_IsBusy(&cy_capsense_context))
        {
            /* Process all widgets */
            Cy_CapSense_ProcessAllWidgets(&cy_capsense_context);

            /* Получаем статус и позицию слайдера */
            i2c_buffer[0] = cy_capsense_tuner.widgetContext[0].wdTouch.numPosition; /* 0 = нет касания, 1 = есть касание */
            if(i2c_buffer[0] != 0)
            {
                i2c_buffer[1] = (uint8_t)cy_capsense_tuner.position[0].x; /* Позиция от 0 до 100 */
            }
            else 
            {
                i2c_buffer[1] = 0;
            }

            /* Establishes synchronized communication with the CAPSENSE Tuner tool */
            Cy_CapSense_RunTuner(&cy_capsense_context);

            /* Start the next scan */
            Cy_CapSense_ScanAllWidgets(&cy_capsense_context);

        }
    }
}


/*******************************************************************************
 * Function Name: initialize_capsense
 ********************************************************************************
 * Summary:
 *  This function initializes the CAPSENSE and configures the CAPSENSE
 *  interrupt.
 *
 *******************************************************************************/
static void initialize_capsense(void)
{
    cy_capsense_status_t status = CY_CAPSENSE_STATUS_SUCCESS;

    /* CAPSENSE interrupt configuration */
    static const cy_stc_sysint_t capsense_interrupt_config =
    {
        .intrSrc = CSD_IRQ,
        .intrPriority = CAPSENSE_INTR_PRIORITY,
    };

    /* Capture the CSD HW block and initialize it to the default state. */
    status = Cy_CapSense_Init(&cy_capsense_context);

    if (CY_CAPSENSE_STATUS_SUCCESS == status)
    {
        /* Initialize CAPSENSE interrupt */
        Cy_SysInt_Init(&capsense_interrupt_config, capsense_isr);
        NVIC_ClearPendingIRQ(capsense_interrupt_config.intrSrc);
        NVIC_EnableIRQ(capsense_interrupt_config.intrSrc);

        /* Initialize the CAPSENSE firmware modules. */
        status = Cy_CapSense_Enable(&cy_capsense_context);
    }

    if(status != CY_CAPSENSE_STATUS_SUCCESS)
    {
        /* This status could fail before tuning the sensors correctly.
         * Ensure that this function passes after the CapSense sensors are tuned
         * as per procedure given in the README.md file */
    }
}


/*******************************************************************************
 * Function Name: capsense_isr
 ********************************************************************************
 * Summary:
 *  Wrapper function for handling interrupts from CAPSENSE block.
 *
 *******************************************************************************/
static void capsense_isr(void)
{
    Cy_CapSense_InterruptHandler(CSD_HW, &cy_capsense_context);
}


/*******************************************************************************
 * Function Name: initialize_capsense_tuner
 ********************************************************************************
 * Summary:
 *  Initializes interface between Tuner GUI and PSoC 4 MCU.
 *
 *******************************************************************************/
static void initialize_capsense_tuner(void)
{
    cy_capsense_context.ptrInternalContext->ptrTunerSendCallback    = rtt_tuner_send;
    cy_capsense_context.ptrInternalContext->ptrTunerReceiveCallback = rtt_tuner_receive;
}


/*******************************************************************************
 * Function Name: rtt_tuner_send
 ********************************************************************************
 * Summary:
 *  This function sends the CAPSENSE data to Tuner through RTT
 *
 *******************************************************************************/
static void rtt_tuner_send(void * context)
{
    (void)context;

    SEGGER_RTT_LOCK();
    SEGGER_RTT_BUFFER_UP *buffer = _SEGGER_RTT.aUp + RTT_TUNER_CHANNEL;
    buffer->RdOff = 0u;
    buffer->WrOff = sizeof(tuner_up_buf);

    memcpy(tuner_up_buf.tuner_data, &cy_capsense_tuner, sizeof(cy_capsense_tuner));
    SEGGER_RTT_UNLOCK();
}


/*******************************************************************************
 * Function Name: rtt_tuner_receive
 ********************************************************************************
 * Summary:
 *  RTT receives the Tuner command
 *
 *******************************************************************************/
static void rtt_tuner_receive(uint8_t ** packet, uint8_t ** tuner_packet, void * context)
{
    uint32_t i;
    static uint32_t data_index = 0u;
    static uint8_t command_packet[16u] = {0u};
    while(0 != SEGGER_RTT_HasData(RTT_TUNER_CHANNEL))
    {
        uint8_t byte;
        SEGGER_RTT_Read(RTT_TUNER_CHANNEL, &byte, 1);
        command_packet[data_index++] = byte;
        if (CY_CAPSENSE_COMMAND_PACKET_SIZE == data_index)
        {
            if (CY_CAPSENSE_COMMAND_OK == Cy_CapSense_CheckTunerCmdIntegrity(&command_packet[0u]))
            {
                data_index = 0u;
                *tuner_packet = (uint8_t *)&cy_capsense_tuner;
                *packet = &command_packet[0u];
                break;
            }
            else
            {
                data_index--;
                for(i = 0u; i < (CY_CAPSENSE_COMMAND_PACKET_SIZE - 1u); i++)
                {
                    command_packet[i] = command_packet[i + 1u];
                }
            }
        }
    }
}

/* EZI2C interrupt handler */
static void ezi2c_isr(void)
{
    Cy_SCB_EZI2C_Interrupt(EZI2C_HW, &EZI2C_context);
}

/* [] END OF FILE */