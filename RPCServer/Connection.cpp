/*
 * Connection.cpp
 *
 *  Created on: Nov 24, 2014
 *      Author: yzhang
 */

#include "Connection.h"

void Connection::open_connection()
{

	m_conn_flag = true;

}

void Connection::close_connection()
{

	m_conn_flag = false;

}

void Connection::process_request(struct xio_msg* req)
{

	struct xio_iovec_ex* sglist = vmsg_sglist(&req->in);
	char* str = NULL;
	int nents = vmsg_sglist_nents(&req->in);
	int len = 0, i = 0;
	char tmp;
	struct message msg_rsp;

	printf("processing request here...\n");

	for (i = 0; i < nents; ++i) {
		str = (char*)sglist[i].iov_base;
		if (str) {
			len = ((unsigned)sglist[i].iov_len > 64) ? 64 : sglist[i].iov_len;
			tmp = str[len];
			str[len] = '\0';
			printf("message data: [%lu][%d][%d] - %s\n", (req->sn + 1), i, len, str);
			str[len] = tmp;
		}
	}

	// Note: for now each session only contain one connection
	memset(&m_channel, 0, sizeof(m_channel));
	memset(m_header_buffer, 0, sizeof(BUFFER_SIZE));
	memset(m_data_buffer, 0, sizeof(BUFFER_SIZE));

	msg_rsp.type = MSG_RSP;
	memcpy(m_header_buffer, &msg_rsp, sizeof(struct message));

	m_channel.out.header.iov_base = m_header_buffer;
	// Note: iov_len of header cannot beyond 256
	m_channel.out.header.iov_len = sizeof(struct message);

	m_channel.out.sgl_type = XIO_SGL_TYPE_IOV;
	m_channel.out.data_iov.max_nents = XIO_IOVLEN;

	snprintf(m_data_buffer, BUFFER_SIZE, "processed data --%s-- by server with pid: %d", str, getpid());

	m_channel.out.data_iov.sglist[0].iov_base = m_data_buffer;
	m_channel.out.data_iov.sglist[0].iov_len = strlen(static_cast<const char*>(m_channel.out.data_iov.sglist[0].iov_base)) + 1;

	m_channel.out.data_iov.nents = 1;

	m_channel.request = req;

	req->in.header.iov_base	  = NULL;
	req->in.header.iov_len	  = 0;
	vmsg_sglist_set_nents(&req->in, 0);

	xio_send_response(&m_channel);

}

void Connection::process_response(struct xio_msg* rsp)
{



}
