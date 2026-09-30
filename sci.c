#include "sci_stream.h"

#include "board.h"

#define SCI_REQUESTED_BAUD_HZ    12000000UL
#define SCI_CHANNEL_COUNT        10U
#define SCI_BYTES_PER_FLOAT      4U
#define SCI_JUSTFLOAT_TAIL_SIZE  4U
#define SCI_FRAME_SIZE           ((SCI_CHANNEL_COUNT * SCI_BYTES_PER_FLOAT) + \
                                  SCI_JUSTFLOAT_TAIL_SIZE)

static uint16_t g_txFrame[SCI_FRAME_SIZE];
static volatile uint16_t g_txIndex = 0U;
static volatile bool g_txBusy = false;
static volatile uint32_t g_droppedFrames = 0UL;

static void SCI_fillTxFIFO(void)
{
    while((g_txIndex < SCI_FRAME_SIZE) &&
          (SCI_getTxFIFOStatus(sci_BASE) != SCI_FIFO_TX16))
    {
        SCI_writeCharNonBlocking(sci_BASE, g_txFrame[g_txIndex]);
        g_txIndex++;
    }
}

__interrupt void SCIA_TX_ISR(void)
{
    SCI_clearInterruptStatus(sci_BASE, SCI_INT_TXFF);

    if(g_txIndex < SCI_FRAME_SIZE)
    {
        SCI_fillTxFIFO();
    }
    else
    {
        SCI_disableInterrupt(sci_BASE, SCI_INT_TXFF);
        g_txBusy = false;
    }

    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP9);
}

void SCI_StreamingInit(void)
{
    SCI_disableInterrupt(sci_BASE, SCI_INT_TXFF | SCI_INT_RXFF);
    SCI_disableModule(sci_BASE);
    SCI_resetTxFIFO(sci_BASE);
    SCI_resetRxFIFO(sci_BASE);
    SCI_resetChannels(sci_BASE);

    /*
     * LSPCLK = 192.5 MHz and BRR = 1 produce 12.03125 Mbaud.  VOFA+ and
     * the onboard FT2232H are configured for 12.0 Mbaud (0.26% error).
     */
    SCI_setConfig(sci_BASE, DEVICE_LSPCLK_FREQ, SCI_REQUESTED_BAUD_HZ,
                  SCI_CONFIG_WLEN_8 | SCI_CONFIG_STOP_ONE |
                  SCI_CONFIG_PAR_NONE);
    SCI_setFIFOInterruptLevel(sci_BASE, SCI_FIFO_TX0, SCI_FIFO_RX0);
    SCI_enableFIFO(sci_BASE);
    SCI_enableModule(sci_BASE);
    SCI_performSoftwareReset(sci_BASE);

    Interrupt_register(INT_SCIA_TX, SCIA_TX_ISR);
    Interrupt_enable(INT_SCIA_TX);
}

bool SCI_SendFrame(const float frame[SCI_CHANNEL_COUNT])
{
    uint16_t channel;
    uint16_t byteIndex;
    uint16_t outputIndex = 0U;
    union
    {
        float value;
        uint32_t bits;
    } sample;

    if(g_txBusy)
    {
        g_droppedFrames++;
        return false;
    }

    /*
     * C28x has 16-bit addressable chars.  Convert through a uint32_t and send
     * the low 8 bits explicitly so the PC receives IEEE-754 little-endian
     * bytes, independent of the C28x char size.
     */
    for(channel = 0U; channel < SCI_CHANNEL_COUNT; channel++)
    {
        sample.value = frame[channel];
        for(byteIndex = 0U; byteIndex < SCI_BYTES_PER_FLOAT; byteIndex++)
        {
            g_txFrame[outputIndex] =
                (uint16_t)((sample.bits >> (8U * byteIndex)) & 0xFFUL);
            outputIndex++;
        }
    }

    /* VOFA+ JustFloat frame marker: 0x7F800000, little endian. */
    g_txFrame[outputIndex++] = 0x00U;
    g_txFrame[outputIndex++] = 0x00U;
    g_txFrame[outputIndex++] = 0x80U;
    g_txFrame[outputIndex]   = 0x7FU;

    g_txIndex = 0U;
    g_txBusy = true;
    SCI_clearInterruptStatus(sci_BASE, SCI_INT_TXFF);
    SCI_fillTxFIFO();
    SCI_enableInterrupt(sci_BASE, SCI_INT_TXFF);

    return true;
}

uint32_t SCI_GetDroppedFrameCount(void)
{
    return g_droppedFrames;
}
