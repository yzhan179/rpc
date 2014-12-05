/*
 * RPCClient.cpp
 *
 *  Created on: Dec 1, 2014
 *      Author: yzhang
 */

#include "RPCClient.h"


// initiate async session event callback function
struct xio_session_ops RPCClient::session_ops {
	&RPCClient::on_session_event, nullptr, &RPCClient::on_session_established, nullptr,
	&RPCClient::on_msg, &RPCClient::on_msg_delivered, nullptr, nullptr, nullptr,nullptr,nullptr
};

int RPCClient::on_session_event(struct xio_session* session, struct xio_session_event_data* event_data, void* cb_user_context)
{

	RPCClient* rpcc = static_cast<RPCClient*>(cb_user_context);

	printf("session event: %s; reason: %s\n",
			xio_session_event_str(event_data->event),
			xio_strerror(event_data->reason));

	switch (event_data->event) {
	case XIO_SESSION_CONNECTION_ESTABLISHED_EVENT:
		//rpcc->m_session_data->on_new_connection(event_data->conn, cb_user_context);
		break;
	case XIO_SESSION_CONNECTION_TEARDOWN_EVENT:
		rpcc->m_session_data->on_connection_teardown(event_data->conn, cb_user_context);
		break;
	case XIO_SESSION_CONNECTION_CLOSED_EVENT:
		// to see if there should be a handler
		break;
	case XIO_SESSION_CONNECTION_DISCONNECTED_EVENT:
		// to see if there should be a handler
		break;
	case XIO_SESSION_REJECT_EVENT:
		// to see if there should be a handler
		break;
	case XIO_SESSION_TEARDOWN_EVENT:
		RPCClient::on_session_teardown(session, cb_user_context);
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

int RPCClient::on_session_established(struct xio_session *session, struct xio_new_session_rsp *rsp, void *cb_user_context)
{

	RPCClient* rpcc = static_cast<RPCClient*>(cb_user_context);

	// fire the first communication
	rpcc->m_session_data->get_connection()->fire_communication();
	++rpcc->nsent;

	return 0;

}

void RPCClient::on_session_teardown(struct xio_session* session, void* cb_user_context)
{

	RPCClient* rpcc = static_cast<RPCClient*>(cb_user_context);

	xio_session_destroy(session);
	delete rpcc->m_session_data;
	xio_context_stop_loop(rpcc->get_ctx(), 0);
	//xio_context_destroy(rpcc->m_ctx);

}

int RPCClient::on_msg(struct xio_session* session, struct xio_msg* msg, int more_in_batch, void* cb_user_context)
{

	RPCClient* rpcc = static_cast<RPCClient*>(cb_user_context);
	struct message msg_buf;

	int i = msg->request->sn;

	memcpy(&msg_buf, msg->in.header.iov_base, msg->in.header.iov_len);

	switch (msg_buf.type) {
	case MSG_REQ:
		printf("message header type: [%lu] - MSG_REQ\n", (msg->sn + 1));
		rpcc->m_session_data->get_connection()->process_request(msg, i);
		break;
	case MSG_RSP:
		printf("message header type: [%lu] - MSG_RSP\n", (msg->request->sn + 1));
		rpcc->m_session_data->get_connection()->process_response(msg, i);
#ifdef DEBUG
		if (rpcc->nsent < 100000) {
			rpcc->m_session_data->get_connection()->fire_communication();
			++rpcc->nsent;
		}
#endif
		break;
	default:
		printf("Unexpected message header type! Ignored!\n");
		break;
	}

	return 0;

}

int RPCClient::on_msg_delivered(struct xio_session *session, struct xio_msg *msg, int more_in_batch, void *conn_user_context)
{

	printf("session OPS: %p -- one way message delivery receipt notification\n", session);
	return 0;

}

void RPCClient::connect()
{

	xio_init();

	m_ctx = xio_context_create(NULL, 0, -1);

	m_params.type = XIO_SESSION_CLIENT;
	m_params.ses_ops = &RPCClient::session_ops;
	m_params.user_context = this;
	m_params.uri = const_cast<char*>(m_uri.c_str());

	m_session = xio_session_create(&m_params);

	// create the session object
	m_session_data = new Session{ m_session };

	m_connection = xio_connect(m_session, m_ctx, 0, NULL, this);

	// create the connection object
	m_session_data->on_new_connection(m_connection, this);

	std::cout << "client pid: " << getpid() << std::endl;

	xio_context_run_loop(m_ctx, XIO_INFINITE);

	std::cout << "exit context loop!" << std::endl;

	//xio_context_destroy(m_ctx);

}

RPCClient::~RPCClient()
{

	// TODO Auto-generated destructor stub
	xio_context_destroy(m_ctx);
	xio_shutdown();

}

int main(int argc, const char** argv)
{

	char arg[256];

	if (argc < 3) {
		printf("Usage: %s <host> <port> <transport:optional>\n", argv[0]);
		exit(0);
	}

	if (argc > 3)
		sprintf(arg, "%s://%s:%s", argv[3], argv[1], argv[2]);
	else
		sprintf(arg, "tcp://%s:%s", argv[1], argv[2]);

	std::string uri{ arg };

	RPCClient* rpcc = new RPCClient{ uri };

	rpcc->connect();

	delete rpcc;

	return 0;

}
