#ifndef PCB_AI_UI_H
#define PCB_AI_UI_H

#include <stdint.h>

void pcb_ai_ui_create(void);
void pcb_ai_ui_set_status(const char *text);
void pcb_ai_ui_set_run_callback(void (*cb)(void));
void pcb_ai_ui_set_next_callback(void (*cb)(void));
void pcb_ai_ui_show_input_patch(const float *input);
void pcb_ai_ui_show_heatmap(const float *input, const float *output);
void pcb_ai_ui_update_result(
    float score,
    float threshold_low,
    float threshold_high,
    int result_code,
    uint32_t latency_ms,
    int sample_index,
    int sample_count,
    int expected_label,
    const char *compare_text
);

#endif /* PCB_AI_UI_H */
