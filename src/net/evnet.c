#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#include "evnet.h"

//设置网络通信读写缓冲区个数 LISTEN_CLIENT CNT +SERVER +UDP CNT

ev_buffer_t ev_bufs[MAX_BUF_CNT];//只用数组作为网络通信（TCP(server/clinet) UDP) 申请队列，后续根据业务情况使用链表分配释放


/*绑定一个可使用得buffer*/
static ev_buffer_t *ev_net_bind_free_buffer(void)  //网络bind 空闲BUFFER
{
	int32_t i;
	for (i = 0; i < 17; ++i) {
		if (0 == ev_bufs[i].using) {
			ev_bufs[i].using = 1;
			return &ev_bufs[i];
		}
	}

	return NULL;
}

/*解绑一个可使用得buffer*/
static void ev_net_unbind_using_buffer(ev_buffer_t *b)
{
	b->using = 0;
	b->rlen = 0;
	b->roff = 0;
	b->woff = 0;
	b->wlen = 0;
	memset(b->r, 0, EV_BUFFER_SIZE);
	memset(b->w, 0, EV_BUFFER_SIZE);
}


#if 0
/*TCP 读取数据到缓冲区*/
static int ev_tcp_read(ev_ctl_t *evctl, ev_fd_t *evfd, int fd, ev_tcp_t *evtcp)
{
	/*读取可容纳缓冲区数量得数据*/
    int32_t bytes = read(fd, evtcp->buffer->r + evtcp->buffer->roff, EV_HALFBUF_SIZE - evtcp->buffer->roff);
	printf("+++++byte.......%d\r\n",bytes);
    if (-1 == bytes) {
        if (EINTR != errno && EAGAIN != errno)
            return 1;
    } else if (0 == bytes) {
        return 2;
    } else {
        evtcp->buffer->rlen = bytes;
        evtcp->buffer->roff += bytes;
		//printf("roff: %d\n",evtcp->buffer->roff);
		evtcp->cb(evctl, evtcp, EV_TCP_DATA, evtcp->arg);
    }
    return 0;
}

/*发送缓冲区数据*/
static int ev_tcp_write(ev_ctl_t *evctl, ev_fd_t *evfd, int fd, ev_tcp_t *evtcp)
{
    int64_t bytes = write(fd, evtcp->buffer->w, evtcp->buffer->woff);
	
    if (-1 == bytes) {
        if (EINTR != errno && EAGAIN != errno) 
            return 1;
    } else {
        int left = evtcp->buffer->woff - bytes;
		if (left > 0)
			memmove(evtcp->buffer->w, evtcp->buffer->w + bytes, left);
		evtcp->buffer->woff = left;
        if (0 == left)
            evctl->unwatch_write(evctl, evfd);
    }
	
    return 0;
}

int ev_tcp_msg(ev_ctl_t *evctl, ev_tcp_t *evtcp, const char *data, int32_t size)
{
	if (NULL == evtcp || evtcp->buffer->woff + size >= EV_BUFFER_SIZE)
		return -1;
	
	memmove(evtcp->buffer->w + evtcp->buffer->woff, data, size);
	evtcp->buffer->woff += size;
	evtcp->buffer->w[evtcp->buffer->woff] = '\r';
	++evtcp->buffer->woff;
	evtcp->buffer->w[evtcp->buffer->woff] = '\n';
	++evtcp->buffer->woff;
	evctl->watch_write(evctl, evtcp->evfd);
	
    return 0;
}
/*设置TCP 事件回调控制*/
void ev_tcp_set(ev_ctl_t *evctl, ev_tcp_t *evtcp, ev_tcp_cb_t cb, void *arg)
{
    evtcp->cb = cb;
    evtcp->arg = arg;
}

/*TCP close 关闭调用*/
void ev_tcp_close(ev_ctl_t *evctl, ev_tcp_t *evtcp)
{
	if (evtcp->evfd)
		evctl->del(evctl, evtcp->evfd);
	if (evtcp->buffer)
		ev_net_unbind_using_buffer(evtcp->buffer);	
	close(evtcp->fd);
	free(evtcp);
    evtcp = NULL;
}


/*根据TCP socket 创建evtcp 控制对象*/
static ev_tcp_t *ev_tcp_create(int sock, ev_tcp_type_t type, ev_tcp_cb_t cb, void *arg)
{
    ev_tcp_t *evtcp = calloc(1, sizeof(ev_tcp_t));
	if (NULL == evtcp)
		return NULL;
	
    evtcp->fd = sock;
    evtcp->type = type;
    evtcp->cb = cb;
    evtcp->arg = arg;
    evtcp->buffer = ev_net_bind_free_buffer();
	
    return evtcp;
}

/*网络事件控制  读、写、错误*/
static void ev_net_cb(ev_ctl_t *evctl, ev_fd_t *evfd, int fd, ev_type_t type, void *arg)
{
    ev_tcp_t *evtcp = arg;
    int error = 0;
	
    switch (type) {
	    case EV_READ:
	        error = ev_tcp_read(evctl, evfd, fd, evtcp);//有数据读取但是是2 则关闭SOCKET
	        break;
	    case EV_WRITE:
	        error = ev_tcp_write(evctl, evfd, fd, evtcp);
	        break;
	    case EV_ERROR:
	        error = 1;
	        break;
	    default:
	        return;
    }
	printf("+++++++error:%d,type:%d\r\n",error,type);
    if (1 == error) {
        evtcp->cb(evctl, evtcp, EV_TCP_ERROR, evtcp->arg);
    } else if (2 == error) {
        evtcp->cb(evctl, evtcp, EV_TCP_CLOSE, evtcp->arg);
    }
}

/*TCP accep 客户端连接*/
static void ev_accept_cb(ev_ctl_t *evctl, ev_fd_t *evfd, int fd, ev_type_t type, void *arg)
{
    ev_tcp_t *accepter = arg;//服务器TCP listen socket  ev_tcp_t
    int sock = accept(fd, NULL, NULL);
    if (-1 == sock) {
		return;
    }
	
	if(-1 == fcntl(sock, F_SETFL, fcntl(sock, F_GETFL) | O_NONBLOCK)) {
		close(sock);
		return;
	}
	
    ev_tcp_t *evtcp = ev_tcp_create(sock, EV_TCP_NORMAL, accepter->cb, accepter->arg);//创建与此客户端通信得tcp socket
	if (NULL == evtcp) {
		return;
	}
	
    evtcp->evfd = evctl->add(evctl, sock, ev_net_cb, evtcp);
    if (!evtcp->evfd) {
		ev_tcp_close(evctl, evtcp);
        return;
    }
	
    evctl->watch_read(evctl, evtcp->evfd);
    accepter->cb(evctl, evtcp, EV_TCP_ACCEPT, accepter->arg);
}

/*启动TCP 服务器*/
ev_tcp_t *ev_tcp_serv_start(ev_ctl_t *evctl, char *ipaddr, uint16_t port, ev_tcp_cb_t cb, void *arg)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == sock) 
		goto err1;

	int reuse = 1;
    if (-1 == setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)))
		goto err2;
	
	if (-1 == fcntl(sock, F_SETFL, fcntl(sock, F_GETFL) | O_NONBLOCK))
		goto err2;

    struct sockaddr_in addr;
	memset(&addr, 0 , sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
	if(NULL == ipaddr)
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
	else
    	addr.sin_addr.s_addr = inet_addr(ipaddr);
	
    if (-1 == bind(sock, (struct sockaddr *)&addr, sizeof(addr)))
		goto err2;
	
    if (-1 == listen(sock, MAXCLIENTS))
		goto err2;

    ev_tcp_t *evtcp = ev_tcp_create(sock, EV_TCP_LISTEN, cb, arg);
	if (NULL == evtcp)
		goto err2;
	
    evtcp->evfd = evctl->add(evctl, sock, ev_accept_cb, evtcp);
    if (!evtcp->evfd) {
 		goto err3;
    }
	
    evctl->watch_read(evctl, evtcp->evfd);
	
	return evtcp;
	
err3:
	ev_net_unbind_using_buffer(evtcp->buffer);
	free(evtcp);
err2:
	close(sock);
err1:
	return NULL;
}

/*TCP 连接控制*/
static void ev_connect_cb(ev_ctl_t *evctl, ev_fd_t *evfd, int fd, ev_type_t type, void *arg)
{
    ev_tcp_t *evtcp = arg;
    
    int ret, error;
    socklen_t optlen = sizeof(error);
    switch (type) {
    case EV_WRITE:
        ret = getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &optlen);
        if (ret == 0 && error == 0) {
			//printf("ev write...\n");
            evtcp->type = EV_TCP_NORMAL;
            evctl->watch_read(evctl, evfd);
            evctl->unwatch_write(evctl, evfd);
            evctl->set(evctl, evfd, ev_net_cb, evtcp);
			evtcp->cb(evctl, evtcp, EV_TCP_CONNECTED, evtcp->arg);
        }
        break;
    case EV_ERROR:
        evtcp->cb(evctl, evtcp, EV_TCP_ERROR, evtcp->arg);
        break;
    default:
        break;
    }
}

/*启动客户端连接*/
ev_tcp_t *ev_tcp_client_start(ev_ctl_t *evctl, char *ipaddr, uint16_t port, ev_tcp_cb_t cb, void *arg)
{
	struct addrinfo *res, hint;
	fd_set rest, west;
	memset(&hint, 0, sizeof(hint));
	hint.ai_family = AF_INET;
	hint.ai_socktype = SOCK_STREAM;
	if (0 != getaddrinfo(ipaddr, NULL, &hint, &res)) {
		goto err0;
	}

	int sock = socket(res->ai_family, res->ai_socktype, 0);
	if (-1 == sock)
		goto err1;
	if(-1 == fcntl(sock, F_SETFL, fcntl(sock, F_GETFL) | O_NONBLOCK))
		goto err2;
	
    struct sockaddr_in addr;
	
	memset(&addr, 0 , sizeof(addr));
    addr.sin_family = res->ai_family;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = ((struct sockaddr_in *)(res->ai_addr))->sin_addr.s_addr;

    int ret = connect(sock, (struct sockaddr *)&addr, sizeof(addr));
    if (-1 == ret && EINPROGRESS != errno)
       goto err2;

	/*sky add select timeout connect no response goto err2*/
    FD_ZERO(&rest);
    FD_ZERO(&west);
    FD_SET(sock, &rest);
    FD_SET(sock, &west);
	struct timeval tempval;
    tempval.tv_sec = 2;
    tempval.tv_usec = 0;
    int flag = select(sock+1, &rest, &west, NULL, &tempval);//监听套接的可读和可写条件
    if(flag < 0) 
	{
		 goto err2;
    }
	else
	{
		if(!FD_ISSET(sock, &rest) && !FD_ISSET(sock, &west)) 
		{
			goto err2;
		}
	}

    ev_tcp_t *evtcp = ev_tcp_create(sock, ret == 0 ? EV_TCP_NORMAL : EV_TCP_CONNECT, cb, arg);
	if (NULL == evtcp)
		goto err2;
	
	evtcp->evfd = evctl->add(evctl, sock, ret == 0 ? ev_net_cb : ev_connect_cb, evtcp);
    if (NULL == evtcp->evfd) {
        ev_tcp_close(evctl, evtcp);		
		goto err1;
    }
	
    if (0 == ret)
    {
        evctl->watch_read(evctl, evtcp->evfd);
    }
    else
    {
        evctl->watch_write(evctl, evtcp->evfd);
    }
	freeaddrinfo(res);
    return evtcp;
	
err2:
	close(sock);
err1:
	freeaddrinfo(res);
err0:
	return NULL;
}

/*TCP 断开 移除事件*/
void ev_tcp_detach(ev_ctl_t *evctl, ev_tcp_t *evtcp)
{
    evctl->del(evctl, evtcp->evfd);
    evtcp->evfd = NULL;
}

/*TCP 监听 连接 正常通信*/
int ev_tcp_attach(ev_ctl_t *evctl, ev_tcp_t *evtcp)
{
    switch (evtcp->type) {
	    case EV_TCP_LISTEN:
	        evtcp->evfd = evctl->add(evctl, evtcp->fd, ev_accept_cb, evtcp);
	        if (!evtcp->evfd)
	            return -1;
	        evctl->watch_read(evctl, evtcp->evfd);
	        break;
	    case EV_TCP_CONNECT:
	        evtcp->evfd = evctl->add(evctl, evtcp->fd, ev_connect_cb, evtcp);
	        if (!evtcp->evfd)
	            return -1;
	        evctl->watch_write(evctl, evtcp->evfd);
	        break;
	    case EV_TCP_NORMAL:
	        evtcp->evfd = evctl->add(evctl, evtcp->fd, ev_net_cb, evtcp);
	        if (!evtcp->evfd)
	            return -1;
	        evctl->watch_read(evctl, evtcp->evfd);
	        if (evtcp->buffer->woff)
				evctl->watch_write(evctl, evtcp->evfd);
	        break;
	    default:
	        return -1;
    }
    return 0;
}
#endif

/*****************************UDP 连接*/
static ev_udp_t *ev_udp_create(int sock, void *arg)
{
    ev_udp_t *evudp = calloc(1, sizeof(ev_udp_t));
	if (NULL == evudp)
		return NULL;
	
    evudp->fd = sock;
    //evudp->type = type;
    //evudp->cb = cb;
    evudp->arg = arg;
    evudp->buffer = ev_net_bind_free_buffer();
	
    return evudp;
}

/*TCP 读取数据到缓冲区*/
static int ev_udp_read(T_EVENT_CTL *evctl, T_EVENT_FD *evfd, int fd, ev_udp_t *evudp)
{
	int cliend_addr_len = sizeof(struct sockaddr_in);
	struct sockaddr_in *client_addr;
	int len = recvfrom(fd,  evudp->buffer->r + evudp->buffer->roff, EV_HALFBUF_SIZE - evudp->buffer->roff, 0, (struct sockaddr*)client_addr, &cliend_addr_len);
	if(-1 == len)
		return 1;
	evudp->buffer->rlen = len;
    evudp->buffer->roff += len;
	evudp->cb(evctl, evudp,evudp->arg);
		
    return 0;
}

/*发送缓冲区数据*/
static int ev_udp_write(T_EVENT_CTL *evctl, T_EVENT_FD *evfd, int fd, ev_udp_t *evtcp)
{
   
	
    return 0;
}

/*网络事件控制  读、写、错误*/
static void ev_udpnet_cb(T_EVENT_CTL *evctl, T_EVENT_FD *evfd, int fd, E_EV_TYPE type, void *arg)
{
    ev_udp_t *evudp = arg;
    int error = 0;
	
    switch (type) {
	    case EV_READ:
			 //printf("read.....\r\n");
	        error = ev_udp_read(evctl, evfd, fd, evudp);
	        break;
	    case EV_WRITE:
		     printf("write.....\r\n");
	        error = ev_udp_write(evctl, evfd, fd, evudp);
	        break;
	    case EV_ERROR:
	        error = 1;
	        break;
	    default:
	        return;
    }
}

/*启动UDP server*/
ev_udp_t *udp_start(T_EVENT_CTL *evctl, char *ipaddr, uint16_t port, ev_udp_cb_t cb,void *arg)
{
	 struct sockaddr_in udpaddr;
	 int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	 if(-1 == sockfd)
	 	goto err1;
	 int on = 1;
	 setsockopt(sockfd,SOL_SOCKET,SO_BROADCAST,&on,sizeof(on)); 
	 bzero(&udpaddr, sizeof(udpaddr));
	 udpaddr.sin_family = AF_INET;
	 if(ipaddr == NULL)
	 udpaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	 else
	 udpaddr.sin_addr.s_addr = inet_addr(ipaddr);
	 
	 udpaddr.sin_port = htons(port);
	 if(-1 == bind(sockfd, (struct sockaddr*)&udpaddr, sizeof(udpaddr)))
	 	goto err2;
	  ev_udp_t *evudp = ev_udp_create(sockfd, arg);
	  evudp->cb = cb;
	  evudp->evfd =EVIO_EventFd_Add(evctl,sockfd,ev_udpnet_cb,evudp);
	  EVIO_Event_Watch_Read(evctl, evudp->evfd);
	 return evudp;
err2:
    printf("udp  bind  error.....\r\n");
	close(sockfd);
err1:
	printf("udp  socket error.....\r\n");
	return NULL;
}