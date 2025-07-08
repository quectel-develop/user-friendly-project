/*****************************************************************/ /**
* @file qos_queue_list.h
* @brief
* @author larson.li@quectel.com
* @date 2024-12-23
*
* @copyright Copyright (c) 2023 Quectel Wireless Solution, Co., Ltd.
* All Rights Reserved. Quectel Wireless Solution Proprietary and Confidential.
*
* @par EDIT HISTORY FOR MODULE
* <table>
* <tr><th>Date <th>Version <th>Author <th>Description"
* <tr><td>2024-12-23 <td>1.0 <td>Larson.Li <td> Init
* </table>
**********************************************************************/
#ifndef __QOSA_QUEUE_LIST_H__
#define __QOSA_QUEUE_LIST_H__

#include "qosa_def.h"
#include "qosa_system.h"

/**
 * @struct qosa_link_type_t
 * @brief Doubly linked list element node
 */
typedef struct qosa_link_struct
{
    struct qosa_link_struct* next_ptr; /*!< Pointer to the next link in the list. If OSA_NULL, there is no next link. */
    struct qosa_link_struct* prev_ptr; /*!< Pointer to the previous link in the list. If OSA_NULL, there is no previous link. */
} qosa_link_type_t;

typedef qosa_link_type_t qosa_head_link_type_t;

/**
 * @struct qosa_type_t
 * @brief Used by Queue Utility Services to represent a queue. Note that
 * users must not directly access the fields of a queue; only Queue Utility
 * Services is to access the fields.
 */
typedef struct qosa_struct
{
    qosa_head_link_type_t link;  /*!< Used for linking the items into a queue. */
    int32_t               cnt;   /*!< Keeps track of the number of items enqueued.This field is not necessary
                                for normal operations and is used for debugging purposes. */
    osa_mutex_t           mutex; /*!< mutex,to provide interface security */
} qosa_type_t;

/**
 * @struct qosa_generic_item_type_t
 * @brief Used for generic items, which must have qosa_link_type_t as the
 * first element.
 *
 * This structure allows the linear search function to traverse the list
 * without having any information about the elements.
 */
typedef struct
{
    qosa_link_type_t link; /*!< Used for linking the items into a queue. */
} qosa_generic_item_type_t;

/** Used by the searching functions to determine if an item is in
  the queue. It returns nonzero if the element is to be operated on;
  otherwise, it returns 0.

  For linear searching, the operation is to return a pointer to the
  item and terminate the search.

  For linear deleting, the operation is to remove the item from the
  queue and continue the traversal.
*/
typedef int8_t (*qosa_compare_func)(void* item_ptr, void* compare_val);

/**
 * @brief Initializes a specified queue. This function must be called for each
 * queue before using the queue with other Queue Utility Services.
 *
 * @param[in] qosa_type_t * q_ptr
 *           - Pointer to the queue to initialize.
 *
 * @return qosa_type_t*
 *         - A pointer to the initialized queue.
 */
qosa_type_t* qosa_init(qosa_type_t* q_ptr);

/**
 * @brief Initializes a specified link. This function must be called for each
 * link before using the link with other Queue Utility Services.
 *
 * @param[in] void * item_ptr
 *           - Item to be initialized.
 *
 * @param[in] qosa_link_type_t * link_ptr
 *           - Link to be initialized.
 *
 * @return qosa_link_type_t*
 *        - A pointer to the initialized link.
 */
qosa_link_type_t* qosa_link(void* item_ptr, qosa_link_type_t* link_ptr);

/**
 * @brief Enqueues an item to a specified queue using a specific link.
 *
 * @param[in] qosa_type_t * q_ptr
 *           - Pointer to the item to be enqueued.
 *
 * @param[in] qosa_link_type_t * link_ptr
 *           - Pointer to the link where the item is to be placed.
 * @dependencies
 *   The specified queue must have been initialized previously via a call
 *   to qosa_init(). \n
 *   The specified link field of the item must have been initialized previously
 *   via a call to qosa_link().
 *
 * @sideeffects
 *   The specified item is placed at the tail of a specific queue.
 */
void qosa_put(qosa_type_t* q_ptr, qosa_link_type_t* link_ptr);

/**
 * @berif Removes an item from the head of a specified queue.
 *
 * @param[in] qosa_type_t* q_ptr
 *           - Pointer to the queue.
 *
 * @return void *
 *        - Returns a pointer to the dequeued item, or OSA_NULL if the specified
 *          queue is empty.
 *
 * @dependencies
 * The specified queue must have been initialized previously via a call
 * to qosa_init().
 */
void* qosa_get(qosa_type_t* q_ptr);

/**
 * @brief Removes an item from the tail of a specified queue
 *
 * @param[in] qosa_type_t * q_ptr
 *           - Pointer to the queue
 *
 * @return void *
 *        - Returns a pointer to the dequeued item, or OSA_NULL if the specified
 *          queue is empty.
 *
 * @dependencies
 * The specified queue must have been initialized previously via a call
 * to qosa_init().
 */
void* qosa_last_get(qosa_type_t* q_ptr);

/**
 * @brief Returns the number of items currently queued on a specified queue.
 *
 * @param[in] qosa_type_t * q_ptr
 *           - Pointer to the queue
 *
 * @return osa_int32_t
 *        - The number of items.
 *
 * @dependencies
 * The specified queue must have been initialized previously via a call
 * to qosa_init().
 */
int32_t qosa_cnt(qosa_type_t* q_ptr);

/**
 * @brief Returns a pointer to the data block at the head of the queue. The data
 * block is not removed from the queue.
 *
 * @param[in] qosa_type_t * q_ptr
 *           - Pointer to the queue
 *
 * @return void*
 *        - Returns a pointer to the queue item, or OSA_NULL if the specified queue
 *          is empty.
 *
 * @dependencies
 * The specified queue must have been initialized previously via a call
 * to qosa_init().
 */
void* qosa_check(qosa_type_t* q_ptr);

/**
 * @brief Returns a pointer to the data block at the head of the queue. The data
 * block is not removed from the queue.
 *
 * @param[in] qosa_type_t * q_ptr
 *           - Pointer to the queue
 *
 * @return void*
 *        - Returns a pointer to the queue item, or OSA_NULL if the specified queue
 *          is empty.
 *
 * @dependencies
 * The specified queue must have been initialized previously via a call
 * to qosa_init().
 */
void* qosa_last_check(qosa_type_t* q_ptr);

/**
 * @brief Returns a pointer to the next item on the queue.
 *
 * @param[in] qosa_type_t * q_ptr
 *           - Pointer to the queue
 *
 * @param[in] qosa_link_type_t * link_ptr
 *           - Pointer to the link on the queue.
 *
 * @return void*
 *        - Returns a pointer to the next item on the queue, or OSA_NULL if the end of
 *          the queue is reached.
 *
 * @dependencies
 * The specified queue must have been initialized previously via a call
 * to qosa_init().
 */
void* qosa_next(qosa_type_t* q_ptr, qosa_link_type_t* link_ptr);

/**
 * @brief Returns a pointer to the prev item on the queue.
 *
 * @param[in] qosa_type_t * q_ptr
 *           - Pointer to the queue
 *
 * @param[in] qosa_link_type_t * link_ptr
 *           - Pointer to the link on the queue.
 *
 * @return void*
 *        - Returns a pointer to the next item on the queue, or OSA_NULL if the end of
 *          the queue is reached.
 *
 * @dependencies
 * The specified queue must have been initialized previously via a call
 * to qosa_init().
 */
void* qosa_prev(qosa_type_t* q_ptr, qosa_link_type_t* link_ptr);

/**
 * @brief Inserts an item before a specified item on the queue.
 *
 * @param[in] qosa_type_t * q_ptr
 *           - Pointer to the queue
 *
 * @param[in] qosa_link_type_t * link_ptr
 *           - Pointer to where the item is to be inserted.
 *
 * @param[in] qosa_link_type_t * q_item_ptr
 *           - Pointer to the item to insert.
 *
 * @dependencies
 * The specified queue must have been initialized previously via a call
 * to qosa_init().
 */
void qosa_insert(qosa_type_t* q_ptr, qosa_link_type_t* q_insert_ptr, qosa_link_type_t* q_item_ptr);

/**
 * @brief Given a comparison function, this function traverses the elements in
 * a queue, calls the compare function, and returns a pointer to the
 * current element being compared if the user-passed compare function
 * returns nonzero.
 *
 * The user compare function is to return 0 if the current element
 * is not pertinent to the compare function.
 *
 * @param[in] qosa_type_t * q_ptr
 *           - Pointer to the queue to be traversed.
 *
 * @param[in] qosa_compare_func compare_func
 *           - Comparison function to be used.
 *
 * @param[in] void * compare_val
 *           - Pointer to the value against which to compare.
 *
 * @return void*
 *        - A pointer to the element.
 *
 * @dependencies
 * The specified queue must have been initialized previously via a call
 * to qosa_init(). \n
 * The user's queue elements must have qosa_link_type_t as the first element
 * of the queued structure.
 */
void* qosa_linear_search(qosa_type_t* q_ptr, qosa_compare_func compare_func, void* compare_val);

/**
 * @brief Deletes an item from a specified queue.
 *
 * @param[in] qosa_type_t * q_ptr
 *           - Pointer to the queue.
 *
 * @param[in] qosa_link_type_t * q_delete_ptr
 *           - Pointer to the item to be deleted.
 *
 * @dependencies
 * The specified queue must have been initialized previously via a call
 * to qosa_init().
 */
void qosa_delete(qosa_type_t* q_ptr, qosa_link_type_t* q_delete_ptr);

/**
 * @brief Destroys a specified queue. This function is to be called if the queue
 * is no longer required.
 *
 * @param[in] qosa_type_t * q_ptr
 *           - Pointer to the queue to be destroyed.
 *
 */
void qosa_destroy(qosa_type_t* q_ptr);

/**
 * @brief Enqueues an item to a specified queue using a specific link.
 *
 * @param[in] qosa_type_t * q_ptr
 *           - Pointer to the item to be enqueued.
 *
 * @param[in] qosa_link_type_t * link_ptr
 *           - Pointer to the link where the item is to be placed.
 *
 */
void qosa_put_head(qosa_type_t* q_ptr, qosa_link_type_t* link_ptr);

#endif /* __QOSA_QUEUE_LIST_H__ */
