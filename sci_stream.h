#ifndef SCI_STREAMING_H_
#define SCI_STREAMING_H_

#include <stdbool.h>
#include <stdint.h>

#include "driverlib.h"
#include "device.h"

#ifdef __cplusplus
extern "C" {
#endif

void SCI_StreamingInit(void);
bool SCI_SendFrame(const float frame[10U]);
uint32_t SCI_GetDroppedFrameCount(void);

#ifdef __cplusplus
}
#endif

#endif /* SCI_STREAMING_H_ */
