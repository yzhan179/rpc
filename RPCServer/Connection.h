/*
 * Connection.h
 *
 *  Created on: Nov 24, 2014
 *      Author: yzhang
 */

#ifndef CONNECTION_H_
#define CONNECTION_H_

#include <iostream>
#include <memory>

#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <errno.h>
#include <string.h>
#include "libxio.h"

#ifndef			BUFFER_SZIE
#define			BUFFER_SIZE			4096
#endif

#define			MAX_FILE_NAME		128

enum M_TYPE {
	MSG_REQ,
	MSG_RSP,
	MSG_SEND_REQ,
	MSG_SEND_RSP,
	MSG_RECV_REQ,
	MSG_RECV_RSP,
	MSG_READ_REQ,
	MSG_READ_RSP,
	MSG_WRITE_REQ,
	MSG_WRITE_RSP,
	MSG_OPEN_FILE_REQ,
	MSG_OPEN_FILE_RSP,
	MSG_CLOSE_FILE_REQ,
	MSG_CLOSE_FILE_RSP
};

struct message {

	enum M_TYPE type;

	union {
		char file_name[MAX_FILE_NAME];
		int fd;
	} data;

}__attribute((packed));

// class for connection data: each connection object is the resource held by the thread communicating on the connection
class Connection {

public:
	// never use the default constructor
	Connection() = delete;

	// constructor: explicit with argument xio_connection
	explicit Connection(struct xio_connection* conn): m_conn{ conn }, m_conn_flag{ false }
	{
		// register the buffer
		m_header_buffer = new char[BUFFER_SIZE];
		m_data_buffer = new char[BUFFER_SIZE];

		// clean the message channel
		memset(&m_channel, 0, sizeof(struct xio_msg));

	}

	// copy constructor
	Connection(const Connection& conn): m_conn{conn.m_conn}, m_conn_flag{ conn.m_conn_flag }, m_header_buffer{ new char[BUFFER_SIZE] }, m_data_buffer{ new char[BUFFER_SIZE] }
	{

		std::uninitialized_copy(conn.m_header_buffer, conn.m_header_buffer + BUFFER_SIZE, m_header_buffer);
		std::uninitialized_copy(conn.m_data_buffer, conn.m_data_buffer + BUFFER_SIZE, m_data_buffer);
		memcpy(&m_channel, &conn.m_channel, sizeof(struct xio_msg));

	}

	// copy assignment
	Connection& operator=(const Connection& conn)
	{

		Connection tmp{ conn };
		std::swap(tmp, *this);
		return *this;

	}

	// moving constructor
	Connection(Connection&& conn): m_conn{ conn.m_conn }, m_conn_flag{ conn.m_conn_flag }, m_header_buffer{ conn.m_header_buffer }, m_data_buffer{ conn.m_data_buffer }
	{

		std::swap(m_channel, conn.m_channel);
		conn.m_conn = nullptr;
		conn.m_conn_flag = false;
		conn.m_header_buffer = nullptr;
		conn.m_data_buffer = nullptr;
		memset(&conn.m_channel, 0, sizeof(struct xio_msg));

	}

	// moving assignment
	Connection& operator=(Connection&& conn)
	{

		std::swap(m_conn, conn.m_conn);
		std::swap(m_conn_flag, conn.m_conn_flag);
		std::swap(m_header_buffer, conn.m_header_buffer);
		std::swap(m_data_buffer, conn.m_data_buffer);
		std::swap(m_channel, conn.m_channel);

		return *this;

	}

	// set connection flag
	void open_connection();

	// clear connection flag
	void close_connection();

	void process_request(struct xio_msg* req);

	void process_response(struct xio_msg* rsp);

	// destructor
	virtual ~Connection()
	{

		delete[] m_header_buffer;
		delete[] m_data_buffer;

	}

private:
	struct xio_connection* m_conn;					// xio_connection data structure
	bool m_conn_flag;								// flag to indicate the connection status of the communication
	char* m_header_buffer;							// buffer for message header part (using xio_mr when doing RDMA operation)
	char* m_data_buffer;							// buffer for message data part (using xio_mr when doing RDMA operation)
	struct xio_msg m_channel;						// message channel for communication

};

#endif /* CONNECTION_H_ */
