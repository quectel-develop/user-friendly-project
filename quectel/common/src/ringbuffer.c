/****************************************************************************
*
* Copy right: 2020-, Copyrigths of Quectel Ltd.
****************************************************************************
* File name: ringbuffer.c
* History: Rev1.0 2020-12-1
****************************************************************************/

#include "ringbuffer.h"
#include "debug_service.h"
#include "at.h"
#include "qosa_def.h"
#if defined(WIN32) || defined(_WIN32) || defined(__WIN64)
#elif __linux__
#else
#include "cmsis_os.h"
#endif

#define ringbuffer_space_len(rb) ((rb)->buffer_size - ringbuffer_data_len(rb))

// ���λΪ���ڽ�����ɱ��λ,��14λΪ��ǰ�������ݳ���
// volatile uint16_t g_uart_recv_sta = 0;	

// /**
//  * @description: ��ǽ������
//  * @param  none
//  * @return none
//  */
// void set_uart_recv_sta(void)
// {
// 	g_uart_recv_sta |= 1<<15;
// }

// /**
//  * @description: ���ص������ݽ��ճ���
//  * @param  none
//  * @return none
//  */
// uint16_t get_uart_recv_data_len(void)
// {
// 	return g_uart_recv_sta; 		
// }

// /**
//  * @description: ��ѯ��ǽ���״̬λ
//  * @param  none
//  * @return none
//  */
// uint16_t get_uart_recv_sta(void)
// {
// 	return (g_uart_recv_sta&(1<<15));
// }

// /**
//  * @description: �������ݳ�������
//  * @param  none
//  * @return none
//  */
// void set_uart_recv_data_len(void)
// {
// 	g_uart_recv_sta ++;	
// }

// /**
//  * @description: ������
//  * @param  none
//  * @return none
//  */
// void uart_recv_sta_clean(void)
// {
// 	g_uart_recv_sta = 0;	
// }

// /**
//  * @description: ������ʱ������(������,���û�ʵ��) 
//  * @param  none
//  * @return none
//  */
// __weak void tim_start_count(void)
// {
// 	/*ע�⣺����Ҫ�ص�ʱ,��Ӧ�޸Ĵ˺���,tim_start_count() ��Ҫ���û��ļ���ʵ��*/
// 	/* �ú�����������TIM��ʱ�� */
// 	LOG_E("please implement this function< tim_start_count() > in the user program!");
// }

// /**
//  * @description: �����ʱ������(������,���û�ʵ��) 
//  * @param  none
//  * @return none
//  */
// __weak void tim_clear_count(void)
// {
// 	/*ע�⣺����Ҫ�ص�ʱ,��Ӧ�޸Ĵ˺���,tim_clear_count() ��Ҫ���û��ļ���ʵ��*/
// 	/* �ú������ڽ�TIM��ʱ���������� */
// 	LOG_E("please implement this function< tim_clear_count() > in the user program!");
// }

// /**
// * @description:  �������ݵ�rb
//  * @param  ch ���洢����
//  * @return none
//  */
// void rb_data_putchar(const uint8_t ch)
// {
// 	if( 0 == get_uart_recv_sta() ) 						// ����δ���
// 	{
// 		if(get_uart_recv_data_len() < BUFF_SIZE_MAX)
// 		{
// 			tim_clear_count();
// 			if( 0 == get_uart_recv_data_len() )
// 			{
// 				tim_start_count();						// ʹ�ܶ�ʱ��7���ж�
// 			}
// 			ringbuffer_putstr(&g_uart_rxcb,&ch,1);
// 			// ���յ�һ�ֽ�����,���г����ۼ�
// 			set_uart_recv_data_len();
// 		}
// 		else
// 		{
// 			set_uart_recv_sta();
// 		}
// 	}
// }

/**
 * @description:  ��ʼ��rb
 * @param 			rb  		ringbuffer
 *					bf_pool 	�ڴ��
 *					size		���ݴ�С
 * @return ִ�н����
 */
int ringbuffer_init(struct ringbuffer* rb, uint8_t* bf_pool, uint16_t size)
{
    if (rb&&bf_pool)
    {
        rb->buffer = (uint8_t*)bf_pool;
        rb->buffer_size   = ALIGN_DOWN(size,4);
        rb->head   = 0;
        rb->tail   = 0;
        return 0;
    }
    else
        return -1;
}

/**
 * @description:  ʹ�õ�rb��С��ֵ
 * @param 			rb  ringbuffer
 * @return ִ�н����
 */
uint16_t ringbuffer_data_len(struct ringbuffer* rb)
{
    return ((rb->buffer_size - rb->head + rb->tail) % rb->buffer_size);
}

/**
 * @description: �������ݵ�rb��
 * @param 			rb  			ringbuffer
 * 					data			����������buffer
 * 					data_length		���͵����ݳ���
 * @return ִ�н����
 */
uint16_t ringbuffer_putstr(struct ringbuffer* rb, const uint8_t* data, uint16_t data_length )
{
    uint16_t space_len = rb->buffer_size - 1 - rb->tail;

    uint16_t put_data_len = MIN(data_length, (space_len + rb->head) % rb->buffer_size);

    space_len++;

    memcpy(&rb->buffer[rb->tail], data, MIN(put_data_len, space_len));

    if ( space_len < put_data_len )
    {
        memcpy( &rb->buffer[ 0 ], data + space_len, put_data_len - space_len );
    }

    rb->tail = (rb->tail + put_data_len) % rb->buffer_size;

    return put_data_len;
}

/**
 * @description: ��rb�ж�ȡ����
 * @param 			rb  			ringbuffer
 * 					data			����ȡbuffer
 * 					data_length		��ȡ�����ݳ���
 * @return ִ�н����
 */
int ringbuffer_getstr(struct ringbuffer* rb, uint8_t* data, uint16_t data_length)
{
    uint16_t i, used_space, max_read_len, head;

    head = rb->head;

    used_space = ringbuffer_data_len(rb);

    max_read_len = MIN(data_length, used_space);

    if ( max_read_len != 0)
    {
        for ( i = 0; i != max_read_len; i++, ( head = ( head + 1 ) % rb->buffer_size ) )
        {
            data[ i ] = rb->buffer[ head ];
        }
		rb->head = (rb->head + max_read_len) % rb->buffer_size;
    }
    return 0;
}
