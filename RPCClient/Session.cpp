/*
 * Session.cpp
 *
 *  Created on: Dec 1, 2014
 *      Author: yzhang
 */

#include "Session.h"


void Session::on_new_connection(struct xio_connection* conn, void* cb_user_context)
{

	try {
		this->m_conn = new Connection{ conn };
	} catch (std::bad_alloc& ba) {
		std::cerr << "create connection object failed! caught bad_alloc: " << ba.what() << std::endl;
	}

	m_conn->open_connection();

}

void Session::on_connection_teardown(struct xio_connection* conn, void* cb_user_context)
{

	if (m_conn != nullptr) {
		m_conn->close_connection();

		delete this->m_conn;
		this->m_conn = nullptr;
	}

	xio_connection_destroy(conn);

}

Connection* Session::get_connection()
{

	return m_conn;

}

Session::~Session()
{

	// TODO Auto-generated destructor stub
	if (m_conn != nullptr)
		delete m_conn;

}
