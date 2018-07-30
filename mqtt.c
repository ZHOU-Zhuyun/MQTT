/**
 ******************************************************************************
 * @file    mqtt.c
 * @author  Zhuyun Zhou
 * @brief   C library for MQTT communications
 ******************************************************************************
 */

#include <string.h>
#include "mqtt.h"

/** @brief  MQTT connection
 * @param  serverAddress               tambori.disc.upv.es
 * @param  serverPort                  1883
 * @param  clientId                    any text
 * @param  cleanSession                0 or 1
 * @param  username                    possibly null (not used)
 * @param  password                    possibly null (not used)
 * @param  keepAlive                   Seconds (positive number, 0: default)
 * @param  lastWillTopic               any text (topic)
 * @param  lastWillValue               any text (topic value)
 * @param  lastWillValueLength         strlen(lastWillValue)
 * @retval return value of CONNACK (= Connection Acknowledge) packet
 * Return Code	Return Code Response
 * 0	Connection accepted
 * 1	Connection refused, unacceptable protocol version
 * 2	Connection refused, identifier rejected
 * 3	Connection refused, server unavailable
 * 4	Connection refused, bad username or password
 * 5	Connection refused, not authorized
 */
int mqtt_connect(char* serverAddress, int serverPort, char* clientId,
		int cleanSession, char* username, char* password, int keepAlive,
		char* lastWillTopic, char* lastWillValue, int lastWillValueLength) {
	unsigned int total_packet_length;
	uint8_t connect_flags;

	/* Verify input parameters for possible invalid (illegal) values */
	// If ClientId is a zero-byte, CleanSession MUST be 1
	// Otherwise, Server MUST return CONNACK code 0x02 (Identifier rejected)
	// and then close the Network Connection
	if ((clientID == NULL) && (cleanSession != 1)) {
		return MQTT_ERROR_INVALID_PARAM;
	}
	// cleanSession can be only 0 or 1
	if ((cleanSession < 0) || (cleanSession > 1)) {
		return MQTT_ERROR_INVALID_PARAM;
	}
	// If password is NOT NULL then username must be NOT NULL also
	// this is mandated by the mqtt protocol
	if ((password != NULL) && (username == NULL)) {
		return MQTT_ERROR_INVALID_PARAM;
	}

	// Format a buffer following the mqtt CONNECT format
	total_packet_length = 0;
	total_packet_length += _mqttEncodePacketType(
			&mqttBufferTx[total_packet_length], MQTT_CONTROL_PACKET_CONNECT,
			MQTT_PACKET_MAXLEN - total_packet_length);
	total_packet_length += _mqttEncodeString(
			&mqttBufferTx[total_packet_length], "MQTT",
			MQTT_PACKET_MAXLEN - total_packet_length);
	mqttBufferTx[total_packet_length++] = 4;        //Protocol level
	connect_flags = 0;
	if (username != NULL) {
		connect_flags |= CONNECT_FLAGS_USERNAME;
	}
	if (password != NULL) {
		connect_flags |= CONNECT_FLAGS_PASSWORD;
	}
	if (lastWillTopic != NULL) {
		connect_flags |= CONNECT_FLAGS_WILL;
		// Encode other bit fields related to the lastWill message:
		// WillRetain, WillQoS => IMPORTANT: These value
		// should be additional input parameters
		if (lastWillValue != NULL) {
			connect_flags |= CONNECT_FLAGS_WILL_RETAIN;
		}
		if (lastWillValueLength != NULL) {
			// Encode the QoS value into the byte
			connect_flags &= ~CONNECT_FLAGS_WILL_QOS_MASK;
			connect_flags |= (qos_value << CONNECT_FLAGS_WILL_QOS_SHIFT)
					& CONNECT_FLAGS_WILL_QOS_MASK;
		}
	}
	if (cleanSession) {
		// True, clean former sessions
		conncet_flags |= CONNECT_FLAGS_CLEANSESSION;
	}
	mqttBufferTx[total_packet_length++] = connect_flags;
	total_packet_length += _mqttEncodeUnit16(
			&mqttBufferTx[total_packet_length], (uint16_t) keepAlive,
			MQTT_PACKET_MAXLEN - total_packet_length);
	// If clientId is NULL this function must encode a zero-length string
	total_packet_length += _mqttEncodeString(
			&mqttBufferTx[total_packet_length], clientId,
			MQTT_PACKET_MAXLEN - total_packet_length);
	if (lastWillTopic != NULL) {
		total_packet_length += _mqttEncodeString(
				&mqttBufferTx[total_packet_length], lastWillTopic,
				MQTT_PACKET_LENGTH - total_packet_length);
		total_packet_length += _mqttCopyBuffer(
				&mqttBufferTx[total_packet_length], lastWillValue,
				lastWillValueLength, MQTT_PACKET_MAXLEX - total_packet_length);
	}
	if (username != NULL) {
		total_packet_length += _mqttEncodeString(
				&mqttBufferTx[total_packet_length], username,
				MQTT_PACKET_MAXLEN - total_packet_length);
	}
	if (password != NULL) {
		total_packet_length += _mqttEncodeString(
				&mqttBufferTx[total_packet_length], password,
				MQTT_PACKET_MAXLEN - total_packet_length);
	}

	_mqttEncodeRemainingLength(mqttBufferTx[1], total_packet_length);
}

/**
 * @brief  Helper function - _mqttEncodeString
 * @param  str (char*)
 * @param  buffer (char*)
 * @param  lengthAvailable (uint16_t) avoid passing MQTT_PACKET_MAXLEN
 * @retval length (uint16_t) or 0 if length passes MQTT_PACKET_MAXLEN
 */
uint16_t _mqttEncodeString(char* str, char* buffer, uint16_t lengthAvailable) {
	uint16_t len;

	if (lengthAvailable < 2) {
		return 0;
	}
	// length of string is 0
	else if ((str == NULL) || (*str == '\0')) {
		buffer[0] = 0;
		buffer[1] = 0;
		return 2;
	}

	len = strlen(str);

	if (lengthAvailable < len + 2) {
		return 0;
	} else {
		// MSB
		buffer[0] = len >> 8;
		// LSB
		buffer[1] = len & 0x00FF;

		for (i = 0; i < len; i++) {
			buffer[2 + i] = str[i];
		}
		/* Alternative code:
		 * copyString (str, &buffer[2], len);
		 */

		return len + 2;
	}
}

/**
 * @brief  Helper function - _mqttEncodeRemainingLength, encodes info backwards
 * Delay the encoding of RemainingLength to the end, when we know exactly RL
 * @param  buffer (char*)
 * @param  remainingLength (uint32_t) - 0 .. 4,294,967,295
 * Digits	From	To
 * 1	0(0x00)	127(0x7F)
 * 2	128(0x80, 0x01)	16 383(0xFF, 0x7F)
 * 3	16 384(0x80, 0x80, 0x01)	2 097 151(0xFF, 0xFF, 0x7F)
 * 4	2 097 152(0x80, 0x80, 0x80, 0x01)	268 435 455(0xFF, 0xFF, 0xFF, 0x7F)
 * @retval length (uint16_t)
 */
uint16_t _mqttEncodeRemainingLength(char* buffer, uint32_t remainingLength) {
	uint8_t i = 4;
	do {
		buffer[i] = remainingLength % 128;
		remainingLength /= 128;
		// if there are more data to encode, set the top bit of this byte
		if (remainingLength > 0) {
			buffer[i] |= 128;
		}
		i--;
	} while (remainingLength > 0);
	return 4-i;
}

/**
 * @brief  Helper function - _mqttDecodeString
 * @param  str (char*)
 * @param  buffer (char*)
 * @retval length (uint16_t)
 */
uint16_t _mqttDecodeString(char* str, char* buffer) {

}
