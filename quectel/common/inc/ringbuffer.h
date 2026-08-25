#ifndef __RINGBUFFER_H__
#define __RINGBUFFER_H__
#include <stdint.h>
#include <string.h>

#define RINGBUFFER_SIZE     (5 * 1024)

typedef struct ringbuffer
{
  uint8_t*  buffer;
  uint16_t  buffer_size;
  uint16_t  size;
  volatile uint16_t  head;
  volatile uint16_t  tail;
}ringbuffer_t;


int16_t  ringbuffer_init(ringbuffer_t* rb, uint8_t* buffer, uint16_t size);
uint16_t ringbuffer_length(ringbuffer_t* rb);
uint16_t ringbuffer_put(ringbuffer_t* rb, const uint8_t* data, uint16_t data_length);
uint16_t ringbuffer_get(ringbuffer_t* rb, uint8_t* data, uint16_t data_length);

#endif /* __RINGBUFFER_H__ */
