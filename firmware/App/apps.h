#ifndef APPS_H_
#define APPS_H_

/* Each app runs its own loop and returns when BACK is pressed. */
void app_ibutton_read(void);
void app_ibutton_emulate(void);
void app_ibutton_saved(void);
void app_ir_receive(void);
void app_ir_replay(void);
void app_ir_send_nec(void);
void app_battery(void);
void app_led_test(void);
void app_sleep(void);

#endif
