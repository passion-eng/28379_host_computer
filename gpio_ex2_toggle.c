#include "driverlib.h"
#include "device.h"
#include "board.h"
#include "sci_stream.h"
#include "data.h"
#include "led.h"

#define SAMPLE_RATE_HZ  10000UL

static float g_signalFrame[SIGNAL_CH];

__interrupt void CPUTIMER0_ISR(void)
{
    Signal_Generate(g_signalFrame);
    (void)SCI_SendFrame(g_signalFrame);

    CPUTimer_clearOverflowFlag(CPUTIMER0_sci_BASE);
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
}

static void StreamingTimer_Init(void)
{
    CPUTimer_stopTimer(CPUTIMER0_sci_BASE);
    CPUTimer_setPreScaler(CPUTIMER0_sci_BASE, 0U);
    CPUTimer_setPeriod(CPUTIMER0_sci_BASE,
                       DEVICE_SYSCLK_FREQ / SAMPLE_RATE_HZ);
    CPUTimer_reloadTimerCounter(CPUTIMER0_sci_BASE);
    CPUTimer_clearOverflowFlag(CPUTIMER0_sci_BASE);

    Interrupt_register(INT_TIMER0, CPUTIMER0_ISR);
    Interrupt_enable(INT_TIMER0);
    CPUTimer_enableInterrupt(CPUTIMER0_sci_BASE);
}

void main(void)
{
    Device_init();
    Interrupt_initModule();
    Interrupt_initVectorTable();
    Board_init();

    LED_Init();
    Signal_Init();
    SCI_StreamingInit();
    StreamingTimer_Init();

    Interrupt_enableMaster();
    CPUTimer_startTimer(CPUTIMER0_sci_BASE);

    for(;;)
    {
        /* Sampling and transmission are interrupt driven. */
    }
}
