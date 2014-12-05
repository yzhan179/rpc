/*
 * RPCServer.h
 *
 *  Created on: Nov 21, 2014
 *      Author: yzhang
 */

#ifndef RPCSERVER_H_
#define RPCSERVER_H_

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

#define			MAX_SESSION			3

class RPCServer {

public:
	// hash table for multiple session objects (Async event callback function will modify this member, so set it as public)
	std::unordered_map<std::string, std::unique_ptr<Session>> m_ht_session;

	// delete the default constructor (the RPCServer must be initialized with URI)
	RPCServer() = delete;

	// constructor with URI (called explicitly)
	explicit RPCServer(const std::string& uri): m_ht_session{}, m_uri{ uri }, m_ctx{ nullptr }, m_server{ nullptr } {}

	// show the uri (for debugging)
#ifdef DEBUG
	void show_uri() const { std::cout << m_uri << std::endl; }
#endif

	// run the RPCServer
	void run_server();

	// destructor
	virtual ~RPCServer();

private:
	std::string m_uri;																		// RPCserver unified resource identifier

	struct xio_context* m_ctx;																// xio concurrency context
	struct xio_server* m_server;															// xio server resource
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
	 * new session notification - server side only
	 *
	 *  @param[in] session				the session
	 *  @param[in] req					new session request information
	 *  @param[in] cb_user_context		user private data provided in session
	 *			        				open
	 *  @returns 0
	 */
	static int on_new_session(struct xio_session* session, struct xio_new_session_req* req, void* cb_user_context);

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

	// callback function for handling the session teardown event
	static void on_session_teardown(struct xio_session* session, void* cb_user_context);

};

#endif /* RPCSERVER_H_ */
