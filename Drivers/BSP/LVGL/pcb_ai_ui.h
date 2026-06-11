#ifndef PCB_AI_UI_H
#define PCB_AI_UI_H

#include <stdint.h>

#define PCB_AI_UI_MODE_ANOMALY    0
#define PCB_AI_UI_MODE_CLASSIFIER 1

void pcb_ai_ui_create(void);
void pcb_ai_ui_set_status(const char *text);
void pcb_ai_ui_set_run_callback(void (*cb)(void));
void pcb_ai_ui_set_next_callback(void (*cb)(void));
void pcb_ai_ui_set_anomaly_callback(void (*cb)(void));
void pcb_ai_ui_set_classifier_callback(void (*cb)(void));
void pcb_ai_ui_set_mode(int mode);
void pcb_ai_ui_update_sample_info(int sample_index, int sample_count, int expected_label);
void pcb_ai_ui_show_input_patch(const float *input);
void pcb_ai_ui_show_heatmap(const float *input, const float *output);
void pcb_ai_ui_clear_heatmap(void);
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
void pcb_ai_ui_update_classifier_result(
    int sample_index,
    int sample_count,
    int expected_label,
    const char *predicted_class,
    float top1_prob,
    const char *top3_line0,
    const char *top3_line1,
    const char *top3_line2,
    uint32_t latency_ms
);

#endif /* PCB_AI_UI_H */
