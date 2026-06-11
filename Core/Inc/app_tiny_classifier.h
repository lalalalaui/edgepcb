#ifndef APP_TINY_CLASSIFIER_H
#define APP_TINY_CLASSIFIER_H

#include <stdint.h>

#ifndef TINY_CLASSIFIER_INPUT_C
#define TINY_CLASSIFIER_INPUT_C 3
#endif
#ifndef TINY_CLASSIFIER_INPUT_H
#define TINY_CLASSIFIER_INPUT_H 96
#endif
#ifndef TINY_CLASSIFIER_INPUT_W
#define TINY_CLASSIFIER_INPUT_W 96
#endif
#ifndef TINY_CLASSIFIER_INPUT_SIZE
#define TINY_CLASSIFIER_INPUT_SIZE (3 * 96 * 96)
#endif
#ifndef TINY_CLASSIFIER_NUM_CLASSES
#define TINY_CLASSIFIER_NUM_CLASSES 6
#endif

typedef struct {
    int top1_index;
    float top1_logit;
    float top1_prob;
    float logits[TINY_CLASSIFIER_NUM_CLASSES];
    float probs[TINY_CLASSIFIER_NUM_CLASSES];
} tiny_classifier_result_t;

int app_tiny_classifier_init(void);

int app_tiny_classifier_run(
    const float *input,
    tiny_classifier_result_t *result,
    uint32_t *latency_ms
);

const char *app_tiny_classifier_last_error(void);
const char *app_tiny_classifier_class_name(int index);

#endif /* APP_TINY_CLASSIFIER_H */
