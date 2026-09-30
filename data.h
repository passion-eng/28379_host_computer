#ifndef USER_DATA_GEN_H_
#define USER_DATA_GEN_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SIGNAL_CH  10U

void Signal_Init(void);
void Signal_Generate(float out[SIGNAL_CH]);

#ifdef __cplusplus
}
#endif

#endif /* USER_DATA_GEN_H_ */
