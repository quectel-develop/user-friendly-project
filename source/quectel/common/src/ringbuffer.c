#include "ringbuffer.h"
#include "at.h"
#include "qosa_def.h"
#include "debug_service.h"
#if defined(WIN32) || defined(_WIN32) || defined(__WIN64)
#elif __linux__
#else
#include "cmsis_os.h"
#endif


/**
  * @brief  Initialize ringbuffer
  * @param  rb      Ringbuffer
  * @param  bf_pool Memory pool
  * @param  size    Data size
  * @retval Result code
  */
int16_t ringbuffer_init(ringbuffer_t* rb, uint8_t* bf_pool, uint16_t size)
{
    if(rb == NULL || bf_pool == NULL)
        return -1;

    rb->buffer = (uint8_t*)bf_pool;
    rb->buffer_size = ALIGN_DOWN(size, 4);
    rb->head   = 0;
    rb->tail   = 0;

    return 0;
}


/**
  * @brief  Calculate the length of data that has been used in the ringbuffer
  * @param  rb  Ringbuffer
  * @retval The length of data that has been used in the ringbuffer
  */
uint16_t ringbuffer_used_length(ringbuffer_t* rb)
{
    return ((rb->buffer_size - rb->head + rb->tail) % rb->buffer_size);
}


/**
  * @brief  Calculate the remaining data length of the ringbuffer
  * @param  rb  Ringbuffer
  * @retval The remaining data length of the ringbuffer
  */
uint16_t ringbuffer_remain_length(ringbuffer_t* rb)
{
    return (rb->buffer_size - ringbuffer_used_length(rb));
}


 /**
  * @brief  Write data to the ringbuffer
  * @param  rb          Ringbuffer
  * @param  data        The pointer for writing data
  * @param  data_length The length of the data to be written
  * @retval The length of the data that has been written
  */
uint16_t ringbuffer_put(ringbuffer_t* rb, const uint8_t* data, uint16_t data_length)
{
    if(rb == NULL || rb->buffer == NULL)
        return 0;

    uint16_t remain_len = rb->buffer_size - 1 - rb->tail;
    uint16_t put_len = MIN(data_length, (remain_len + rb->head) % rb->buffer_size);

    remain_len++;

    memcpy(&rb->buffer[rb->tail], data, MIN(put_len, remain_len));

    if (remain_len < put_len)
    {
        memcpy(&rb->buffer[0], data + remain_len, put_len - remain_len);
    }

    rb->tail = (rb->tail + put_len) % rb->buffer_size;

    return put_len;
}


  /**
  * @brief  Read data from the ringbuffer
  * @param  rb          Ringbuffer
  * @param  data        Pointer for storing data
  * @param  data_length The length of the data to be read
  * @retval The length of the data that has been read
  */
uint16_t ringbuffer_get(ringbuffer_t* rb, uint8_t* data, uint16_t data_length)
{
    if(rb == NULL || rb->buffer == NULL)
        return 0;

    uint16_t used_len = ringbuffer_used_length(rb);
    uint16_t read_len = MIN(data_length, used_len);
    uint16_t head = rb->head;
    uint16_t i;

    if (read_len != 0)
    {
        for (i = 0; i < read_len; i++)
        {
            data[i] = rb->buffer[head];
            head = (head + 1) % rb->buffer_size;
        }
		rb->head = (rb->head + read_len) % rb->buffer_size;
    }

    return read_len;
}
