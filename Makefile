#include ./build.cfg
CFLAGS = -O2 -Wall -g
LDFLAGS=-L/home/cftc/sunzhguy/NXP_IM6ULL/PIS_system/ffmpeg_lib/lib
LIB_PATH+=-L/home/cftc/sunzhguy/NXP_IM6ULL/rootfs/tools_porting/libasound/alsa-lib/lib


CFLAGS += -I ./src
CFLAGS += -I ./src/evio
CFLAGS += -I ./src/hash
CFLAGS += -I ./src/net
CFLAGS += -I ./src/timer


#nanomsg
CFLAGS += -I ./src/nanomsg
GFLAGS += -I ./src/nanomsg/aio
CFLAGS += -I ./src/nanomsg/core
CFLAGS += -I ./src/nanomsg/devices
CFLAGS += -I ./src/nanomsg/protocols
CFLAGS += -I ./src/nanomsg/protocols/utils
CFLAGS += -I ./src/nanomsg/protocols/bus
CFLAGS += -I ./src/nanomsg/protocols/pair
CFLAGS += -I ./src/nanomsg/protocols/pipeline
CFLAGS += -I ./src/nanomsg/protocols/pubsub
CFLAGS += -I ./src/nanomsg/protocols/reqrep
CFLAGS += -I ./src/nanomsg/protocols/survey
CFLAGS += -I ./src/nanomsg/protocols/utils
CFLAGS += -I ./src/nanomsg/transports/inproc
CFLAGS += -I ./src/nanomsg/transports/ipc
CFLAGS += -I ./src/nanomsg/transports/tcp
CFLAGS += -I ./src/nanomsg/transports/utils
CFLAGS += -I ./src/nanomsg/utils

CFLAGS += -DNN_SHARED_LIB

CFLAGS += -DNN_HAVE_EVENTFD=1
CFLAGS += -DHAVE_PIPE=1
CFLAGS += -DNN_HAVE_PIPE=1
CFLAGS += -DHAVE_PIPE2=1
CFLAGS += -DNN_HAVE_PIPE2=1
CFLAGS += -DNN_HAVE_CLOCK_MONOTONIC=1

CFLAGS += -DNN_USE_POLL
CFLAGS += -DNN_HAVE_POLL=1

#CFLAGS += -DNN_USE_EPOLL=1
CFLAGS += -DNN_HAVE_ACCEPT4=1
CFLAGS += -DNN_HAVE_SOCKETPAIR=1
CFLAGS += -DNN_HAVE_SEMAPHORE=1
CFLAGS += -DNN_HAVE_GCC_ATOMIC_BUILTINS=1
CFLAGS += -DNN_USE_EVENTFD=1
CFLAGS += -DNN_HAVE_MSG_CONTROL=1


CFLAGS += -I/home/cftc/sunzhguy/NXP_IM6ULL/PIS_system/ffmpeg_lib/include
INC_PATH += -I/home/cftc/sunzhguy/NXP_IM6ULL/rootfs/tools_porting/libasound/alsa-lib/include/

TOP_DIR = .
INSTALL_DIR = /bin
PROJECT = Pisc.axf
DEBUG_DIR = /debug


#User
SOURCE  = src/main.c
SOURCE  += src/udp_service.c
#evio
SOURCE  += src/evio/evio.c

#hash
SOURCE  += src/hash/shash.c

#mini-heap
SOURCE  += src/mini-heap/mini-heap.c

#nanomsg
SOURCE  += src/nanomsg/core/ep.c
SOURCE  += src/nanomsg/core/global.c
SOURCE  += src/nanomsg/core/pipe.c
SOURCE  += src/nanomsg/core/poll.c
SOURCE  += src/nanomsg/core/sock.c
SOURCE  += src/nanomsg/core/sockbase.c
SOURCE  += src/nanomsg/core/symbol.c

SOURCE  += src/nanomsg/aio/ctx.c
SOURCE  += src/nanomsg/aio/fsm.c
SOURCE  += src/nanomsg/aio/pool.c
SOURCE  += src/nanomsg/aio/timer.c
SOURCE  += src/nanomsg/aio/timerset.c
SOURCE  += src/nanomsg/aio/usock.c
SOURCE  += src/nanomsg/aio/worker.c

SOURCE  += src/nanomsg/utils/alloc.c
SOURCE  += src/nanomsg/utils/atomic.c
SOURCE  += src/nanomsg/utils/chunk.c
SOURCE  += src/nanomsg/utils/chunkref.c
SOURCE  += src/nanomsg/utils/clock.c
SOURCE  += src/nanomsg/utils/closefd.c
SOURCE  += src/nanomsg/utils/efd.c

SOURCE  += src/nanomsg/utils/err.c
SOURCE  += src/nanomsg/utils/hash.c
SOURCE  += src/nanomsg/utils/list.c
SOURCE  += src/nanomsg/utils/msg.c
SOURCE  += src/nanomsg/utils/condvar.c
SOURCE  += src/nanomsg/utils/mutex.c
SOURCE  += src/nanomsg/utils/once.c
SOURCE  += src/nanomsg/utils/queue.c
SOURCE  += src/nanomsg/utils/random.c
SOURCE  += src/nanomsg/utils/sem.c
SOURCE  += src/nanomsg/utils/sleep.c
SOURCE  += src/nanomsg/utils/strcasecmp.c
SOURCE  += src/nanomsg/utils/strcasestr.c
SOURCE  += src/nanomsg/utils/strncasecmp.c
SOURCE  += src/nanomsg/utils/thread.c
SOURCE  += src/nanomsg/utils/wire.c

SOURCE  += src/nanomsg/devices/device.c

SOURCE  += src/nanomsg/protocols/utils/dist.c
SOURCE  += src/nanomsg/protocols/utils/excl.c
SOURCE  += src/nanomsg/protocols/utils/fq.c
SOURCE  += src/nanomsg/protocols/utils/lb.c
SOURCE  += src/nanomsg/protocols/utils/priolist.c
SOURCE  += src/nanomsg/protocols/bus/bus.c
SOURCE  += src/nanomsg/protocols/bus/xbus.c
SOURCE  += src/nanomsg/protocols/pipeline/push.c
SOURCE  += src/nanomsg/protocols/pipeline/pull.c
SOURCE  += src/nanomsg/protocols/pipeline/xpull.c
SOURCE  += src/nanomsg/protocols/pipeline/xpush.c
SOURCE  += src/nanomsg/protocols/pair/pair.c
SOURCE  += src/nanomsg/protocols/pair/xpair.c
SOURCE  += src/nanomsg/protocols/pubsub/pub.c
SOURCE  += src/nanomsg/protocols/pubsub/sub.c
SOURCE  += src/nanomsg/protocols/pubsub/trie.h
SOURCE  += src/nanomsg/protocols/pubsub/trie.c
SOURCE  += src/nanomsg/protocols/pubsub/xpub.h
SOURCE  += src/nanomsg/protocols/pubsub/xpub.c
SOURCE  += src/nanomsg/protocols/pubsub/xsub.c
SOURCE  += src/nanomsg/protocols/reqrep/req.c
SOURCE  += src/nanomsg/protocols/reqrep/rep.c
SOURCE  += src/nanomsg/protocols/reqrep/task.c
SOURCE  += src/nanomsg/protocols/reqrep/xrep.c
SOURCE  += src/nanomsg/protocols/reqrep/xreq.c
SOURCE  += src/nanomsg/protocols/survey/respondent.c
SOURCE  += src/nanomsg/protocols/survey/surveyor.c
SOURCE  += src/nanomsg/protocols/survey/xrespondent.c
SOURCE  += src/nanomsg/protocols/survey/xsurveyor.c

SOURCE  += src/nanomsg/transports/utils/backoff.c
SOURCE  += src/nanomsg/transports/utils/dns.c
#SOURCE  += src/nanomsg/transports/utils/dns_getaddrinfo.inc
SOURCE  += src/nanomsg/transports/utils/iface.c
SOURCE  += src/nanomsg/transports/utils/literal.c
SOURCE  += src/nanomsg/transports/utils/port.c
SOURCE  += src/nanomsg/transports/utils/streamhdr.c
SOURCE  += src/nanomsg/transports/utils/base64.c
SOURCE  += src/nanomsg/transports/inproc/binproc.c
SOURCE  += src/nanomsg/transports/inproc/cinproc.c
SOURCE  += src/nanomsg/transports/inproc/inproc.c
SOURCE  += src/nanomsg/transports/inproc/ins.c
SOURCE  += src/nanomsg/transports/inproc/msgqueue.c
SOURCE  += src/nanomsg/transports/inproc/sinproc.c
SOURCE  += src/nanomsg/transports/ipc/aipc.c
SOURCE  += src/nanomsg/transports/ipc/bipc.c
SOURCE  += src/nanomsg/transports/ipc/cipc.c
SOURCE  += src/nanomsg/transports/ipc/ipc.c
SOURCE  += src/nanomsg/transports/ipc/sipc.c

SOURCE  += src/nanomsg/transports/tcp/atcp.c
SOURCE  += src/nanomsg/transports/tcp/btcp.c
SOURCE  += src/nanomsg/transports/tcp/ctcp.c
SOURCE  += src/nanomsg/transports/tcp/stcp.c
SOURCE  += src/nanomsg/transports/tcp/tcp.c


SOURCE  += src/nanomsg/transports/ws/aws.c

SOURCE  += src/nanomsg/transports/ws/bws.c

SOURCE  += src/nanomsg/transports/ws/cws.c

SOURCE  += src/nanomsg/transports/ws/sws.c
SOURCE  += src/nanomsg/transports/ws/ws.c

SOURCE  += src/nanomsg/transports/ws/ws_handshake.c
SOURCE  += src/nanomsg/transports/ws/sha1.c

		################unix##############
#SOURCE  += src/nanomsg/aio/usock_posix.inc
#SOURCE  += src/nanomsg/aio/worker_posix.inc
#SOURCE  += src/nanomsg/utils/thread_posix.inc

		###############NN_HAVE_POLL)
    	#add_definitions (-DNN_USE_POLL)
#SOURCE  += src/nanomsg/aio/poller_poll.inc
SOURCE   += src/nanomsg/aio/poller.c
		##############NN_HAVE_EVENTFD
#SOURCE  += src/nanomsg/utils/efd_eventfd.h
#SOURCE  += src/nanomsg/utils/efd_eventfd.c


#evio
SOURCE  += src/net/evnet.c

#evtimer
SOURCE  += src/timer/evtimer.c

######################################zlog################################
SOURCE  +=$(wildcard src/zlog/*.c)




SUBDIRS := $(shell find src -type d)

#$(shell mkdir -p $(TOP_DIR)$(INSTALL_DIR))

$(shell for val in $(SUBDIRS);do \
mkdir -p $(TOP_DIR)$(INSTALL_DIR)/$${val}; \
done;)
$(shell cp zlog_config/* bin/)

OBJS = $(SOURCE:%.c=$(TOP_DIR)$(INSTALL_DIR)/%.o)

all:$(TOP_DIR)$(INSTALL_DIR)/$(PROJECT)
$(TOP_DIR)$(INSTALL_DIR)/%o:%c

$(TOP_DIR)$(INSTALL_DIR)/$(PROJECT):$(OBJS) $(HEADERS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)  -lpthread  


$(TOP_DIR)$(INSTALL_DIR)/%.o:%.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@  

install:
	cp $(TOP_DIR)$(INSTALL_DIR)/$(PROJECT) $(IMAGE_DIR)/userfs/bin

############################################################
debug:
#$(TOP_DIR)$(INSTALL_DIR)/%o:%c

#$(TOP_DIR)$(INSTALL_DIR)/$(PROJECT):$(OBJS) $(HEADERS)
#	$(CC) $(CFLAGS) -o $@ $^ -lpthread -g

#$(TOP_DIR)$(INSTALL_DIR)/%.o:%.c $(HEADERS)
#	$(CC) $(CFLAGS) -c $< -o $@
	arm-linux-insight $(TOP_DIR)$(INSTALL_DIR)/$(PROJECT)
#	insight $(TOP_DIR)$(INSTALL_DIR)/$(PROJECT)
#	ddd -debugger arm-linux-gdb $(TOP_DIR)$(INSTALL_DIR)/$(PROJECT)

############################################################
clean:
	-rm -rf $(TOP_DIR)$(INSTALL_DIR)
test:
	insight $(TOP_DIR)$(INSTALL_DIR)/$(PROJECT)


zlog:
	@echo "get SOUCE-->C File"$(SOURCEx)