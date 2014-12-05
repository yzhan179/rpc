/*
 * Session.h
 *
 *  Created on: Dec 1, 2014
 *      Author: yzhang
 */

#ifndef SESSION_H_
#define SESSION_H_

#include <iostream>
#include <new>

#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <errno.h>
#include <string.h>

#include "Connection.h"

// class for session data: each session indicate a communication path between a server and a client
class Session {

public:
	// never use the default constructor
	Session() = delete;

	// constructor with specific session ID
	explicit Session(struct xio_session* session): m_session{ session }, m_conn{ nullptr } {}

	// copy constructor
	Session(const Session& ses): m_session{ ses.m_session }, m_conn{ new Connection{ nullptr } }
	{

		std::uninitialized_copy(ses.m_conn, ses.m_conn + sizeof(class Connection), m_conn);

	}

	// copy assignment
	Session& operator=(const Session& ses)
	{

		Session tmp{ ses };
		std::swap(tmp, *this);
		return *this;

	}

	// move constructor
	Session(Session&& ses): m_session{ ses.m_session }, m_conn{ ses.m_conn }
	{

		ses.m_session = nullptr;
		ses.m_conn = nullptr;

	}

	// move assignment
	Session& operator=(Session&& ses)
	{

		std::swap(m_session, ses.m_session);
		std::swap(m_conn, ses.m_conn);
		return *this;

	}

	// initialize the connection object inside session
	void on_new_connection(struct xio_connection* conn, void* cb_user_context);

	// destroy the connection object
	void on_connection_teardown(struct xio_connection* conn, void* cb_user_context);

	// get the connection object for further operations
	Connection* get_connection();

	// destructor
	virtual ~Session();


private:
	struct xio_session* m_session;					// data structure for xio_session (can be used as a session ID)
	Connection* m_conn;								// for now each session will only contain one connection object

};

#endif /* SESSION_H_ */
