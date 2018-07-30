/**
 ******************************************************************************
 * @file    mqtt.h
 * @author  Zhuyun Zhou
 * @brief   C library for MQTT communications
 ******************************************************************************
 */

#include <stdint.h>
#include "helpers.h"

#ifndef __MQTT_H
#define __MQTT_H

// Values for MQTT Control Packet Type field for different supported packets
// first byte of fixed header
#define MQTT_CONTROL_PACKET_CONNECT     1
#define MQTT_CONTROL_PACKET_CONNACK     2

// Define bit fields
#define CONNECT_FLAGS_USERNAME          0x80
#define CONNECT_FLAGS_PASSWORD          0x40
#define CONNECT_FLAGS_WILL_RETAIN       0x20
#define CONNECT_FLAGS_WILL              0x04
#define CONNECT_FLAGS_CLEANSESSION      0x02

// Define QoS value, which includes bit 4 and 3, 4 values 0,1,2,3 are valid
#define CONNECT_FLAGS_WILL_QOS_MASK     0x18
#define CONNECT_FLAGS_WILL_QOS_SHIFT    3

/* Negative return values for internal errors */
// Response message does not confirm to CONNACK format
#define MQTT_ERROR_INVALID_RESPONSE     -3
// Error code to signal it's impossible to connect with the remote destination
#define MQTT_ERROR_NO_CONNECTION        -2
// General error code for invalid parameter values
#define MQTT_ERROR_INVALID_PARAM        -1
/* Zero and positive values for the return codes coming from CONNACK*/
#define MOTT_CONNECT_ACCEPTED           0
#define MQTT_CONNECT_REFUSED_PORT       1       // wrong protocol version
#define MQTT_CONNECT_REFUSED_ID         2       // identifier rejected
#define MQTT_CONNECT_REFUSED_SERVER     3       // server unavailable
#define MQTT_CONNECT_REFUSED_USER       4       // bad username or password
#define MQTT_CONNECT_REFUSED_NOTAUTH    5       // not authorized

// The longest possible mqtt packet accepted
#define MQTT_PACKET_MAXLEN              100
/* Buffer to format mqtt packets */
// Send to remote destination, transmit messages
static mqttBufferTx[MQTT_PACKET_MAXLEN];
// Coming from remote destination, receive messages
static mqttBufferRx[MQTT_PACKET_MAXLEN];

typedef struct {
	uint8_t session_present;
} t_conn_ack_flags;

typedef struct {
	uint8_t conn_return_code;
	t_conn_ack_flags conn_ack_flags;
} t_mqtt_persistent_state;

// Persistent state maintained in mqtt module between calls from application
// to reject sending messages if not connected, for example
//static t_mqtt_persistent_state mqtt_persistent_state = {
//	// Any value indicating the mqtt_connect() function has not been called yet
//	// or it has been called but failed
//	.conn_return_code = 255;
//};

int mqtt_connect(char* serverAddress, int serverPort, char* clientId,
		int cleanSession, char* username, char* password, int keepAlive,
		char* lastWillTopic, char* lastWillValue, int lastWillValueLength);

/** Send the formatted buffer to a remote destination.
 * Currently: Send the formatted buffer to the serial port used for debugging,
 * so we can verify step 2 has been correctly implemented.
 * In the future: Open a TCP connection on serverAddress at port serverPort;
 * if this operation fails, return a negative error code.
 */
// Open the TCP connection
// and send the mqttBufferTx of total_packet_length bytes to the data broke



/** Wait for the CONNACK packet coming from the remote destination.
 * Currently: Wait for a message coming from the serial port used for debugging
 * In the future: Wait for a message coming from the TCP connection
 */
// Wait for the data broker response in mqttBufferRx of rx_packet_length bytes
// from the data broker

// Decode the CONNACK message
//if(rx_packet_length != 4)
//{
//	return MQTT_ERROR_INVALID_RESPONSE;
//}
//if(_mqttDecodePacketType(mqttBufferRx[0]) != MQTT_CONTROL_PACKET_CONNACK)
//{
//	return MQTT_ERROR_INVALID_RESPONSE;
//}
//if(mqttBufferRx[1] != 2)
//{
//	return MQTT_ERROR_INVALID_RESPONSE;
//}
//_mqttDecodeConnAckFlags(mqttBufferRx[2],&mqtt_persistent_state.conn_ack_flags);
//mqtt_persistent_state.conn_return_code = mqttBufferRx[3];
//return mqttBufferRx[3];
// }

// Send the decoded CONNACK message back to the debug channel,
// so we can verify the message is correct

// Send a debug message informing of the information
// you have extracted from the CONNACK message

// Return the Return Code Response received in the CONNACK message

uint16_t _mqttEncodeString(char* str, char* buffer, uint16_t lengthAvailable);
uint16_t _mqttEncodeRemainingLength(char* str, uint16_t remainingLength);
uint16_t _mqttDecodeString(char* str, char* buffer);

#endif // __MQTT_H
