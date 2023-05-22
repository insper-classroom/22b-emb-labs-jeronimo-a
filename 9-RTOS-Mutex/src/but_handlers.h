#include <asf.h>
#include <string.h>
#include "ili9341.h"
#include "lvgl.h"
#include "touch/touch.h"

static void power_btn_handler(lv_event_t *);
static void m_btn_handler(lv_event_t *);
static void clock_btn_handler(lv_event_t *);
static void up_btn_handler(lv_event_t *);
static void down_btn_handler(lv_event_t *);