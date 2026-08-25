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
    rb->size   = 0;
    return 0;
}


uint16_t ringbuffer_length(ringbuffer_t* rb)
{
    return rb->size;
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

    uint16_t free_len = rb->buffer_size - rb->size;
    uint16_t write_len = MIN(data_length, free_len);
    if (write_len == 0)
        return 0;

    uint16_t tail = rb->tail;
    uint16_t contigious_len = rb->buffer_size - tail;

    if (write_len <= contigious_len)
    {
        memcpy(rb->buffer + tail, data, write_len);
        rb->tail = (tail + write_len) % rb->buffer_size;
    }
    else
    {
        memcpy(rb->buffer + tail, data, contigious_len);
        memcpy(rb->buffer, data + contigious_len, write_len - contigious_len);
        rb->tail = write_len - contigious_len;
    }
    rb->size += write_len;
    return write_len;
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

    uint16_t read_len = MIN(data_length, rb->size);
    uint16_t head = rb->head;
    if (0 == read_len)
        return 0;
    uint16_t contigious_len = rb->buffer_size - rb->head;

    if (read_len <= contigious_len)
    {
        uint8_t* src = rb->buffer + rb->head;
        memcpy(data, src, read_len);
        rb->head += read_len;
        rb->head = (head + read_len) % rb->buffer_size;
    }
    else
    {
        uint8_t* src1 = rb->buffer + rb->head;
        uint8_t* src2 = rb->buffer;
        memcpy(data, src1, contigious_len);
        memcpy(data + contigious_len, src2, read_len - contigious_len);
        rb->head = read_len - contigious_len;
    }
    rb->size -= read_len;
    return read_len;
}

