/*
 * Connection.cpp
 *
 *  Created on: Dec 1, 2014
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

void Connection::process_request(struct xio_msg* req, const int sn)
{

	struct xio_iovec_ex* isglist = vmsg_sglist(&req->in);
	int inents = vmsg_sglist_nents(&req->in);

	printf("request message: [%lu] - %s\n", (req->sn + 1), (char*)(inents > 0 ? isglist[0].iov_base : NULL));

}

void Connection::process_response(struct xio_msg* rsp, const int sn)
{

	struct xio_iovec_ex* isglist = vmsg_sglist(&rsp->in);
	int inents = vmsg_sglist_nents(&rsp->in);

	printf("response message: [%lu] - %s\n", (rsp->request->sn + 1), (char*)(inents > 0 ? isglist[0].iov_base : NULL));

	xio_release_response(rsp);

}

void Connection::fire_communication()
{

	struct message msg_req;
	std::cout << "fire communication here..." << std::endl;

	memset(m_header_buffer, 0, sizeof(BUFFER_SIZE));
	memset(m_data_buffer, 0, sizeof(BUFFER_SIZE));
	memset(&m_channel, 0, sizeof(m_channel));

	msg_req.type = MSG_REQ;
	memcpy(m_header_buffer, &msg_req, sizeof(struct message));

	m_channel.out.header.iov_base = m_header_buffer;
	// Note: iov_len of header cannot beyond 256
	m_channel.out.header.iov_len = sizeof(struct message);

	m_channel.in.sgl_type = XIO_SGL_TYPE_IOV;
	m_channel.in.data_iov.max_nents = XIO_IOVLEN;

	m_channel.out.sgl_type = XIO_SGL_TYPE_IOV;
	m_channel.out.data_iov.max_nents = XIO_IOVLEN;

	sprintf(m_data_buffer, "data block %d from client with pid: %d", ++m_blk_num, getpid());

	m_channel.out.data_iov.sglist[0].iov_base = m_data_buffer;
	m_channel.out.data_iov.sglist[0].iov_len = strlen(static_cast<const char*>(m_channel.out.data_iov.sglist[0].iov_base)) + 1;
	m_channel.out.data_iov.nents = 1;

	xio_send_request(m_conn, &m_channel);

}
