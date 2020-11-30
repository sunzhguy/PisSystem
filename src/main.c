/*
 * @Descripttion: 
 * @vesion: 
 * @Author: sunzhguy
 * @Date: 2020-07-15 11:02:55
 * @LastEditor: sunzhguy
 * @LastEditTime: 2020-11-30 14:31:00
 */ 


#include <stdio.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include "evio/evio.h"
#include "net/evnet.h"
#include "hash/shash.h"
#include <assert.h>
#include "nanomsg/pair.h"
#include "nanomsg/nn.h"
#include "udp_service.h"
#include "main.h"



void timer_out_ctrl(void *_self, ev_timer_t *ev_timer,  void *arg)
{
    ev_ctl_t * evctl = (ev_ctl_t * )_self;
    printf("++++++++++++++++++++++++++++++++++++++++timout->index:%d tm:%d,%p\r\n",ev_timer->index,ev_timer->expire,evctl);
	ev_init_timer(ev_timer,1000,timer_out_ctrl,NULL);
    ev_start_timer(evctl,ev_timer);
}

#if 0
void *pthread_func(void*arg)
{

	int n = nn_socket(AF_SP, NN_PAIR);
 
    if (nn_connect(n, "inproc://b2a_loop") < 0) {
        return -1;
    }
    size_t size = sizeof(size_t);
	int s=0;
    if (nn_getsockopt(n, NN_SOL_SOCKET, NN_RCVFD, &s, &size) < 0) {
        return -1;
    }

while(1)
{
	sleep(1);
	printf("hello world\r\n");
	char *str = "OK";
    uint8_t *dat = nn_allocmsg(3, 0);
    if (NULL != dat) {
        memcpy(dat, str, 3);
	 nn_send(n, &dat, NN_MSG, NN_DONTWAIT);
	}
	
}

	
}

static void watcher_B2A_cb (ev_ctl_t *evctl, ev_fd_t *evfd, int fd, ev_type_t type, void *arg)
{
   
    uint8_t *dat = NULL;
	int n = *(int*)arg;
    uint32_t bytes = nn_recv(n, &dat, NN_MSG, NN_DONTWAIT);
    if (bytes <= 0) {
        return;
    }
    //杞彂鍒癇
    printf("A:%s (B->A)\r\n", (char *)dat);
	nn_freemsg(dat);
}

#endif


void handle_udp_event(ev_ctl_t *ctl, struct servfds *sfd)
{
	struct server *s = sfd->arg;
	printf("handle udp event....\r\n");
}

static void main_loop_cb(ev_ctl_t *evctl, ev_fd_t *evfd, int fd, ev_type_t type, void *arg)
{
	struct servfds *sfd = arg;
	struct server *s = sfd->arg;

    switch (type) {
	    case EV_READ:
	        sfd->cb(evctl, sfd);
	        break;
		case EV_WRITE:
			printf("Main loop write event, unexpected");
			break;
		case EV_ERROR:
			printf("Main loop error event, unexpected");
			break;
	    default:
			printf("Main loop unknow event, unexpected");
    }
}

int32_t nn_socket_init(struct server *s)
{
	struct servloop *sl = &s->sloop;
	size_t size = sizeof(size_t);
	/*sl->sfds_udp.n = nn_socket(AF_SP, NN_PAIR);
	if (-1 == sl->sfds_udp.n)
		goto err;
	if (-1 == nn_bind(sl->sfds_udp.n, "inproc://udp2serv"))
		goto err1;
	
	if (-1 == nn_getsockopt(sl->sfds_udp.n, NN_SOL_SOCKET, NN_RCVFD, (char *)&sl->sfds_udp.s, &size))
		goto err1;
	*/
	return 0;
err1:
	//nn_close(sl->sfds_udp.n);
err:
	return -1;
}
void nn_socket_close(struct server *s)
{
	struct servloop *sl = &s->sloop;
	//close(sl->sfds_udp.s);
	//nn_close(sl->sfds_udp.n);

}
int32_t  main_loop(struct server *s)
{
	
	struct servloop *sl = &s->sloop;
	if (-1 == nn_socket_init(s))
		return -1;

	sl->sfds_udp.cb = handle_udp_event;//UDP enventloop main server
    sl->sfds_udp.arg = s;

	 s->sloop.evctl = evctl_create();  //创建一个事件控制器 creator event contoler
	 if(NULL == s->sloop.evctl)
	 goto err1;
	 
    sl->sfds_udp.evfd = ev_fd_add(sl->evctl, sl->sfds_udp.s, main_loop_cb, &sl->sfds_udp);//add the event fd 
	if (NULL == sl->sfds_udp.evfd)
		goto err2;
	 ev_watch_read(s->sloop.evctl, sl->sfds_udp.evfd);

	 ev_init_timer(&s->ev_timer,1000,timer_out_ctrl,NULL);
	 ev_start_timer(s->sloop.evctl,&s->ev_timer);

	 return 0;
err2:
	evctl_free(sl->evctl);
err1:
	//nn_socket_close(s);
	return -1;
}

void main_loop_del(struct server *s)
{
	struct servloop *sl = &s->sloop;
	ev_fd_del(sl->evctl, sl->sfds_udp.evfd);
	evctl_free(sl->evctl);
	nn_socket_close(s);
}
int main( void )
{
     int i =0;
	 int ret =0;
	 struct server server;
	 pthread_t thread_udp;
	 
	 ret = main_loop(&server);
	 if(-1 == ret)
	 goto err1;
     server.start_num = 0;
	 pthread_mutex_init(&server.start_lock, NULL);
	 pthread_cond_init(&server.start_cond, NULL);

	 ret =pthread_create(&thread_udp,NULL,udp_service,&server);
	 if(-1 == ret)
	 goto err2;

	 while (server.start_num < 1) {
        pthread_mutex_lock(&server.start_lock);
        pthread_cond_wait(&server.start_cond, &server.start_lock);
        pthread_mutex_unlock(&server.start_lock);
        printf("start num is %d", server.start_num);    
  	}
     while(1)
     {
        ev_loop_start(server.sloop.evctl);
     }
err2:
	printf("main err2...\r\n");
	 main_loop_del(&server);
err1:
	printf("main err1....\r\n");
	 return -1;

        
}