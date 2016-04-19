#ifndef VN_FW_INTERFACE_H_
#define VN_FW_INTERFACE_H_

/*-----------------------------------------------------------------------
 * $RCSfile: vn_fw_interface.h,v $
 * interface between framework and modules
 * 
 * Usage
 * Sending  messages: create a connection and a message, call 
 * vn_fw_connection_sendMessage,  the respons will be sent to the 
 * callback function provided when creating the message. The framework
 * will destroy messages when used, it is an error to use a message
 * more then once or to create a message and never send it. A connection
 * can be used to send more than one message to the same node, the
 * connection should be destroyed by the module who created it.
 * 
 * Processing requests: To process requests sent by other nodes, provide
 * the framework with a vn_fw_requestHandler callback function by calling
 * vn_fw_setRequestHandler(). The connection should be destroyed by the 
 * framework, the only exception is if a module doesn't want to process
 * a request.
 
 Using timers: Create a timer providing a timeout in ms, an callback to
 be called when timing out and wheter the timer should be periodic or not. 
 Call the timer start function to start the timer. Call the timer stop 
 function to halt the timer and the destroy function when the timer is not
 needed anymore.
 * -----------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <netinet/in.h>
#include <errno.h>

#define TRUE 1
#define FALSE 0

#include "u_list.h"
#include "u_transport.h"

/* Defined at the top since it is used by message. */
typedef int64_t connection_h_t;

#define MAX_PAYLOAD_SIZE    65535

#define CONN_ERROR_CLEAN        0
#define CONN_ERROR_TIMEOUT      1
#define CONN_ERROR_DESTROY      2
#define CONN_ERROR_CONNECT      3
#define CONN_ERROR_RESET        4
#define CONN_ERROR_LOGIN        5
#define CONN_ERROR_BADF         6
#define CONN_ERROR_PROTOCOL     7
#define CONN_ERROR_CANTCONNECT	8
#define CONN_ERROR_NUMOFERRORS	9 // should have been an enum, but shit happens



/****************************************************************************
 * Message
 ****************************************************************************/

/**
 * Type representing a message between two nodes in voddlernet, at module
 * level.
 */
typedef int64_t message_h_t;
#define ILLEGAL_MESSAGE_HANDLE -1

typedef enum {
    MESSAGE_SUCCESS,
    MESSAGE_FAILURE,
    MESSAGE_INVALID_HANDLE,
    MESSAGE_OUT_OF_RESOURCE
    /* More to come as needed. */
} eMessageStatus;

/**
 * Callback function that is called when a response to the message has been
 * received.
 * @param msg Handle to the response that was received.
 * @param conn Handle to connection over which the message was sent/received.
 * @return 0 on success, -1 on failure.
 */
typedef int32_t (*message_responseHandler)(message_h_t msg, connection_h_t conn);

/**
 * Callback function that is called if an error occured when sending a
 * message.
 * @param msg Message used as parameter to vn_fw_sendMessage().
 * @param conn Connection used as parameter to vn_fw_sendMessage().
 * @param errType Type of the error.
 * @return 0 on success, -1 on failure.
 */
typedef int32_t (*message_errorHandler)(message_h_t msg, connection_h_t conn,
        int32_t errType);


/**
 * Creates a new message.
 * @param type Message type. See u_protocol.h for a list of available message types.
 * @param payloadSize Number of elements in 'payload'.
 * @param payload Actual data of the message.
 * @param responseHandler Funcion called when response is received.
 * @param errorHandler Funcion called if an error occurs.
 * @return A valid message handle on success, ILLEGAL_MESSAGE_HANDLE on
 * failure.
 */
message_h_t vn_fw_message_create(uint16_t type, int32_t payloadSize, uint8_t *payload,
        message_responseHandler responseHandler,
        message_errorHandler errorHandler);

/**
 * @param msg Handle to message
 * @return the type of the message
 */
uint16_t vn_fw_message_getType(message_h_t msg);

/**
 * @param msg Handle to message
 * @return the payload size of the message
 */
int32_t vn_fw_message_getPayloadSize(message_h_t msg);

/**
 * Retrieves the payload of a message. The returned value should NOT be free()d
 * after use.
 * @param msg Handle to message
 * @return the payload of the message
 */
uint8_t *vn_fw_message_getPayload(message_h_t msg);

/**
 * Checks if a given message handle is valid
 * @param msg Handle to message
 * @return TRUE(1) if it exists, FALSE(0) if it does not
 */
int32_t vn_fw_message_isValid(message_h_t msg);

/**
 * Adds a local parameter to a message
 * @param msg Handle to message
 * @param param Parameter to attach to message
 * @return MESSAGE_SUCCESS on success, MESSAGE_INVALID_HANDLE on failure
 */
int32_t vn_fw_message_setParam(message_h_t msg, void * param);

/**
 * Fetches the local parameter from a message
 * @param msg Handle to message
 * @return Pointer to parameter on success, NULL on failure or parameter not set
 */
void * vn_fw_message_getParam(message_h_t msg);

/**
 * Returns the request message that matches a response message in a message exchange.
 * @param newmsg Handle to response message
 * @return A handle to the request message, or ILLEGAL_MESSAGE_HANDLE
 */
message_h_t vn_fw_message_getRequest(message_h_t newmsg);


/****************************************************************************
 * Connection
 ****************************************************************************/

/*
 * Type representing a connection to a remote node. In order to send messages
 * to another node, a connection first needs to be established using
 * vn_fw_createConnection(). With the handle that is retrieved from that
 * function, messages can then be sent to the remote node using
 * vn_fw_connection_sendMessage().
 */

#define ILLEGAL_CONNECTION_HANDLE -1

typedef enum {
    CONNECTION_SUCCESS,
    CONNECTION_FAILURE,
    CONNECTION_INVALID_HANDLE,
    CONNECTION_OUT_OF_RESOURCE
    /* More to come as needed. */
} eConnectionStatus;

typedef void (*conn_closedListener)(connection_h_t conn, int32_t errType);

/**
 * Creates a new connection and establishes it with the remote node
 * corresponding to the given node ID.
 * @param nodeId Node ID of the node to connect to.
 * @param param Any state the connection needs to retain between request and response.
 * @return A valid connection handle on success, ILLEGAL_CONNECTION_HANDLE on
 * failure.
 */
connection_h_t vn_fw_connection_create(const sNodeId *nodeId, void* param);

/**
 * Closes the given connection and frees the memory it uses.
 * @param conn The connection to terminate.
 * @return CONNECTION_SUCCESS on success, appropriate error status on failure.
 */
eConnectionStatus vn_fw_connection_destroy(connection_h_t conn);

/**
 * Sends a message on a connection.
 * @param conn Handle to connection to send the message on.
 * @param msg Handle to message to send.
 * @return CONNECTION_SUCCESS on success, appropriate error status on failure.
 */
eConnectionStatus vn_fw_connection_sendMessage(connection_h_t conn, message_h_t msg);

/**
 * @param conn Handle to connection
 * @return peer nodeId associated with this connection. This is a pointer to
 * the connection's own data and not a copy. Do not free().
 */
const sNodeId *vn_fw_connection_getPeerNodeId(connection_h_t conn);

/**
 * @param conn Handle to connection
 * @return parameter associated with this connection
 */
void *vn_fw_connection_getParam(connection_h_t conn);

/**
 * Sets the timeout for a given connection, so it isn't automatically destroyed
 * @param conn Handle to connection
 * @param timeout New timeout value for the connection, in milliseconds
 * @return CONNECTION_SUCCESS on success, appropriate error status on failure
 */
eConnectionStatus vn_fw_connection_setTimeout(connection_h_t conn, int32_t timeout);

/**
 * return the last time this connections timer was restarted, in us
 */
int64_t vn_fw_connection_lastTimerRestart(connection_h_t conn);

/**
 * get the timeout time for this connection, not the time left
 * @param conn Handle to connection
 * @return conns timeout
 */
int32_t vn_fw_connection_getTimeout(connection_h_t conn);
 
/**
 * Checks if a given connection handle is valid
 * @param conn Handle to connection
 * @return TRUE(1) if it exists, FALSE(0) if it does not
 */
int32_t vn_fw_connection_isValid(connection_h_t conn);

/****************************************************************************
 * Request handler
 ****************************************************************************/

/**
 * Request handler callback function.
 * @param msg Handle to received message.
 * @param conn Handle to connection over which the request was received.
 */
typedef void (*vn_fw_requestHandler)(message_h_t msg, connection_h_t conn);

/**
 * Assigns a request handler function to be called when a message of the given
 * type is received.
 * @param type Message type of messages which should trigger the calling of the
 * request handler.
 * @param requestHandler Request handler function to use.
 */
void vn_fw_setRequestHandler(uint16_t type, vn_fw_requestHandler requestHandler);

/****************************************************************************
 * Timer
 ****************************************************************************/

typedef int64_t timer_h_t;
#define ILLEGAL_TIMER_HANDLE -1

typedef enum {
    TIMER_NOT_PERIODIC  = 0,
    TIMER_PERIODIC      = 1
} eTimerPeriodic;

typedef enum {
    TIMER_SUCCESS,
    TIMER_FAILURE,
    TIMER_INVALID_HANDLE,
    TIMER_OUT_OF_RESOURCE
    /* More to come as needed. */
} eTimerStatus;

/**
 * Type for callback functions that are called when a timer triggers.
 * @param timer Handle to the timer that trigged.
 * @param param Application-specific parameter.
 * @return 0 on success, -1 on failure.
 */
typedef int32_t (*vn_fw_timer_expiredHandler)(timer_h_t timer, void* param);

/**
 * Creates a timer. This function does not start the timer after creation. To
 * start it, use vn_fw_timer_start().
 * @param timerExpiredHandler Callback function to call when the timer
 * triggers.
 * @param param Parameter which is sent to 'timerExpiredHandler'.
 * @param time Timer timeout in milliseconds. 
 * @param periodic If 0, timer is once-only. If non-zero, timer restarts after
 * it has triggered.
 * @return A valid timer_h_t on success, ILLEGAL_TIMER_HANDLE on failure.
 */

timer_h_t vn_fw_timer_create(vn_fw_timer_expiredHandler timerExpiredHandler,
        void* param, int32_t delay, eTimerPeriodic periodic);


/**
 * Closes the given timer and frees the memory it uses.
 * @param timer Handle to the timer to terminate.
 * @return TIMER_SUCCESS on success, appropriate error status on failure.
 */
eTimerStatus vn_fw_timer_destroy(timer_h_t timer);

/**
 * Set the timer expired handler for the timer.
 * @param timer Handle to timer.
 * @param timerExpiredHandler Callback function to call when the timer
 * triggers.
 * @return TIMER_SUCCESS on success, appropriate error status on failure.
 */
eTimerStatus vn_fw_timer_setExpiredHandler(timer_h_t timer, vn_fw_timer_expiredHandler timerExpiredHandler);

/**
 * Set how long it will take until the timer expires next time.
 * @param timer Handle to timer.
 * @param time Timer timeout in milliseconds.
 * @return TIMER_SUCCESS on success, appropriate error status on failure.
 */
eTimerStatus vn_fw_timer_setTime(timer_h_t timer, int32_t delay);

/**
 * return the current timeout time or UINT32_MAX on failure
 */
uint32_t vn_fw_timer_getTime(timer_h_t timer);

/**
 * return last time vn_fw_timer_setTime was called, in ms
 */
int64_t vn_fw_timer_getLastTime(timer_h_t timer);

/**
 * Starts the given timer.
 * @param timer Handle to timer to start.
 * @return TIMER_SUCCESS on success, appropriate error status on failure.
 */
eTimerStatus vn_fw_timer_start(timer_h_t timer);

/**
 * Stops the given timer.
 * @param timer Handle to timer to stop.
 * @return TIMER_SUCCESS on success, appropriate error status on failure.
 */
eTimerStatus vn_fw_timer_stop(timer_h_t timer);

/**
 * Checks if a given connection handle is valid
 * @param timer Handle to timer
 * @return TRUE(1) if it exists, FALSE(0) if it does not
 */
int32_t vn_fw_timer_isValid(timer_h_t timer);

#endif /*VN_FW_INTERFACE_H_*/
