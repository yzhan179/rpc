/*
 * RPCClient.h
 *
 *  Created on: Dec 1, 2014
 *      Author: yzhang
 */

#ifndef RPCCLIENT_H_
#define RPCCLIENT_H_

#include <iostream>

#include <iostream>
#include <sstream>
#include <memory>
#include <string>
#include <unordered_map>
#include <new>

#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <errno.h>
#include "libxio.h"

#include "Connection.h"
#include "Session.h"

#define			DEBUG
#define			MAX_SESSION			2048

class RPCClient {

public:
	// session object (Async event callback function will modify this member, so set it as public)
	Session* m_session_data;

	// number of sent actions
	int nsent;

	// delete the default constructor (the RPCServer must be initialized with URI)
	RPCClient() = delete;

	// constructor with URI (called explicitly)
	explicit RPCClient(const std::string& uri): m_session_data{ nullptr }, nsent{ 0 }, m_uri{ uri }, m_ctx{ nullptr }, m_connection{ nullptr }, m_session{ nullptr }
	{

		memset(&m_params, 0, sizeof(struct xio_session_params));

	}

	// show the uri (for debugging)
#ifdef DEBUG
	void show_uri() const { std::cout << m_uri << std::endl; }
#endif

	// run the RPCServer
	void connect();

	// return the accelio context
	struct xio_context* get_ctx() const { return m_ctx; }

	// return the connection handler
	struct xio_connection* get_connection() const { return m_connection; }

	// destructor
	virtual ~RPCClient();

private:
	std::string m_uri;																		// RPCserver unified resource identifier

	struct xio_context* m_ctx;																// xio concurrency context
	struct xio_connection* m_connection;													// connection handler in RPCClient
	struct xio_session* m_session;															// session resource
	struct xio_session_params m_params;														// parameter for the session
	static struct xio_session_ops session_ops;												// register the session operations

	/**
	 * generic error event notification
	 *
	 *  @param[in] session				the session
	 *  @param[in] data					session event data information
	 *  @param[in] cb_user_context		user private data provided in session
	 *									open
	 *  @returns 0
	 */
	static int on_session_event(struct xio_session* session, struct xio_session_event_data* event_data, void* cb_user_context);

	/**
	* session established notification - client side only
	*
	*  @param[in] session				the session
	*  @param[in] rsp					new session's response information
	*  @param[in] cb_user_context		user private data provided in session
	*			        				open
	*  @returns 0
	*/
	static int on_session_established(struct xio_session *session, struct xio_new_session_rsp *rsp, void *cb_user_context);

	/**
	 * message arrived notification
	 *
	 *  @param[in] session				the session
	 *  @param[in] msg					the incoming message
	 *  @param[in] more_in_batch		hint that more incoming messages
	 *									are expected
	 *  @param[in] conn_user_context	user private data provided in
	 *									connection open on which
	 *									the message send
	 *  @returns 0
	 */
	static int on_msg(struct xio_session* session, struct xio_msg* msg, int more_in_batch, void* cb_user_context);

	/**
	 * one way message delivery receipt notification
	 *
	 *  @param[in] session				the session
	 *  @param[in] msg					the incoming message
	 *  @param[in] more_in_batch		hint that more incoming messages
	 *									are expected
	 *  @param[in] conn_user_context	user private data provided in
	 *									connection open on which
	 *									the message send
	 *  @returns 0
	 */
	static int on_msg_delivered(struct xio_session *session, struct xio_msg *msg, int more_in_batch, void *conn_user_context);

	// callback function for handling the session teardown event
	static void on_session_teardown(struct xio_session* session, void* cb_user_context);

};

#endif /* RPCCLIENT_H_ */
