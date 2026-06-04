#ifndef APP_AI_H
#define APP_AI_H

#include <stdint.h>

int app_ai_init(void);
int app_ai_run(const float *input, float *score, uint32_t *latency_ms);
const float *app_ai_get_output(void);
const char *app_ai_last_error(void);

#endif /* APP_AI_H */
