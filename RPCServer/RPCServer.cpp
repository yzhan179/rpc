/*
 * RPCServer.cpp
 *
 *  Created on: Nov 21, 2014
 *      Author: yzhang
 */

#include "RPCServer.h"


// initiate async session event callback function
struct xio_session_ops RPCServer::session_ops {
	&RPCServer::on_session_event, &RPCServer::on_new_session, nullptr, nullptr,
	&RPCServer::on_msg, nullptr, nullptr, nullptr, nullptr,nullptr,nullptr
};

int RPCServer::on_session_event(struct xio_session* session, struct xio_session_event_data* event_data, void* cb_user_context)
{

	printf("session event: %s; session: %p, connection: %p, reason: %s\n",
			xio_session_event_str(event_data->event),
			session,
			event_data->conn,
			xio_strerror(event_data->reason));

	RPCServer* rpcs = static_cast<RPCServer*>(cb_user_context);

	std::ostringstream os;
	os << std::hex << static_cast<void*>(session);
	std::string key = os.str();

	switch (event_data->event) {
	case XIO_SESSION_ERROR_EVENT:
		// to see if there should be a handler
		break;
	case XIO_SESSION_NEW_CONNECTION_EVENT:
		if (rpcs->m_ht_session.count(key) != 0) {
			rpcs->m_ht_session.find(key)->second->on_new_connection(event_data->conn, cb_user_context);
		}
		break;
	case XIO_SESSION_CONNECTION_CLOSED_EVENT:
		// to see if there should be a handler
		break;
	case XIO_SESSION_CONNECTION_DISCONNECTED_EVENT:
		// to see if there should be a handler
		break;
	case XIO_SESSION_CONNECTION_TEARDOWN_EVENT:
		if (rpcs->m_ht_session.count(key) != 0) {
			rpcs->m_ht_session.find(key)->second->on_connection_teardown(event_data->conn, cb_user_context);
		}
		else {		// still need to release the connection resource (in the scenario of the session has been rejected)
			xio_connection_destroy(event_data->conn);
		}
		break;
	case XIO_SESSION_TEARDOWN_EVENT:
		RPCServer::on_session_teardown(session, cb_user_context);
		break;
	default:
		printf("unexpected session event: session:%p, %s. reason: %s\n",
				session,
				xio_session_event_str(event_data->event),
				xio_strerror(event_data->reason));
		break;
	}

	return 0;

}

int RPCServer::on_new_session(struct xio_session* session, struct xio_new_session_req* req, void* cb_user_context)
{

	printf("new session event: session %p\n", session);

	RPCServer* rpcs = static_cast<RPCServer*>(cb_user_context);

	if (rpcs->m_ht_session.size() > MAX_SESSION - 1) {
		std::cerr << "=================================================" << std::endl;
		std::cerr << "Reach the upper limitation of number of sessions!" << std::endl << "Reject the new session request..." << std::endl;
		std::cerr << "=================================================" << std::endl;
		xio_reject(session, (enum xio_status)EISCONN, NULL, 0);

		return -1;
	}

	std::unique_ptr<Session> ses{ new Session{ session } };

	std::ostringstream os;
	os << std::hex << static_cast<void*>(session);
	std::string key = os.str();

	rpcs->m_ht_session.insert(std::make_pair(key, std::move(ses)));

	xio_accept(session, NULL, 0, NULL, 0);

	return 0;

}

int RPCServer::on_msg(struct xio_session* session, struct xio_msg* msg, int more_in_batch, void* cb_user_context)
{

	struct message msg_buf;

	memcpy(&msg_buf, msg->in.header.iov_base, msg->in.header.iov_len);

	RPCServer* rpcs = static_cast<RPCServer*>(cb_user_context);

	std::ostringstream os;
	os << std::hex << static_cast<void*>(session);
	std::string key = os.str();

	switch (msg_buf.type) {
	case MSG_REQ:
		printf("message header type: [%lu] - MSG_REQ\n", (msg->sn + 1));
		rpcs->m_ht_session.find(key)->second->get_connection()->process_request(msg);
		break;
	case MSG_RSP:
		printf("message header type: [%lu] - MSG_RSP\n", (msg->sn + 1));
		rpcs->m_ht_session.find(key)->second->get_connection()->process_response(msg);
		break;
	default:
		printf("Unexpected message header type! Ignored!\n");
		break;
	}

	return 0;

}

void RPCServer::on_session_teardown(struct xio_session* session, void* cb_user_context)
{

	RPCServer* rpcs = static_cast<RPCServer*>(cb_user_context);

	std::ostringstream os;
	os << std::hex << static_cast<void*>(session);
	std::string key = os.str();

	if (rpcs->m_ht_session.count(key) != 0) {
		rpcs->m_ht_session.erase(key);
	}

	xio_session_destroy(session);

}

void RPCServer::run_server()
{

	xio_init();

	m_ctx = xio_context_create(NULL, 0, -1);
	m_server = xio_bind(m_ctx, &RPCServer::session_ops, m_uri.c_str(), NULL, 0, this);

	if (m_server) {
		std::cout << "server pid: " << getpid() << "; listening on " << m_uri << std::endl;
		xio_context_run_loop(m_ctx, XIO_INFINITE);
		std::cout << "exit context loop" << std::endl;
		xio_unbind(m_server);
	}

	//xio_context_destroy(m_ctx);

}

RPCServer::~RPCServer()
{

	m_ht_session.clear();

	xio_context_destroy(m_ctx);
	xio_shutdown();

}

int main(int argc, const char** argv)
{

	char arg[256];

	if (argc < 3) {
		printf("Usage: %s <host> <port> <transport:optional>\n", argv[0]);
		exit(1);
	}

	if (argc > 3)
		sprintf(arg, "%s://%s:%s", argv[3], argv[1], argv[2]);
	else
		sprintf(arg, "tcp://%s:%s", argv[1], argv[2]);

	std::string uri{ arg };

	RPCServer* rpcs = new RPCServer{ uri };

	rpcs->run_server();

	delete rpcs;

	return 0;

}
