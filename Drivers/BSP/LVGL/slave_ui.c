#include "./BSP/LVGL/slave_ui.h"

#include <stdio.h>
#include <string.h>
#include "stm32h7xx_hal.h"

#define SLAVE_STATION_COUNT       8U
#define SMS_TEXT_MAX              96U

#define TOP_H                     52
#define STATUS_H                  24
#define NAV_H                     48
#define PAD                       8

typedef enum {
    PAGE_HOME = 0,
    PAGE_RX,
    PAGE_SMS,
    PAGE_SETUP,
    PAGE_COUNT
} slave_page_t;

typedef struct {
    uint8_t station_id;
    bool group_enabled;
    bool carrier_detected;
    bool selected_call;
    bool group_call;
    bool muted;
    uint8_t volume;
    uint8_t squelch;
    uint8_t af_level;
    int16_t rssi_dbm;
    uint16_t battery_mv;
    uint8_t battery_percent;
    uint32_t packet_count;
    uint8_t last_sender;
    char sms_text[SMS_TEXT_MAX];
} slave_ui_state_t;

typedef struct {
    lv_obj_t *root;
    lv_obj_t *top;
    lv_obj_t *status;
    lv_obj_t *content;
    lv_obj_t *nav_btn[PAGE_COUNT];
    lv_obj_t *label_rx;
    lv_obj_t *label_id;
    lv_obj_t *label_batt;
    lv_obj_t *label_rssi;
    lv_obj_t *label_uptime;
    lv_obj_t *label_status;
    lv_obj_t *label_station_value;
    lv_obj_t *label_group_value;
    lv_obj_t *bar_af;
    lv_obj_t *bar_rssi;
    slave_page_t page;
    slave_ui_state_t state;
} slave_ui_t;

static slave_ui_t g_ui;

#define C_BG        lv_color_hex(0x0F1419)
#define C_PANEL     lv_color_hex(0x172028)
#define C_PANEL_2   lv_color_hex(0x1F2A33)
#define C_LINE      lv_color_hex(0x344450)
#define C_TEXT      lv_color_hex(0xF4F7FA)
#define C_MUTED     lv_color_hex(0x9DAAB5)
#define C_ACCENT    lv_color_hex(0x42C6E8)
#define C_WARN      lv_color_hex(0xF2C24B)
#define C_OK        lv_color_hex(0x64D48E)
#define C_BAD       lv_color_hex(0xE96666)

static void render_page(slave_page_t page);

static lv_obj_t *make_obj(lv_obj_t *parent, int32_t x, int32_t y, int32_t w, int32_t h, lv_color_t bg)
{
    lv_obj_t *obj = lv_obj_create(parent);
    lv_obj_set_pos(obj, x, y);
    lv_obj_set_size(obj, w, h);
    lv_obj_set_style_bg_color(obj, bg, 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(obj, 0, 0);
    lv_obj_set_style_pad_all(obj, 0, 0);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);
    return obj;
}

static lv_obj_t *make_label(lv_obj_t *parent,
                            const char *text,
                            int32_t x,
                            int32_t y,
                            const lv_font_t *font,
                            lv_color_t color)
{
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, text);
    lv_obj_set_pos(label, x, y);
    lv_obj_set_style_text_color(label, color, 0);
    if (font != NULL) {
        lv_obj_set_style_text_font(label, font, 0);
    }
    return label;
}

static lv_obj_t *make_panel(lv_obj_t *parent, int32_t x, int32_t y, int32_t w, int32_t h, const char *title)
{
    lv_obj_t *panel = make_obj(parent, x, y, w, h, C_PANEL);
    lv_obj_set_style_radius(panel, 6, 0);
    lv_obj_set_style_border_width(panel, 1, 0);
    lv_obj_set_style_border_color(panel, C_LINE, 0);
    if (title != NULL) {
        make_label(panel, title, 10, 8, &lv_font_montserrat_14, C_MUTED);
        make_obj(panel, 10, 31, w - 20, 1, C_LINE);
    }
    return panel;
}

static lv_obj_t *make_button(lv_obj_t *parent,
                             const char *text,
                             int32_t x,
                             int32_t y,
                             int32_t w,
                             int32_t h,
                             lv_color_t bg,
                             lv_event_cb_t cb)
{
    lv_obj_t *btn = lv_button_create(parent);
    lv_obj_set_pos(btn, x, y);
    lv_obj_set_size(btn, w, h);
    lv_obj_set_style_bg_color(btn, bg, 0);
    lv_obj_set_style_radius(btn, 5, 0);
    lv_obj_set_style_shadow_width(btn, 0, 0);
    lv_obj_set_style_border_width(btn, 1, 0);
    lv_obj_set_style_border_color(btn, C_LINE, 0);
    lv_obj_clear_flag(btn, LV_OBJ_FLAG_SCROLLABLE);
    if (cb != NULL) {
        lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, NULL);
    }

    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, C_TEXT, 0);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
    lv_obj_center(label);
    return btn;
}

static lv_obj_t *make_value_card(lv_obj_t *parent,
                                 int32_t x,
                                 int32_t y,
                                 int32_t w,
                                 int32_t h,
                                 const char *name,
                                 const char *value,
                                 lv_color_t color)
{
    lv_obj_t *panel = make_panel(parent, x, y, w, h, NULL);
    make_label(panel, name, 10, 8, &lv_font_montserrat_14, C_MUTED);
    lv_obj_t *label = make_label(panel, value, 10, 32, &lv_font_montserrat_18, color);
    lv_obj_set_width(label, w - 20);
    lv_label_set_long_mode(label, LV_LABEL_LONG_CLIP);
    return panel;
}

static lv_obj_t *make_row(lv_obj_t *parent, int32_t y, const char *name, const char *value, lv_color_t color)
{
    make_label(parent, name, 12, y, &lv_font_montserrat_14, C_MUTED);
    lv_obj_t *label = make_label(parent, value, 118, y, &lv_font_montserrat_14, color);
    lv_obj_set_width(label, 170);
    lv_label_set_long_mode(label, LV_LABEL_LONG_CLIP);
    return label;
}

static int32_t rssi_to_percent(int16_t dbm)
{
    if (dbm <= -120) {
        return 0;
    }
    if (dbm >= -40) {
        return 100;
    }
    return ((int32_t)dbm + 120) * 100 / 80;
}

static const char *rx_status_text(void)
{
    if (!g_ui.state.carrier_detected) {
        return "IDLE";
    }
    if (g_ui.state.selected_call) {
        return g_ui.state.group_call ? "GROUP" : "CALL";
    }
    return "CARRIER";
}

static void update_top(void)
{
    char buf[40];
    lv_color_t rx_color = g_ui.state.selected_call ? C_OK :
                          (g_ui.state.carrier_detected ? C_WARN : C_BAD);

    if (g_ui.label_rx != NULL) {
        lv_label_set_text(g_ui.label_rx, rx_status_text());
        lv_obj_set_style_text_color(g_ui.label_rx, rx_color, 0);
    }

    if (g_ui.label_id != NULL) {
        snprintf(buf, sizeof(buf), "ID:%u", g_ui.state.station_id);
        lv_label_set_text(g_ui.label_id, buf);
    }

    if (g_ui.label_batt != NULL) {
        snprintf(buf, sizeof(buf), "BAT:%u%%", g_ui.state.battery_percent);
        lv_label_set_text(g_ui.label_batt, buf);
        lv_obj_set_style_text_color(g_ui.label_batt,
                                    g_ui.state.battery_percent < 20U ? C_BAD : C_OK,
                                    0);
    }

    if (g_ui.label_rssi != NULL) {
        snprintf(buf, sizeof(buf), "%ddBm", (int)g_ui.state.rssi_dbm);
        lv_label_set_text(g_ui.label_rssi, buf);
    }

    if (g_ui.label_station_value != NULL) {
        snprintf(buf, sizeof(buf), "%u", g_ui.state.station_id);
        lv_label_set_text(g_ui.label_station_value, buf);
    }

    if (g_ui.label_group_value != NULL) {
        lv_label_set_text(g_ui.label_group_value, g_ui.state.group_enabled ? "ON" : "OFF");
        lv_obj_set_style_text_color(g_ui.label_group_value,
                                    g_ui.state.group_enabled ? C_OK : C_WARN,
                                    0);
    }

    if (g_ui.bar_af != NULL) {
        lv_bar_set_value(g_ui.bar_af, g_ui.state.af_level, LV_ANIM_OFF);
    }

    if (g_ui.bar_rssi != NULL) {
        lv_bar_set_value(g_ui.bar_rssi, rssi_to_percent(g_ui.state.rssi_dbm), LV_ANIM_OFF);
    }
}

static void nav_event(lv_event_t *event)
{
    uintptr_t raw = (uintptr_t)lv_obj_get_user_data(lv_event_get_target(event));
    render_page((slave_page_t)raw);
}

static void station_minus_event(lv_event_t *event)
{
    (void)event;
    if (g_ui.state.station_id == 0U) {
        g_ui.state.station_id = SLAVE_STATION_COUNT - 1U;
    } else {
        g_ui.state.station_id--;
    }
    update_top();
}

static void station_plus_event(lv_event_t *event)
{
    (void)event;
    g_ui.state.station_id = (uint8_t)((g_ui.state.station_id + 1U) % SLAVE_STATION_COUNT);
    update_top();
}

static void group_toggle_event(lv_event_t *event)
{
    (void)event;
    g_ui.state.group_enabled = !g_ui.state.group_enabled;
    update_top();
    render_page(PAGE_SETUP);
}

static void mute_toggle_event(lv_event_t *event)
{
    (void)event;
    g_ui.state.muted = !g_ui.state.muted;
    render_page(g_ui.page);
}

static void volume_minus_event(lv_event_t *event)
{
    (void)event;
    if (g_ui.state.volume >= 5U) {
        g_ui.state.volume -= 5U;
    }
    render_page(g_ui.page);
}

static void volume_plus_event(lv_event_t *event)
{
    (void)event;
    if (g_ui.state.volume <= 95U) {
        g_ui.state.volume += 5U;
    }
    render_page(g_ui.page);
}

static void sms_clear_event(lv_event_t *event)
{
    (void)event;
    g_ui.state.sms_text[0] = '\0';
    render_page(PAGE_SMS);
}

static void simulate_call_event(lv_event_t *event)
{
    (void)event;
    g_ui.state.carrier_detected = true;
    g_ui.state.selected_call = true;
    g_ui.state.group_call = false;
    g_ui.state.rssi_dbm = -62;
    g_ui.state.af_level = 58;
    update_top();
    render_page(PAGE_HOME);
}

static void simulate_sms_event(lv_event_t *event)
{
    (void)event;
    slave_ui_set_sms("HELLO STATION 3", 0U, false);
    render_page(PAGE_SMS);
}

static void update_nav(void)
{
    for (uint8_t i = 0; i < PAGE_COUNT; i++) {
        lv_color_t bg = (i == (uint8_t)g_ui.page) ? C_ACCENT : C_PANEL_2;
        lv_color_t fg = (i == (uint8_t)g_ui.page) ? C_BG : C_TEXT;
        lv_obj_set_style_bg_color(g_ui.nav_btn[i], bg, 0);
        lv_obj_set_style_text_color(lv_obj_get_child(g_ui.nav_btn[i], 0), fg, 0);
    }
}

static void render_home(void)
{
    lv_obj_t *c = g_ui.content;
    char value[40];

    make_value_card(c, 8, 8, 186, 76, "Frequency", "35.000 MHz", C_ACCENT);
    make_value_card(c, 202, 8, 186, 76, "RX State", rx_status_text(),
                    g_ui.state.selected_call ? C_OK : C_WARN);

    snprintf(value, sizeof(value), "S%u%s", g_ui.state.station_id,
             g_ui.state.group_enabled ? " + GRP" : "");
    make_value_card(c, 396, 8, 186, 76, "Address", value, C_TEXT);

    snprintf(value, sizeof(value), "%u%%", g_ui.state.battery_percent);
    make_value_card(c, 590, 8, 202, 76, "Battery", value,
                    g_ui.state.battery_percent < 20U ? C_BAD : C_OK);

    lv_obj_t *rx = make_panel(c, 8, 94, 380, 250, "Receive Monitor");
    g_ui.bar_rssi = lv_bar_create(rx);
    lv_obj_set_pos(g_ui.bar_rssi, 18, 58);
    lv_obj_set_size(g_ui.bar_rssi, 248, 22);
    lv_bar_set_range(g_ui.bar_rssi, 0, 100);
    lv_obj_set_style_bg_color(g_ui.bar_rssi, C_PANEL_2, 0);
    lv_obj_set_style_bg_color(g_ui.bar_rssi, C_ACCENT, LV_PART_INDICATOR);
    make_label(rx, "RSSI", 286, 59, &lv_font_montserrat_14, C_MUTED);

    g_ui.bar_af = lv_bar_create(rx);
    lv_obj_set_pos(g_ui.bar_af, 18, 112);
    lv_obj_set_size(g_ui.bar_af, 248, 22);
    lv_bar_set_range(g_ui.bar_af, 0, 100);
    lv_obj_set_style_bg_color(g_ui.bar_af, C_PANEL_2, 0);
    lv_obj_set_style_bg_color(g_ui.bar_af, C_OK, LV_PART_INDICATOR);
    make_label(rx, "AF", 286, 113, &lv_font_montserrat_14, C_MUTED);

    make_row(rx, 156, "Demod", "AM audio", C_TEXT);
    make_row(rx, 184, "Audio out", g_ui.state.muted ? "Muted" : "Headphone", g_ui.state.muted ? C_WARN : C_OK);
    make_row(rx, 212, "Data", "AFSK 300bps", C_TEXT);

    lv_obj_t *msg = make_panel(c, 396, 94, 396, 250, "Last Message");
    snprintf(value, sizeof(value), "From: %s%u",
             g_ui.state.group_call ? "Group/S" : "S",
             g_ui.state.last_sender);
    make_label(msg, value, 14, 48, &lv_font_montserrat_14, C_MUTED);

    lv_obj_t *sms = make_label(msg,
                               g_ui.state.sms_text[0] ? g_ui.state.sms_text : "(no received SMS)",
                               14, 82,
                               &lv_font_montserrat_18,
                               g_ui.state.sms_text[0] ? C_TEXT : C_MUTED);
    lv_obj_set_width(sms, 360);
    lv_label_set_long_mode(sms, LV_LABEL_LONG_WRAP);
    snprintf(value, sizeof(value), "%lu", (unsigned long)g_ui.state.packet_count);
    make_row(msg, 190, "Packets", value, C_TEXT);
    make_button(msg, g_ui.state.muted ? "UNMUTE" : "MUTE", 14, 202, 110, 34,
                g_ui.state.muted ? C_WARN : C_PANEL_2, mute_toggle_event);

    update_top();
}

static void render_rx(void)
{
    lv_obj_t *c = g_ui.content;
    char value[40];

    lv_obj_t *left = make_panel(c, 8, 8, 384, 336, "Voice Receive");
    make_row(left, 48, "Carrier", g_ui.state.carrier_detected ? "Detected" : "Idle",
             g_ui.state.carrier_detected ? C_OK : C_MUTED);
    make_row(left, 78, "Selected", g_ui.state.selected_call ? "Yes" : "No",
             g_ui.state.selected_call ? C_OK : C_WARN);
    make_row(left, 108, "Call type", g_ui.state.group_call ? "Group" : "Station", C_TEXT);
    make_row(left, 138, "Headphone", g_ui.state.muted ? "Muted" : "Enabled",
             g_ui.state.muted ? C_WARN : C_OK);

    snprintf(value, sizeof(value), "%u%%", g_ui.state.volume);
    make_row(left, 168, "Volume", value, C_TEXT);
    make_button(left, "-", 18, 204, 58, 42, C_PANEL_2, volume_minus_event);
    make_button(left, "+", 90, 204, 58, 42, C_PANEL_2, volume_plus_event);
    make_button(left, g_ui.state.muted ? "UNMUTE" : "MUTE", 166, 204, 120, 42,
                g_ui.state.muted ? C_WARN : C_PANEL_2, mute_toggle_event);

    lv_obj_t *bar = lv_bar_create(left);
    lv_obj_set_pos(bar, 18, 272);
    lv_obj_set_size(bar, 320, 24);
    lv_bar_set_range(bar, 0, 100);
    lv_bar_set_value(bar, g_ui.state.af_level, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(bar, C_PANEL_2, 0);
    lv_obj_set_style_bg_color(bar, C_OK, LV_PART_INDICATOR);
    make_label(left, "AF level", 18, 250, &lv_font_montserrat_14, C_MUTED);

    lv_obj_t *right = make_panel(c, 400, 8, 392, 336, "Receiver Chain");
    make_row(right, 48, "RF band", "30-40 MHz", C_OK);
    make_row(right, 78, "RX freq", "35.000 MHz", C_ACCENT);
    make_row(right, 108, "Antenna", "<= 1m", C_OK);
    make_row(right, 138, "Demod", "AM envelope", C_TEXT);
    make_row(right, 168, "SMS modem", "AFSK 1200/2200", C_TEXT);
    make_row(right, 198, "Squelch", "Manual", C_WARN);
    make_row(right, 228, "Distance", ">= 5m target", C_WARN);
    make_button(right, "SIM CALL", 18, 276, 130, 42, C_ACCENT, simulate_call_event);
    make_button(right, "SIM SMS", 168, 276, 130, 42, C_PANEL_2, simulate_sms_event);
}

static void render_sms(void)
{
    lv_obj_t *c = g_ui.content;
    char value[48];

    lv_obj_t *inbox = make_panel(c, 8, 8, 502, 336, "SMS Inbox");
    snprintf(value, sizeof(value), "Last sender: S%u", g_ui.state.last_sender);
    make_label(inbox, value, 14, 48, &lv_font_montserrat_14, C_MUTED);
    make_label(inbox, g_ui.state.group_call ? "Frame: GROUP + ASCII + CRC" : "Frame: ADDR + ASCII + CRC",
               14, 76, &lv_font_montserrat_14, C_MUTED);

    lv_obj_t *body = make_panel(inbox, 14, 112, 474, 140, NULL);
    lv_obj_t *sms = make_label(body,
                               g_ui.state.sms_text[0] ? g_ui.state.sms_text : "(waiting for English SMS)",
                               12, 14,
                               &lv_font_montserrat_18,
                               g_ui.state.sms_text[0] ? C_TEXT : C_MUTED);
    lv_obj_set_width(sms, 448);
    lv_label_set_long_mode(sms, LV_LABEL_LONG_WRAP);

    make_button(inbox, "CLEAR", 14, 278, 112, 40, C_WARN, sms_clear_event);
    make_button(inbox, "SIM SMS", 146, 278, 112, 40, C_ACCENT, simulate_sms_event);

    lv_obj_t *status = make_panel(c, 518, 8, 274, 336, "Data Status");
    snprintf(value, sizeof(value), "%lu", (unsigned long)g_ui.state.packet_count);
    make_row(status, 48, "Packets", value, C_TEXT);
    make_row(status, 78, "Data rate", "300 bps", C_TEXT);
    make_row(status, 108, "Mark", "1200 Hz", C_TEXT);
    make_row(status, 138, "Space", "2200 Hz", C_TEXT);
    make_row(status, 168, "Charset", "ASCII", C_OK);
    make_row(status, 198, "Target", "My ID/Group", C_ACCENT);
    make_row(status, 228, "CRC", "Required", C_WARN);
}

static void render_setup(void)
{
    lv_obj_t *c = g_ui.content;
    char value[40];

    lv_obj_t *addr = make_panel(c, 8, 8, 384, 336, "Station Address");
    make_label(addr, "Station ID", 20, 54, &lv_font_montserrat_14, C_MUTED);
    g_ui.label_station_value = make_label(addr, "0", 154, 48, &lv_font_montserrat_18, C_ACCENT);
    make_button(addr, "-", 20, 92, 72, 48, C_PANEL_2, station_minus_event);
    make_button(addr, "+", 110, 92, 72, 48, C_PANEL_2, station_plus_event);
    make_label(addr, "Range: S0-S7", 204, 108, &lv_font_montserrat_14, C_MUTED);

    make_label(addr, "Accept group call", 20, 174, &lv_font_montserrat_14, C_MUTED);
    g_ui.label_group_value = make_label(addr, "ON", 184, 174, &lv_font_montserrat_18, C_OK);
    make_button(addr, "TOGGLE", 20, 214, 140, 42, C_PANEL_2, group_toggle_event);
    lv_obj_t *note = make_label(addr, "Save ID in RF/config layer.", 20, 286, &lv_font_montserrat_14, C_WARN);
    lv_obj_set_width(note, 340);
    lv_label_set_long_mode(note, LV_LABEL_LONG_WRAP);

    lv_obj_t *sys = make_panel(c, 400, 8, 392, 336, "System Checklist");
    snprintf(value, sizeof(value), "%umV / %u%%", g_ui.state.battery_mv, g_ui.state.battery_percent);
    make_row(sys, 48, "Battery", value, g_ui.state.battery_percent < 20U ? C_BAD : C_OK);
    make_row(sys, 78, "Power", "Battery only", C_OK);
    make_row(sys, 108, "Audio out", "Headphone", C_OK);
    make_row(sys, 138, "RX freq", "Match master", C_ACCENT);
    make_row(sys, 168, "Direct test", "20dB attenuator", C_WARN);
    make_row(sys, 198, "Antenna", "<= 1m", C_OK);
    make_row(sys, 228, "RF decode", "TODO", C_WARN);
    make_row(sys, 258, "SMS parser", "TODO", C_WARN);
}

static void render_page(slave_page_t page)
{
    if (page >= PAGE_COUNT) {
        return;
    }

    g_ui.page = page;
    g_ui.label_station_value = NULL;
    g_ui.label_group_value = NULL;
    g_ui.bar_af = NULL;
    g_ui.bar_rssi = NULL;
    lv_obj_clean(g_ui.content);

    switch (page) {
    case PAGE_HOME:
        render_home();
        break;
    case PAGE_RX:
        render_rx();
        break;
    case PAGE_SMS:
        render_sms();
        break;
    case PAGE_SETUP:
        render_setup();
        break;
    default:
        break;
    }

    update_nav();
    update_top();
}

static void timer_event(lv_timer_t *timer)
{
    (void)timer;
    char buf[48];
    uint32_t sec = HAL_GetTick() / 1000U;
    uint32_t h = sec / 3600U;
    uint32_t m = (sec % 3600U) / 60U;
    uint32_t s = sec % 60U;

    if (g_ui.label_uptime != NULL) {
        snprintf(buf, sizeof(buf), "%02lu:%02lu:%02lu",
                 (unsigned long)h,
                 (unsigned long)m,
                 (unsigned long)s);
        lv_label_set_text(g_ui.label_uptime, buf);
    }

    if (g_ui.label_status != NULL) {
        snprintf(buf, sizeof(buf), "RSSI %ddBm  AF %u%%",
                 (int)g_ui.state.rssi_dbm,
                 g_ui.state.af_level);
        lv_label_set_text(g_ui.label_status, buf);
    }

    update_top();
}

void slave_ui_create(lv_obj_t *parent)
{
    static const char *nav_names[PAGE_COUNT] = {"HOME", "VOICE", "SMS", "SETUP"};
    int32_t sw;
    int32_t sh;

    memset(&g_ui, 0, sizeof(g_ui));
    g_ui.root = parent;
    g_ui.state.station_id = 3U;
    g_ui.state.group_enabled = true;
    g_ui.state.rssi_dbm = -96;
    g_ui.state.af_level = 0U;
    g_ui.state.volume = 65U;
    g_ui.state.squelch = 35U;
    g_ui.state.battery_mv = 7400U;
    g_ui.state.battery_percent = 84U;
    g_ui.state.last_sender = 0U;

    lv_obj_clean(parent);
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(parent, C_BG, 0);
    lv_obj_set_style_pad_all(parent, 0, 0);

    sw = lv_obj_get_width(parent);
    sh = lv_obj_get_height(parent);
    if (sw <= 0) {
        sw = 800;
    }
    if (sh <= 0) {
        sh = 480;
    }

    g_ui.top = make_obj(parent, 0, 0, sw, TOP_H, C_BG);
    make_label(g_ui.top, "Wireless Call Slave", 12, 8, &lv_font_montserrat_18, C_TEXT);
    make_label(g_ui.top, "35.000MHz  AM  AFSK", 12, 31, &lv_font_montserrat_14, C_ACCENT);
    make_label(g_ui.top, "RX:", sw - 302, 10, &lv_font_montserrat_14, C_MUTED);
    g_ui.label_rx = make_label(g_ui.top, "IDLE", sw - 268, 10, &lv_font_montserrat_14, C_BAD);
    g_ui.label_id = make_label(g_ui.top, "ID:3", sw - 190, 10, &lv_font_montserrat_14, C_TEXT);
    g_ui.label_batt = make_label(g_ui.top, "BAT:84%", sw - 120, 10, &lv_font_montserrat_14, C_OK);
    g_ui.label_rssi = make_label(g_ui.top, "-96dBm", sw - 120, 31, &lv_font_montserrat_14, C_MUTED);
    make_obj(g_ui.top, 0, TOP_H - 1, sw, 1, C_LINE);

    g_ui.content = make_obj(parent, 0, TOP_H, sw, sh - TOP_H - STATUS_H - NAV_H, C_BG);

    lv_obj_t *nav = make_obj(parent, 0, sh - NAV_H, sw, NAV_H, C_BG);
    make_obj(nav, 0, 0, sw, 1, C_LINE);
    for (uint8_t i = 0; i < PAGE_COUNT; i++) {
        int32_t btn_w = (sw - (PAGE_COUNT + 1) * PAD) / PAGE_COUNT;
        lv_obj_t *btn = make_button(nav,
                                    nav_names[i],
                                    PAD + i * (btn_w + PAD),
                                    7,
                                    btn_w,
                                    34,
                                    C_PANEL_2,
                                    nav_event);
        lv_obj_set_user_data(btn, (void *)(uintptr_t)i);
        g_ui.nav_btn[i] = btn;
    }

    g_ui.status = make_obj(parent, 0, sh - NAV_H - STATUS_H, sw, STATUS_H, C_BG);
    make_obj(g_ui.status, 0, 0, sw, 1, C_LINE);
    lv_obj_t *bottom_info = make_label(g_ui.status, "Uptime:", 12, 5, &lv_font_montserrat_14, C_MUTED);
    g_ui.label_uptime = make_label(g_ui.status, "00:00:00", 78, 5, &lv_font_montserrat_14, C_TEXT);
    g_ui.label_status = make_label(g_ui.status, "RSSI -96dBm  AF 0%", sw - 190, 5, &lv_font_montserrat_14, C_MUTED);
    (void)bottom_info;

    lv_timer_create(timer_event, 250, NULL);
    render_page(PAGE_HOME);
}

void slave_ui_set_rx_state(bool carrier_detected,
                           bool selected_call,
                           bool group_call,
                           int16_t rssi_dbm,
                           uint8_t af_level)
{
    g_ui.state.carrier_detected = carrier_detected;
    g_ui.state.selected_call = selected_call;
    g_ui.state.group_call = group_call;
    g_ui.state.rssi_dbm = rssi_dbm;
    g_ui.state.af_level = af_level > 100U ? 100U : af_level;
    update_top();
}

void slave_ui_set_sms(const char *text, uint8_t sender_id, bool group_call)
{
    if (text == NULL) {
        text = "";
    }
    snprintf(g_ui.state.sms_text, sizeof(g_ui.state.sms_text), "%s", text);
    g_ui.state.last_sender = sender_id % SLAVE_STATION_COUNT;
    g_ui.state.group_call = group_call;
    g_ui.state.packet_count++;
    g_ui.state.carrier_detected = true;
    g_ui.state.selected_call = true;
    update_top();
}

void slave_ui_set_battery(uint16_t millivolts, uint8_t percent)
{
    g_ui.state.battery_mv = millivolts;
    g_ui.state.battery_percent = percent > 100U ? 100U : percent;
    update_top();
}

uint8_t slave_ui_get_station_id(void)
{
    return g_ui.state.station_id;
}

bool slave_ui_group_enabled(void)
{
    return g_ui.state.group_enabled;
}
