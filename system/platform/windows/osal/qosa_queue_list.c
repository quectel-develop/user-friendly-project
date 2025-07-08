/*****************************************************************/ /**
* @file qosa_queue_list.c
* @brief 
* @author larson.li@quectel.com
* @date 2024-12-23
* 
* @copyright Copyright (c) 2023 Quectel Wireless Solution, Co., Ltd. 
* All Rights Reserved. Quectel Wireless Solution Proprietary and Confidential.
* 
* @par EDIT HISTORY FOR MODULE
* <table>
* <tr><th>Date <th>Version <th>Author <th>Description
* <tr><td>2024-12-23 <td>1.0 <td>Larson.Li <td> Init
* </table>
**********************************************************************/
#include "qosa_queue_list.h"
#include "qosa_def.h"

//#define list_q_printf(msg, ...)  printf("%s,%d," msg "\r\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define list_q_printf(...)

/* NOTUSED */
#define NOTUSED(i)                                                                                                                                             \
    if (i)                                                                                                                                                     \
    {                                                                                                                                                          \
    }


/* ==========================================================================
FUNCTION Q_LOCK_INIT
DESCRIPTION
   Initializes the mutex for the given queue. It is expected that this call
   happens only once for every queue. If calling multiple times for a given
   queue, ensure that the mutex is deleted/freed (q_lock_delete), prior to it.
DEPENDENCIES
   None.
RETURN VALUE
   None.
SIDE EFFECTS
   None.
============================================================================= */
void q_lock_init(qosa_type_t* q_ptr)
{
    int ret = 0;

    ret = qosa_mutex_create(&q_ptr->mutex);
    list_q_printf("...");
    if (ret != QOSA_OK)
    {
        list_q_printf("qosa_mutex_create failed");
    }

    return;
} /* END q_lock_init */

/* ==========================================================================
FUNCTION Q_LOCK_DELETE
DESCRIPTION
   Deletes the mutex for the given queue
DEPENDENCIES
   None.
RETURN VALUE
   None.
SIDE EFFECTS
   None.
============================================================================= */
void q_lock_delete(qosa_type_t* q_ptr)
{
    int ret = 0;
    ret = qosa_mutex_delete(q_ptr->mutex);
    if (ret != QOSA_OK)
    {
        list_q_printf("qosa_mutex_delete failed");
    }
    return;
} /* END q_lock_delete */

/* ==========================================================================
FUNCTION Q_LOCK
DESCRIPTION
   Locks the mutex for the given queue
DEPENDENCIES
   None.
RETURN VALUE
   None.
SIDE EFFECTS
   None.
============================================================================= */
static void q_lock(qosa_type_t* q_ptr)
{
    int ret = 0;
    ret = qosa_mutex_lock(q_ptr->mutex, QOSA_WAIT_FOREVER);
    if (ret != QOSA_OK)
    {
        list_q_printf("qosa_mutex_lock failed");
    }
    return;
} /* END q_lock */

/* ==========================================================================
FUNCTION Q_UNLOCK
DESCRIPTION
   Frees the mutex for the given queue
DEPENDENCIES
   None.
RETURN VALUE
   None.
SIDE EFFECTS
   None.
============================================================================= */
static void q_unlock(qosa_type_t* q_ptr)
{
    int ret = 0;
    ret = qosa_mutex_unlock(q_ptr->mutex);
    if (ret != QOSA_OK)
    {
        list_q_printf("qosa_mutex_unlock failed");
    }
    return;
} /* END q_unlock */

/*==========================================================================
FUNCTION Q_INIT

DESCRIPTION
  This function initializes a specified queue. It should be called for each
  queue prior to using the queue with the other Queue Services.

  Note : when qosa_init is called, q_ptr has to either from bss/global or if
         it is from heap, then osa_memset to 0
DEPENDENCIES
  None.

RETURN VALUE
  A pointer to the initialized queue.

SIDE EFFECTS
  The specified queue is initialized for use with Queue Services.
===========================================================================*/
qosa_type_t* qosa_init(qosa_type_t* q_ptr /* Ptr to queue to be initialized. */
)
{
    QOSA_ASSERT(q_ptr != NULL);
    q_ptr->link.next_ptr = (qosa_link_type_t*)(&q_ptr->link); /* Points to q link. */
    q_ptr->link.prev_ptr = (qosa_link_type_t*)(&q_ptr->link); /* Points to q link. */

    q_ptr->cnt = 0;

    q_lock_init(q_ptr);

    return q_ptr;
} /* END qosa_init */

/*===========================================================================
FUNCTION Q_LINK

DESCRIPTION
  This function initializes a specified link. It should be called for each
  link prior to using the link with the other Queue Services.

DEPENDENCIES
  None.

RETURN VALUE
  A pointer to the initialized link.

SIDE EFFECTS
  The specified link is initialized for use with the Queue Services.
===========================================================================*/
qosa_link_type_t* qosa_link(
    void*             item_ptr, /* Ptr to item or variable containing link. */
    qosa_link_type_t* link_ptr  /* Ptr to link field within variable. */
)
{
    QOSA_ASSERT(link_ptr != NULL);
    link_ptr->next_ptr = NULL;
    link_ptr->prev_ptr = NULL;

    NOTUSED(item_ptr);  // below macro is empty... remove this line when it's not

    return link_ptr;
} /* END qosa_link */

/*===========================================================================
FUNCTION Q_PUT

DESCRIPTION
  This function enqueues an item onto a specified queue using a specified
  link.

DEPENDENCIES
  The specified queue should have been previously initialized via a call
  to qosa_init. The specified link field of the item should have been prev-
  iously initialized via a call to q_init_link.

RETURN VALUE
  None.

SIDE EFFECTS
  The specified item is placed at the tail of the specified queue.
===========================================================================*/
void qosa_put(
    qosa_type_t*      q_ptr,   /* Ptr to queue. */
    qosa_link_type_t* link_ptr /* Ptr to item link to use for queueing. */
)
{
    q_lock(q_ptr);

    QOSA_ASSERT(q_ptr != NULL);
    QOSA_ASSERT(link_ptr != NULL);
    link_ptr->next_ptr = (qosa_link_type_t*)&q_ptr->link;
    link_ptr->prev_ptr = q_ptr->link.prev_ptr;

    if (q_ptr->link.prev_ptr != NULL)
    {
        q_ptr->link.prev_ptr->next_ptr = link_ptr;
    }
    q_ptr->link.prev_ptr = link_ptr;
    q_ptr->cnt++;
    list_q_printf("link_ptr=%p,pre=%p,%d", link_ptr, q_ptr->link.prev_ptr, q_ptr->cnt);

    q_unlock(q_ptr);
    return;
} /* END qosa_put */

/*===========================================================================
FUNCTION Q_GET

DESCRIPTION
  This function removes an item from the head of a specified queue.

DEPENDENCIES
  The specified queue should have been initialized previously via a call
  to qosa_init.

RETURN VALUE
  A pointer to the dequeued item. If the specified queue is empty, then
  NULL is returned.

SIDE EFFECTS
  The head item, if any, is removed from the specified queue.
===========================================================================*/
void* qosa_get(qosa_type_t* q_ptr /* Ptr to queue. */
)
{
    qosa_link_type_t* link_ptr;
    qosa_link_type_t* ret_ptr = NULL;
    QOSA_ASSERT(q_ptr != NULL);
    q_lock(q_ptr);

    /* Get ptr to 1st queue item.
     */
    link_ptr = q_ptr->link.next_ptr;

    /* Can only get an item if the queue is non empty
     */
    if (q_ptr->cnt > 0)
    {
        q_ptr->link.next_ptr = link_ptr->next_ptr;

        if (link_ptr->next_ptr != NULL)
        {
            link_ptr->next_ptr->prev_ptr = &q_ptr->link;
        }

        q_ptr->cnt--;

        /* Mark item as no longer in a queue.
         */
        link_ptr->next_ptr = NULL;
        ret_ptr = link_ptr;
    }

    q_unlock(q_ptr);

    return (void*)ret_ptr;
} /* END qosa_get */

/*===========================================================================
FUNCTION Q_LAST_GET

DESCRIPTION
  This function returns the item which was most recently enqueued in a queue.

  Note, this is different from qosa_get() which returns the oldest item in a
  queue.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None
===========================================================================*/
void* qosa_last_get(qosa_type_t* q_ptr)
{
    qosa_link_type_t* link_ptr;
    qosa_link_type_t* ret_ptr = NULL;
    QOSA_ASSERT(q_ptr != NULL);
    q_lock(q_ptr);

    for (link_ptr = (qosa_link_type_t*)q_ptr; link_ptr->next_ptr != q_ptr->link.prev_ptr; link_ptr = link_ptr->next_ptr)
        ;

    if (q_ptr->cnt > 0)
    {
        ret_ptr = link_ptr->next_ptr;
        q_ptr->link.prev_ptr = link_ptr;
        link_ptr->next_ptr = (qosa_link_type_t*)(&q_ptr->link);
        q_ptr->cnt--;
        ret_ptr->next_ptr = NULL;
    }

    q_unlock(q_ptr);

    return (void*)ret_ptr;
} /* qosa_last_get */

/*===========================================================================
FUNCTION Q_CNT

DESCRIPTION
  This function returns the number of items currently queued on a specified
  queue.

DEPENDENCIES
  The specified queue should have been initialized previously via a call
  to qosa_init.

RETURN VALUE
  The number of items currently queued on the specified queue.

SIDE EFFECTS
  None.
===========================================================================*/
int32_t qosa_cnt(qosa_type_t* q_ptr)
{
    QOSA_ASSERT(q_ptr != NULL);
    return q_ptr->cnt;
} /* END qosa_cnt */

/*===========================================================================
FUNCTION Q_CHECK

DESCRIPTION
  This function returns a pointer to the data block at the head of the queue.
  The data block is not removed from the queue.

DEPENDENCIES
  The specified queue should have been initialized previously via a call
  to qosa_init.

RETURN VALUE
  A pointer to the queue item. If the specified queue is empty, then
  NULL is returned.

SIDE EFFECTS
  None
===========================================================================*/
void* qosa_check(qosa_type_t* q_ptr)
{
    qosa_link_type_t* link_ptr;

    qosa_link_type_t* ret_ptr = NULL;

    QOSA_ASSERT(q_ptr != NULL);
    if (q_ptr == NULL)
    {
        return NULL;
    }
    q_lock(q_ptr);

    link_ptr = q_ptr->link.next_ptr;
    list_q_printf("link_ptr=%p,pre=%p,%d", link_ptr, q_ptr->link.prev_ptr, q_ptr->cnt);
    if (q_ptr->cnt > 0)
    {
        ret_ptr = link_ptr;
    }

    q_unlock(q_ptr);

    return (void*)ret_ptr;
} /* END qosa_check */

/*===========================================================================

FUNCTION Q_LAST_CHECK

DESCRIPTION
  This function returns the item which was most recently enqueued in a queue.

  Note, this is different from qosa_check() which returns the oldest item in a
  queue.

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None
===========================================================================*/
void* qosa_last_check(qosa_type_t* q_ptr /* The queue from which the item will be removed */
)
{
    qosa_link_type_t* link_ptr;       /* For returning value. */
    qosa_link_type_t* ret_ptr = NULL; /* For returning value. */

    QOSA_ASSERT(q_ptr != NULL);
    q_lock(q_ptr);

    link_ptr = q_ptr->link.prev_ptr;

    if (q_ptr->cnt > 0)
    {
        ret_ptr = link_ptr;
    }

    q_unlock(q_ptr);

    return (void*)ret_ptr;
} /* qosa_last_check */

/*===========================================================================
FUNCTION Q_NEXT

DESCRIPTION
  This function returns a pointer to the next item on the queue.

DEPENDENCIES
  The specified queue should have been initialized previously via a call
  to qosa_init.

RETURN VALUE
  A pointer to the next item on the queue. If the end of the queue is reached,
  then NULL is returned.

SIDE EFFECTS
  None.
===========================================================================*/
void* qosa_next(qosa_type_t* q_ptr, qosa_link_type_t* q_link_ptr)
{
    void* q_temp_ptr = NULL;
    QOSA_ASSERT(q_link_ptr != NULL);
    q_lock(q_ptr);
    if ((void*)q_link_ptr->next_ptr != (void*)q_ptr)
    {
        q_temp_ptr = q_link_ptr->next_ptr;
    }
    q_unlock(q_ptr);
    return q_temp_ptr;
} /* END qosa_next */

/*===========================================================================
FUNCTION Q_NEXT

DESCRIPTION
  This function returns a pointer to the prev item on the queue.

DEPENDENCIES
  The specified queue should have been initialized previously via a call
  to qosa_init.

RETURN VALUE
  A pointer to the prev item on the queue. If the end of the queue is reached,
  then NULL is returned.

SIDE EFFECTS
  None.
===========================================================================*/
void* qosa_prev(qosa_type_t* q_ptr, qosa_link_type_t* q_link_ptr)
{
    void* q_temp_ptr = NULL;
    QOSA_ASSERT(q_link_ptr != NULL);
    q_lock(q_ptr);
    if ((void*)q_link_ptr->prev_ptr != (void*)q_ptr)
    {
        q_temp_ptr = q_link_ptr->prev_ptr;
    }
    q_unlock(q_ptr);
    return q_temp_ptr;
} /* END qosa_next */

/*===========================================================================
FUNCTION Q_INSERT

DESCRIPTION
  This function inserts an item before a specified item on a queue.

DEPENDENCIES
  The specified queue should have been initialized previously via a call
  to qosa_init.

RETURN VALUE
  None.

SIDE EFFECTS
  Input item is inserted before input item.
===========================================================================*/
void qosa_insert(
    qosa_type_t*      q_ptr,        /* Ptr to the queue */
    qosa_link_type_t* q_insert_ptr, /* Pointer to the item to be inserted */
    qosa_link_type_t* q_item_ptr    /* Ptr to queue item before which q_insert_ptr to be inserted. */
)
{
    qosa_link_type_t* link_ptr;

    QOSA_ASSERT(q_ptr != NULL);
    QOSA_ASSERT(q_insert_ptr != NULL);
    q_lock(q_ptr);

    q_insert_ptr->next_ptr = q_item_ptr;

    /* Start at beginning of queue and find the item that will be before the
    ** new item
    */
    link_ptr = (qosa_link_type_t*)q_ptr;

    QOSA_ASSERT(link_ptr != NULL);
    while (link_ptr->next_ptr != q_item_ptr)
    {
        link_ptr = link_ptr->next_ptr;
        QOSA_ASSERT(link_ptr != NULL);
        if (link_ptr == NULL)
        {
            q_unlock(q_ptr);
            return;
        }
    }
    link_ptr->next_ptr = q_insert_ptr;

    q_ptr->cnt++;

    q_unlock(q_ptr);
    return;
} /* END qosa_insert */

/*===========================================================================
FUNCTION Q_LINEAR_SEARCH

DESCRIPTION
  Given a comparison function, this function traverses the elements in
  a queue, calls the compare function, and returns a pointer to the
  current element being compared if the user passed compare function
  returns non zero.

  The user compare function should return 0 if the current element is
  not the element in which the compare function is interested.

DEPENDENCIES
  The specified queue should have been initialized previously via a call
  to qosa_init.

  The user's queue elements must have qosa_link_type_t as the first element
  of the queued structure.

RETURN VALUE
  A pointer to the found element

SIDE EFFECTS
  None.
===========================================================================*/
void* qosa_linear_search(qosa_type_t* q_ptr, qosa_compare_func compare_func, void* compare_val)
{
    qosa_generic_item_type_t* item_ptr = NULL;

    QOSA_ASSERT(compare_func != NULL);
    item_ptr = (qosa_generic_item_type_t*)qosa_check(q_ptr);
    list_q_printf("%p item_ptr = %p", q_ptr, item_ptr);
    while (item_ptr != NULL)
    {
        list_q_printf("%p item_ptr = %p", q_ptr, item_ptr);
        if (compare_func(item_ptr, compare_val) != 0)
        {
            list_q_printf("return ");
            return item_ptr;
        }
        list_q_printf("next ");
        item_ptr = (qosa_generic_item_type_t*)qosa_next(q_ptr, &item_ptr->link);
    } /* END while traversing the queue */

    return NULL;
} /* END qosa_linear_search */

/*===========================================================================
FUNCTION Q_DELETE

DESCRIPTION
  This function removes an item from a specified queue.

DEPENDENCIES
  The specified queue should have been initialized previously via a call
  to qosa_init.

RETURN VALUE
  None.

SIDE EFFECTS
  Input item is delete from the queue.
===========================================================================*/
void qosa_delete(
    qosa_type_t*      q_ptr,       /* Ptr to the Queue */
    qosa_link_type_t* q_delete_ptr /* Ptr to link of item to delete */
)
{
    qosa_link_type_t* link_ptr;
    qosa_type_t*      real_q_ptr;
    int               qcount;

    QOSA_ASSERT(q_ptr != NULL);
    QOSA_ASSERT(q_delete_ptr != NULL);
    q_lock(q_ptr);

    real_q_ptr = q_ptr;

    QOSA_ASSERT(real_q_ptr != NULL);
    for (qcount = q_ptr->cnt, link_ptr = (qosa_link_type_t*)real_q_ptr; link_ptr->next_ptr != q_delete_ptr && qcount > 0;
         link_ptr = link_ptr->next_ptr, qcount--)
        ;

    QOSA_ASSERT(link_ptr != NULL);
    if (qcount > 0)
    {
        link_ptr->next_ptr = q_delete_ptr->next_ptr;

        if (link_ptr->next_ptr == (qosa_link_type_t*)real_q_ptr)
        {
            real_q_ptr->link.prev_ptr = link_ptr;
        }

        q_ptr->cnt--;
        q_delete_ptr->next_ptr = NULL;
        q_delete_ptr->prev_ptr = NULL;
    }

    q_unlock(q_ptr);
    return;
} /* END qosa_delete */

/*==========================================================================
FUNCTION Q_DESTROY

DESCRIPTION
  This function destroys a specified queue. It should be called if you
  do not require this queue anymore.

DEPENDENCIES
  None.

RETURN VALUE
  None.

SIDE EFFECTS
  Elements in the queue will  not be accessible through this queue
  anymore. It is user's responsibility to deallocate the memory allocated
  for the queue and its elements to avoid leaks
===========================================================================*/
void qosa_destroy(qosa_type_t* q_ptr /* Ptr to queue to be destroyed. */
)
{
    QOSA_ASSERT(q_ptr != NULL);

    q_lock(q_ptr);
    q_ptr->link.next_ptr = NULL;
    q_ptr->link.prev_ptr = NULL;

    q_ptr->cnt = 0;
    q_unlock(q_ptr);

    q_lock_delete(q_ptr);

    return;
} /* END qosa_destroy */

/*===========================================================================
FUNCTION Q_PUT_HEAD

DESCRIPTION
  This function enqueues an item onto a specified queue using a specified
  link.

DEPENDENCIES
  The specified queue should have been previously initialized via a call
  to qosa_init. The specified link field of the item should have been prev-
  iously initialized via a call to q_init_link.

RETURN VALUE
  None.

SIDE EFFECTS
  The specified item is placed at the tail of the specified queue.
===========================================================================*/
void qosa_put_head(qosa_type_t* q_ptr, qosa_link_type_t* link_ptr)
{
    QOSA_ASSERT(q_ptr != NULL);
    QOSA_ASSERT(link_ptr != NULL);
    q_lock(q_ptr);
    if (q_ptr->cnt == 0)
    {
        link_ptr->next_ptr = (qosa_link_type_t*)&q_ptr->link;
        link_ptr->prev_ptr = q_ptr->link.prev_ptr;

        if (q_ptr->link.prev_ptr != NULL)
        {
            q_ptr->link.prev_ptr->next_ptr = link_ptr;
        }
        q_ptr->link.prev_ptr = link_ptr;
        q_ptr->cnt++;
    }
    else
    {
        //拿到头结点
        link_ptr->prev_ptr = (qosa_link_type_t*)&q_ptr->link;
        //头结点的下一个节点
        link_ptr->next_ptr = link_ptr->prev_ptr->next_ptr;
        //头结点的下一个节点的前指针等于当前节点
        link_ptr->prev_ptr->next_ptr = link_ptr;
        //当前节点的下一个节点的前一个节点等于自己
        link_ptr->next_ptr->prev_ptr = link_ptr;
        q_ptr->cnt++;
    }
    q_unlock(q_ptr);
    return;
} /* END qosa_put_head */

