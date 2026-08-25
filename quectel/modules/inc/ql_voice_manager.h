/**
 * @file ql_voice_manager.h 
 * @brief quectel voice manager interface definitions
 */

#ifndef __QL_VOICE_MANAGER_H__
#define __QL_VOICE_MANAGER_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "at.h"
#include "ql_common_def.h"

typedef enum
{
    QL_VOICE_STATE_IDLE = 0,      // No active call, system idle
    QL_VOICE_STATE_HOLD = 1,      // Call local hold
    QL_VOICE_STATE_ORIGINAL = 2,  // Call originated
    QL_VOICE_STATE_CONNECT = 3,   // Call connected
    QL_VOICE_STATE_INCOMING = 4,  // Call incoming
    QL_VOICE_STATE_WAITING = 5,   // Call waiting
    QL_VOICE_STATE_END = 6,       // Call ended
    QL_VOICE_STATE_ALERTING = 7,  // Call alerting
} QL_VOICE_STATE_e;

typedef struct ql_voice_info
{
    int id;              /* Call ID, uniquely identifies a call */
    int dir;             /* Call direction: 0=Mobile Originated (MO), 1=Mobile Terminated (MT) */
    int state;           /* Call state: 1=Local Hold, 2=Original, 3=Connect, 4=Incoming, 5=Waiting, 6=End, 7=Alerting */
    char number[32];     /* Phone number of the remote party */
} ql_voice_info_s;
typedef void (*voice_notice_callback)(ql_voice_info_s, void *);

typedef struct ql_voice_manager
{
    at_client_t client;
    QL_VOICE_STATE_e state;
    /*Do not call encapsulated functions such as voice_answer or voice_hangup 
    within the callback function. Please use a message queue for processing*/
    voice_notice_callback voice_notice;
    void *user_data;
} ql_voice_manager_s;

typedef ql_voice_manager_s* ql_voice_manager_t;

/**
 * @brief Create a voice manager instance
 * @param client AT client handle used for underlying communication
 * @param cb Callback function for incoming call notification
 * @param user_data User private data pointer, will be passed to the callback function
 * @return ql_voice_manager_t Voice manager instance handle, NULL if failed
 */
ql_voice_manager_t ql_voice_manager_create(at_client_t client, voice_notice_callback cb, void *user_data);

/**
 * @brief Initiate a voice call to the specified phone number
 * @param handle Voice manager instance handle
 * @param phone_number Target phone number string
 * @return QL_VOICE_MANAGER_ERR_CODE_E Error code
 */
QL_VOICE_MANAGER_ERR_CODE_E ql_voice_dial(ql_voice_manager_t handle, const char* phone_number);

/**
 * @brief Answer an incoming voice call
 * @param handle Voice manager instance handle
 * @return QL_VOICE_MANAGER_ERR_CODE_E Error code
 */
QL_VOICE_MANAGER_ERR_CODE_E ql_voice_answer(ql_voice_manager_t handle);

/**
 * @brief Place current call on hold and answer waiting call
 * @param handle Voice manager instance handle
 * @return QL_VOICE_MANAGER_ERR_CODE_E Error code
 */
QL_VOICE_MANAGER_ERR_CODE_E ql_voice_hold_and_answer(ql_voice_manager_t handle);

/**
 * @brief Release current call and answer waiting call
 * @param handle Voice manager instance handle
 * @return QL_VOICE_MANAGER_ERR_CODE_E Error code
 */
QL_VOICE_MANAGER_ERR_CODE_E ql_voice_release_and_answer(ql_voice_manager_t handle);

/**
 * @brief Hang up the current voice call
 * @param handle Voice manager instance handle
 * @return void
 */
void ql_voice_hangup(ql_voice_manager_t handle);

/**
 * @brief Destroy the voice manager instance and release resources
 * @param handle Voice manager instance handle returned by ql_voice_manager_create()
 * @return void
 */
void ql_voice_manager_destroy(ql_voice_manager_t handle);

#ifdef __cplusplus
}
#endif

#endif // __QL_VOICE_MANAGER_H__