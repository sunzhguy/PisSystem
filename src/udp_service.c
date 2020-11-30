/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-07-22 08:40:25
 * @LastEditor: sunzhguy
 * @LastEditTime: 2020-11-30 11:22:56
 */ 
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "net/evnet.h"
#include "udp_service.h"
#include "evio/evio.h"
#include "main.h"

struct netfds {
	int s;			/* sys fd*/
	int n;			/* nanomsg fd */
	ev_fd_t *evfd;
	void (*cb)(ev_ctl_t *, struct netfds *);
	void *arg;
};

struct udp_net_ctl {
	char initflag;
	ev_ctl_t *evctl;
	struct netfds s2udp;
	struct server *arg;
}udp_net_ctl;


 void ev_udp_recv_cb(ev_ctl_t *evctl, ev_udp_t * evudp , void *arg)
 {
     int beg =0;
     ev_buffer_t *b = evudp->buffer;
     printf("UDP  recve....%s\r\n",b->r);
     b->roff = 0x00;
 }

void *udp_broadcast_service(void *arg)
{
     ev_ctl_t * ev_ctrl = evctl_create();
    if(ev_ctrl == NULL)
      goto err;
     ev_udp_t *udpcon = udp_start(ev_ctrl,"168.168.102.255",5555,ev_udp_recv_cb,NULL);
	 if(udpcon == NULL)
     {
          printf("UDP init error\r\n");
          goto err2;
     }
     while(1)
     {
        ev_loop_start(ev_ctrl);
     }
err3:
    ev_fd_del(ev_ctrl,udpcon->evfd);
err2:
    evctl_free(ev_ctrl);
err:
    abort();
}

void handle_data_udp_form_server(ev_ctl_t *evctl, struct netfds *s2udp)
{


}

static void server_to_udp_cb(ev_ctl_t *evctl, ev_fd_t *evfd, int fd, ev_type_t type, void *arg)
{
	struct netfds *fds = arg;
	struct net_ctl *ctl = fds->arg;
	//struct server *s = ctl->arg;

    switch (type) {
	    case EV_READ:
	        fds->cb(evctl, fds);
	        break;
		case EV_WRITE:
			printf( "Unexpected write event");
			break;
		case EV_ERROR:
			printf( "Unexpected error event");
			break;
	    default:
			printf( "Unexpected event");
	        break;
    }
}

void udp_ctrl_init(struct udp_net_ctl *ctl)
{
	#if 0
    ctl->s2udp.n = nn_socket(AF_SP, NN_PAIR);
	if (-1 == ctl->s2udp.n)
		goto err1;
	if (-1 == nn_connect(ctl->s2udp.n, "inproc://ss2serv"))
		goto err2;
	size_t size = sizeof(size_t);
	if (-1 == nn_getsockopt(ctl->s2udp.n, NN_SOL_SOCKET, NN_RCVFD, (char *)&ctl->s2udp.s, &size))
		goto err2;
	#endif
	ctl->evctl = evctl_create();
	if (NULL == ctl->evctl)
		goto err3;
	
	#if 0
    ctl->s2udp.cb = handle_data_udp_form_server;
    ctl->s2udp.arg = ctl;
    ctl->s2udp.evfd = ev_fd_add(ctl->evctl, sl->sfds_udp.s, server_to_udp_cb, &sl->sfds_udp);//add the event fd 
	if (NULL == ctl->ss.evfd)
		goto err4;
	
    ctl->evctl->watch_read(ctl->evctl, ctl->ss.evfd);
	#endif
  err5:
	//ctl->evctl->del(ctl->evctl, ctl->ss.evfd);
err4:
	evctl_free(ctl->evctl);
err3:
	//close(ctl->s2udp.s);
err2:
	//nn_close(ctl->s2udp.n);
err1:
	return -1;
}

void *udp_service(void *arg)
{
     pthread_t thread_ubroadt;
     struct server *s =arg;
     ev_ctl_t * ev_ctrl = evctl_create();
     if(ev_ctrl == NULL)
      goto err;
    //assert(0 == pthread_create(&thread_ubroadt, NULL,udp_broadcast_service,s));
	pthread_create(&thread_ubroadt, NULL,udp_broadcast_service,s);

    pthread_mutex_lock(&s->start_lock);
	++s->start_num;
	pthread_cond_signal(&s->start_cond);
	pthread_mutex_unlock(&s->start_lock);
    
     while(1)
     {
        ev_loop_start(ev_ctrl);
     }

err2:
    evctl_free(ev_ctrl);
err:
    abort();
}