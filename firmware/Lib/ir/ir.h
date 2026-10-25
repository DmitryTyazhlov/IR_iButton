#ifndef IR_H_
#define IR_H_

#include <stdbool.h>
#include <stdint.h>

#define IR_MAX_PULSES 140

/* Captured or to-be-sent signal: alternating mark/space durations in us,
 * always starting with a mark. */
typedef struct {
  uint16_t pulse[IR_MAX_PULSES];
  uint16_t len;
} ir_frame_t;

typedef enum {
  IR_PROTO_UNKNOWN,
  IR_PROTO_NEC,
  IR_PROTO_NEC_REPEAT,
} ir_proto_t;

typedef struct {
  ir_proto_t proto;
  uint32_t data; /* NEC: 32 bits as received, LSB first on the wire */
} ir_decoded_t;

/* Transmit: 38 kHz carrier on IR_LED, blocking. */
void ir_tx_init(void);
void ir_send_raw(const uint16_t *pulse, uint16_t len);
void ir_send_frame(const ir_frame_t *frame);
void ir_send_nec(uint8_t address, uint8_t command);

/* Receive: TSOP demodulator on IR_REC, edge timestamps captured by EXTI. */
void ir_rx_start(void);
void ir_rx_stop(void);
/* True once a complete frame (silence > 20 ms) is stored in `frame`. */
bool ir_rx_poll(ir_frame_t *frame);

ir_decoded_t ir_decode(const ir_frame_t *frame);
static inline uint8_t ir_nec_address(uint32_t data) { return data & 0xFF; }
static inline uint8_t ir_nec_command(uint32_t data) { return (data >> 16) & 0xFF; }

/* Interrupt entry points, wired in stm32l0xx_it.c. */
void ir_tx_carrier_isr(void);
void ir_rx_edge_isr(void);

#endif
