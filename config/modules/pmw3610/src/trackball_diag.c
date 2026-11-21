/*
 * Copyright (c) 2026
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/device.h>
#include <zephyr/init.h>
#include <zephyr/input/input.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/atomic.h>

LOG_MODULE_REGISTER(trackball_diag, LOG_LEVEL_INF);

static atomic_t event_count;
static atomic_t delta_x;
static atomic_t delta_y;
static atomic_t wheel_count;

static void trackball_input_callback(struct input_event *event) {
    atomic_inc(&event_count);

    if (event->type != INPUT_EV_REL) {
        return;
    }

    switch (event->code) {
    case INPUT_REL_X:
        atomic_add(&delta_x, event->value);
        break;
    case INPUT_REL_Y:
        atomic_add(&delta_y, event->value);
        break;
    case INPUT_REL_WHEEL:
    case INPUT_REL_HWHEEL:
        atomic_inc(&wheel_count);
        break;
    }
}

#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
INPUT_CALLBACK_DEFINE(DEVICE_DT_GET(DT_NODELABEL(trackball_split)), trackball_input_callback);
#define TRACKBALL_DIAG_SOURCE "central-split-input"
#else
INPUT_CALLBACK_DEFINE(DEVICE_DT_GET(DT_NODELABEL(trackball)), trackball_input_callback);
#define TRACKBALL_DIAG_SOURCE "peripheral-sensor"
#endif

static void trackball_diag_work_handler(struct k_work *work) {
    atomic_val_t events = atomic_set(&event_count, 0);
    atomic_val_t x = atomic_set(&delta_x, 0);
    atomic_val_t y = atomic_set(&delta_y, 0);
    atomic_val_t wheels = atomic_set(&wheel_count, 0);

    LOG_INF("Input summary (%s): events=%d dx=%d dy=%d wheel=%d", TRACKBALL_DIAG_SOURCE,
            (int)events, (int)x, (int)y, (int)wheels);
}

K_WORK_DEFINE(trackball_diag_work, trackball_diag_work_handler);

static void trackball_diag_timer_handler(struct k_timer *timer) {
    k_work_submit(&trackball_diag_work);
}

K_TIMER_DEFINE(trackball_diag_timer, trackball_diag_timer_handler, NULL);

static int trackball_diag_init(void) {
    k_timer_start(&trackball_diag_timer, K_SECONDS(2), K_SECONDS(2));
    return 0;
}

SYS_INIT(trackball_diag_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
