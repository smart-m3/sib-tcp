/*

  Copyright (c) 2009, Nokia Corporation
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:
  
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.  
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in
    the documentation and/or other materials provided with the
    distribution.  
    * Neither the name of Nokia nor the names of its contributors 
    may be used to endorse or promote products derived from this 
    software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
  COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 */

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <glib.h>
#include <dbus/dbus-glib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <whiteboard_log.h>
#include <sib_object.h> // DBus connection to sib-daemon
#include <sibmsg.h> // SSAP parser 'n' gen
//#include <sibdefs.h> // EncodingType
#include <uuid/uuid.h>

#define MAX_THREADS 2000
#define SIB_PORT_DFLT 10010    // the port users will be connecting to

#define BACKLOG 2000     // how many pending connections queue will hold

#define RECVBUFSIZE 128

#define SUBWAKEFREQ 10


typedef struct _Client
{
  int fd;

  guchar *recvbuf;
  gint len;
  gint remaining_len;

  
} Client;


typedef struct _Waiter
{
  GMutex *lock;
  GCond *cond;
  gboolean req_ready;
  int fd;
  NodeMsgContent_t *rsp;
  guchar *subscription_id; /* for unsubscribe */

  guchar *key;

} Waiter;

typedef struct _Listener
{

  int lfd; /* listen socket */
  GSList *open_sockets; /* client sockets */
  GHashTable *clients; /* fd -> Client */
  GMutex *client_lock;
  SibObject *object;
  GHashTable *requests; /* msgid -> waiter */
  GMutex *reqlock;

  GHashTable *subscriptions; /*subsid ->waiter */
  GMutex *sublock;

} Listener;

GMainLoop *mainloop = NULL;
GThreadPool *threadpool = NULL;
Listener *listener = NULL;
SibObject *control = NULL;
int port = SIB_PORT_DFLT;
int subwakefr= SUBWAKEFREQ;

static void refresh_cb(SibObject* context,
		       gpointer user_data);
static void shutdown_cb(SibObject* context,
			gpointer user_data);
static void healthcheck_cb(SibObject* context,
			   gpointer user_data);


static void handle_join_cnf( SibObject *context,
			     guchar *spaceid,
			     guchar *nodeid,
			     gint msgnum,
			     gint success,
			     guchar *credentials,
			     gpointer user_data);

static void handle_leave_cnf( SibObject *context,
			      guchar *spaceid,
			      guchar *nodeid,
			      gint msgnum,
			      gint success,
			      gpointer user_data);

static void handle_insert_cnf( SibObject *context,
			       guchar *spaceid,
			       guchar *nodeid,
			       gint msgnum,
			       gint success,
			       guchar *bNodes,
			       gpointer user_data);
static void handle_remove_cnf( SibObject *context,
			       guchar *spaceid,
			       guchar *nodeid,
			       gint msgnum,
			       gint success,
			       gpointer user_data);
static void handle_update_cnf( SibObject *context,
			       guchar *spaceid,
			       guchar *nodeid,
			       gint msgnum,
			       gint success,
			       guchar *bNodes,
			       gpointer user_data);
static void handle_query_cnf( SibObject *context,
			      guchar *spaceid,
			      guchar *nodeid,
			      gint msgnum,
			      gint success,
			      guchar *results,
			      gpointer user_data);
static void handle_subscribe_cnf( SibObject *context,
				  guchar *spaceid,
				  guchar *nodeid,
				  gint msgnum,
				  gint success,
				  guchar *subscription_id,
				  guchar *results,
				  gpointer user_data);
static void handle_subscription_ind( SibObject *context,
				     guchar *spaceid,
				     guchar *nodeid,
				     gint msgnum,
				     gint sequence_num,
				     guchar *subscription_id,
				     guchar *results_new,
				     guchar *results_obsolete,
				     gpointer user_data);

static void handle_unsubscribe_cnf( SibObject *context,
				    guchar *spaceid,
				    guchar *nodeid,
				    gint msgnum,
				    gint status,
				    guchar *subid,
				    gpointer user_data);
static void handle_unsubscribe_ind( SibObject *context,
				    guchar *spaceid,
				    guchar *nodeid,
				    gint msgnum,
				    gint status,
				    guchar *subscription_id,
				    gpointer user_data);
static void handle_leave_ind( SibObject *context,
			      guchar *spaceid,
			      guchar *nodeid,
			      gint msgnum,
			      gint status,
			      gpointer user_data);


static void register_sib_cb(SibObject* context,
			    SibObjectHandle *handle,
			    gchar *uri,
			    gpointer user_data);


static void parse_args(int argc, char * argv[]);
static void print_usage( char *prog, int ret);

void main_signal_handler(int sig)
{
  static volatile sig_atomic_t signalled = 0;
  if ( 1 == signalled )
    {
      signal(sig, SIG_DFL);
      raise(sig);
    }
  else
    {
      signalled = 1;
      g_main_loop_quit(mainloop);
    }
}


static gpointer listener_thread(gpointer data);

static gpointer sub_wake_thread(gpointer data);

static Listener *listener_new();
static void listener_destroy();
static void listener_handle_request(Listener *listener, Client *client, NodeMsgContent_t *msg);
int listener_create_sib_object(Listener *self );

static Client *client_new(int fd);
static void client_destroy(Client *self);
static void client_handler_thread(gpointer data, gpointer user_data);
static gint client_receive_message(Client *client, NodeMsgContent_t *msg);
static gint client_send_message(int fd, guchar *buf, gint len);

static SibObject *create_control();
static gboolean create_control_idle(gpointer data);
static gboolean create_listener_idle(gpointer data);
static guchar *get_request_key( guchar *nodeid, int msgid);
static int listener_add_request( Listener *self, guchar *nodeid, int msgid, Waiter *waiter);
static Waiter *listener_get_request( Listener *self, guchar *nodeid, int msgid);
static int listener_remove_request( Listener *self, guchar *nodeid, int msgid);

static int listener_add_subscription( Listener *self, guchar *subid, Waiter *waiter);
static Waiter *listener_get_subscription( Listener *self, guchar *subid);
static int listener_remove_subscription( Listener *self, guchar *subid);

static int listener_handle_join(Listener *listener, Client *client, NodeMsgContent_t *msg);
static int listener_handle_leave(Listener *listener, Client *client, NodeMsgContent_t *msg);
static int listener_handle_insert(Listener *listener, Client *client, NodeMsgContent_t *msg);
static int listener_handle_remove(Listener *listener, Client *client, NodeMsgContent_t *msg);
static int listener_handle_update(Listener *listener, Client *client, NodeMsgContent_t *msg);
static int listener_handle_query(Listener *listener, Client *client, NodeMsgContent_t *msg);
static int listener_handle_subscribe(Listener *listener, Client *client, NodeMsgContent_t *msg);
static int listener_handle_unsubscribe(Listener *listener, Client *client, NodeMsgContent_t *msg);


static Waiter *waiter_new(int fd);
int waiter_wait_for_finished(Waiter *self);
int waiter_timed_wait_for_finished(Waiter *self, guint timeout_usec);
void waiter_completed(Waiter *self);
void waiter_destroy(Waiter *self);

int main(int argc, char *argv[])
{
  g_type_init();
  g_thread_init(NULL);
  dbus_g_thread_init();
  GError *gerr = NULL;

  /* Set signal handlers */
  signal(SIGINT, main_signal_handler);
  signal(SIGTERM, main_signal_handler);
  
  parse_args( argc, argv);

  mainloop = g_main_loop_new(NULL, FALSE);
  g_main_loop_ref(mainloop);

  threadpool = g_thread_pool_new( client_handler_thread, 
				  NULL,
				  MAX_THREADS,
				  FALSE,
				  &gerr);
  if(gerr)
    {
      whiteboard_log_debug("Could not create client thread handler: %s", gerr->message);
      g_error_free(gerr);
      exit(1);
    }
  g_idle_add( create_control_idle, NULL);
  g_main_loop_run(mainloop);
  
  g_object_unref( G_OBJECT( control) );
  
  g_main_loop_unref(mainloop);
  
  whiteboard_log_debug("\n\nExiting normally\n\n");
  exit (0);
}

Listener *listener_new()
{
  whiteboard_log_debug_fb();
  Listener *self = g_try_new0( Listener, 1);
  GThread *lt = NULL;
  GThread *sw = NULL;
  GError *gerr = NULL;
  if(!self)
    {
      whiteboard_log_debug("listener_new() : memoro alloc error\n");
      return NULL;
    }
  self->clients = g_hash_table_new_full( g_direct_hash, g_direct_equal, NULL, (GDestroyNotify)client_destroy);
  self->client_lock = g_mutex_new();
  self->requests = g_hash_table_new_full( g_str_hash, g_str_equal, g_free, (GDestroyNotify)waiter_destroy);
  self->reqlock = g_mutex_new();

  self->subscriptions = g_hash_table_new_full( g_str_hash, g_str_equal, g_free, (GDestroyNotify)waiter_destroy);
  self->sublock = g_mutex_new();
  
  if( listener_create_sib_object( self ) < 0 )
    {
      whiteboard_log_error("listener_new() : could not create sib_object\n");
      listener_destroy(self);
      return NULL;
    }
      

  lt = g_thread_create( listener_thread,
			self,
			FALSE,
			&gerr);
  if( gerr)
    {
      whiteboard_log_error("listener_new(): Could not create listener_thread: %s\n", gerr->message );
      g_error_free(gerr);
      g_free(self);
      return NULL;
    }


  sw = g_thread_create( sub_wake_thread,
			self,
			FALSE,
			&gerr);

  if( gerr)
    {
      whiteboard_log_error("listener_new(): Could not create listener_thread: %s\n", gerr->message );
      g_error_free(gerr);
      g_free(self);
      return NULL;
    }

  whiteboard_log_debug_fe();
  return self;
}

int listener_create_sib_object(Listener *self )
{

  gchar tmp[37];
  uuid_t u1;

  whiteboard_log_debug_fb();

  uuid_generate(u1);

  uuid_unparse(u1, tmp);


  self->object = SIB_OBJECT(sib_object_new( tmp,
					    NULL,
					    NULL,
					    "SIB Access",
					    "Not yet done"));
  if(!self->object)
    {
      return -1;
    }
  
  g_signal_connect( G_OBJECT(self->object),
		    SIB_OBJECT_SIGNAL_JOIN_CNF,
		    (GCallback)handle_join_cnf,
		    self);

  g_signal_connect( G_OBJECT(self->object),
		    SIB_OBJECT_SIGNAL_LEAVE_CNF,
		    (GCallback)handle_leave_cnf,
		    self);
      
  g_signal_connect( G_OBJECT(self->object),
		    SIB_OBJECT_SIGNAL_INSERT_CNF,
		    (GCallback)handle_insert_cnf,
		    self);

  g_signal_connect( G_OBJECT(self->object),
		    SIB_OBJECT_SIGNAL_REMOVE_CNF,
		    (GCallback)handle_remove_cnf,
		    self);
  g_signal_connect( G_OBJECT(self->object),
		    SIB_OBJECT_SIGNAL_UPDATE_CNF,
		    (GCallback)handle_update_cnf,
		    self);
  g_signal_connect( G_OBJECT(self->object),
		    SIB_OBJECT_SIGNAL_QUERY_CNF,
		    (GCallback)handle_query_cnf,
		    self);
  g_signal_connect( G_OBJECT(self->object),
		    SIB_OBJECT_SIGNAL_SUBSCRIBE_CNF,
		    (GCallback)handle_subscribe_cnf,
		    self);
  g_signal_connect( G_OBJECT(self->object),
		    SIB_OBJECT_SIGNAL_SUBSCRIPTION_IND,
		    (GCallback)handle_subscription_ind,
		    self);

  g_signal_connect( G_OBJECT(self->object),
		    SIB_OBJECT_SIGNAL_UNSUBSCRIBE_IND,
		    (GCallback)handle_unsubscribe_ind,
		    self);
      
  g_signal_connect( G_OBJECT(self->object),
		    SIB_OBJECT_SIGNAL_UNSUBSCRIBE_CNF,
		    (GCallback)handle_unsubscribe_cnf,
		    self);
      
  g_signal_connect( G_OBJECT(self->object),
		    SIB_OBJECT_SIGNAL_LEAVE_IND,
		    (GCallback)handle_leave_ind,
		    self);
  whiteboard_log_debug_fe();
  return 0;
}

void listener_destroy( )
{
  close( listener->lfd);
}


gpointer sub_wake_thread(gpointer data)
{
	Listener *self = (Listener *)data;
	Waiter *waiter = NULL;
	GHashTableIter iter;
	gchar* key;
	printf ("Time frequency (s)  %d",subwakefr);

	if (subwakefr>0)
	{
		printf ("Wake subscription thread active");
		while (1)
		{
			sleep(subwakefr);

			g_mutex_lock(self->sublock);
			g_hash_table_iter_init(&iter,self->subscriptions);
			while(g_hash_table_iter_next(&iter, (gpointer*)&key, (gpointer*)&waiter))
			{
				g_mutex_lock(waiter->lock);
				waiter->rsp = NULL;
				waiter->req_ready = TRUE;
				g_cond_signal( waiter->cond );
				g_mutex_unlock(waiter->lock);
			}
			g_mutex_unlock(self->sublock);

		}
	}
	printf ("Wake subscription thread closed");
}



gpointer listener_thread(gpointer data)
{
  Listener *self = (Listener *)data;
  int new_fd;  // listen on self->lfd, new connection on new_fd
  gboolean error_found = FALSE;
  struct sockaddr_in my_addr;    // my address information
  int yes=1;
  whiteboard_log_debug_fb();
  if ((self->lfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    whiteboard_log_error("socket: %s\n", strerror(errno));
    self->lfd = -1;
    return NULL;
  }

  if (setsockopt(self->lfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
     whiteboard_log_error("setsockopt: %s\n", strerror(errno));
    exit(1);
  }
  my_addr.sin_family = AF_INET;         // host byte order
  my_addr.sin_port = htons( port );     // short, network byte order
  my_addr.sin_addr.s_addr = INADDR_ANY; // automatically fill with my IP
  memset(my_addr.sin_zero, '\0', sizeof my_addr.sin_zero);
  whiteboard_log_debug("listener: binding socket(%d)\n", self->lfd);

  if (bind( self->lfd, (struct sockaddr *)&my_addr, sizeof (my_addr)) < 0) 
    {
      whiteboard_log_error("Error with bind: %s", strerror(errno));
      close(self->lfd);
      self->lfd = -1;
      g_main_loop_quit(mainloop);
      return NULL;
    }

  whiteboard_log_debug("listener: listening socket(%d)\n", self->lfd);
  if (listen( self->lfd, BACKLOG) < 0) 
    {
      whiteboard_log_error("Error with listen: %s", strerror(errno));
      close(self->lfd);
      self->lfd = -1;
      g_main_loop_quit(mainloop);
      return NULL;
    }

  while( !error_found  ) 
    {  // main accept() loop
      
      new_fd = accept(self->lfd, NULL, NULL);
      if (new_fd  < 0) 
	{
	  whiteboard_log_error("Error with listening socket accept: %s", strerror(errno));
	  error_found = TRUE;
	}
      else
	{
	  Client *client = client_new(new_fd);
	  whiteboard_log_debug("Accepted new Client: %p for socket %d (Listener: %p)\n", client, new_fd, self);
	  g_mutex_lock(self->client_lock);
	  if(client && (g_hash_table_lookup(self->clients, GINT_TO_POINTER(new_fd)) == NULL))
	    {
	      GError *gerr = NULL;
	      g_hash_table_insert(self->clients, GINT_TO_POINTER(new_fd), (gpointer)client);
	      g_thread_pool_push(threadpool, client, &gerr);
	      if(gerr)
		{
		  whiteboard_log_debug("Could not add client thread:%s\n", gerr->message);
		  g_error_free(gerr);
		}
	    }
	  else
	    {
	      whiteboard_log_debug("cannot insert new client w/ fd:%d\n", new_fd);
	      close(new_fd);
	      new_fd = -1;
	    }
	  g_mutex_unlock(self->client_lock);
	}
      
    }
  whiteboard_log_debug_fe();
  return NULL;
}

void client_handler_thread(gpointer data, gpointer user_data)
{
  Client *client = (Client *) data;
  whiteboard_log_debug_fb();
  whiteboard_log_debug("Listener %p, client %p\n", listener, client);
  if(listener && client)
    {
      while(1)
	{
	  int ret = 0;
	  NodeMsgContent_t *msg = parseSSAPmsg_new();
	  ret = client_receive_message(client,msg);
	  if( ret > 0)
	    {
	      listener_handle_request(listener, client, msg);
	      parseSSAPmsg_free( &msg);
	    }
	  else
	    {
	      parseSSAPmsg_free( &msg);
	      break;
	    }
	}
    }
  g_mutex_lock(listener->client_lock);
  g_hash_table_remove(listener->clients, GINT_TO_POINTER(client->fd));
  g_mutex_unlock(listener->client_lock);

  whiteboard_log_debug_fe();

  return;
}

Client *client_new(int fd)
{
  Client *self = g_try_new0(Client, 1);
  if(!self)
    {
      whiteboard_log_debug("client_new(): memory alloc error\n");
    }
  else
    {
      self->recvbuf = g_try_new0(guchar, RECVBUFSIZE );
      if(!self->recvbuf)
	{
	  whiteboard_log_debug("client_new(): memory alloc error for recvbuf\n");
	  g_free(self);
	  self = NULL;
	}
      else
	{
	  self->fd = fd;
	  self->len = 0;
	  self->remaining_len = 0;
	}
    }
  return self;
}

void client_destroy(Client *self)
{
  if(self->fd>0)
    {
      close(self->fd);
    }

  g_free(self->recvbuf);
  g_free(self);
}

gint client_receive_message( Client *client, NodeMsgContent_t *msg)
{
  gint rtmp;
  gint bytes_handled = 0;
  gint bytecountnew = 0;
  ssStatus_t status;
  gint index = 0;
  gboolean finished = FALSE;

  while(!finished)
    {
      if( client->remaining_len )
	{
	  index = client->len - client->remaining_len;
	  //bytecountold = parseM3msg_parsedbytecount(msg);
	  // before first parsed section, parser responds with -1
	  //if(bytecountold < 0) 
	  //  bytecountold = 0;
	  
	  bytes_handled += client->remaining_len;
	  whiteboard_log_debug("Parsing section; index:%d, remaining_len: %d, bytes_handled: %d\n",
			       index, client->remaining_len, bytes_handled);
	  
	  status = parseSSAPmsg_section (msg,
					 (gchar *)(client->recvbuf + index),
					 client->remaining_len,
					 0);
	  
	  if(status == ss_StatusOK)
	    {
	      gint not_handled;
	      bytecountnew = parseSSAPmsg_parsedbytecount(msg);
	      not_handled = bytes_handled - bytecountnew;
	      whiteboard_log_debug("Parse finished successfully, bytecount (%d), not_handled (%d)\n", bytecountnew, not_handled);
	      client->remaining_len = not_handled;
	      bytes_handled = bytecountnew;
	      finished = TRUE;
	    }
	  else if(status == ss_ParsingInProgress)
	    {
	      client->len = 0;
	      client->remaining_len = 0;
	    }
	  else
	    {
	      whiteboard_log_debug("Parse error %d\n", status);
	      finished = TRUE;
	      bytes_handled = -1;
	    }
	}
      
      if (!finished)
	{
	  rtmp = recv( client->fd, client->recvbuf, RECVBUFSIZE , 0);
	  if(rtmp < 0)
	    {
	      whiteboard_log_debug("receive error\n");
	      finished = TRUE;
	      bytes_handled = -1;
	    }
	  else if (rtmp > 0)
	    {
	      gchar *debug_txt = g_strndup( (gchar *)client->recvbuf, rtmp);
	      whiteboard_log_debug("Received message (%d bytes): %s\n", rtmp, debug_txt);
	      g_free(debug_txt);
	      client->len = rtmp;
	      client->remaining_len = rtmp;
	      
	    }
	  else
	    {
	      whiteboard_log_debug("received zero bytes\n");
	      finished = TRUE;
	      bytes_handled = 0;
	    }
	}
    }
  return bytes_handled;
}

static void listener_handle_request(Listener *listener, Client *client, NodeMsgContent_t *msg)
{
  whiteboard_log_debug_fb();

  switch( parseSSAPmsg_get_name(msg))
    {
    case MSG_N_NSET:
      whiteboard_log_debug("message name not set...\n");
      break;
    case MSG_N_JOIN:
      listener_handle_join(listener, client, msg);
      break;
    case MSG_N_LEAVE:
      listener_handle_leave(listener, client, msg);
      break;
    case MSG_N_INSERT:
      listener_handle_insert(listener, client, msg);
      break;
    case MSG_N_UPDATE:
      listener_handle_update(listener, client, msg);
      break;
    case MSG_N_REMOVE:
      listener_handle_remove(listener, client, msg);
      break;
    case MSG_N_QUERY:
      listener_handle_query(listener, client, msg);
      break;
    case MSG_N_SUBSCRIBE:
      listener_handle_subscribe(listener, client, msg);
      break;
    case MSG_N_UNSUBSCRIBE:
      listener_handle_unsubscribe(listener, client, msg);
      break;

    default:
      
      whiteboard_log_debug("Unknown request: %d\n", parseSSAPmsg_get_name(msg));
      break;
    }

  whiteboard_log_debug_fe();

}

int listener_handle_join(Listener *listener, Client *client, NodeMsgContent_t *msg)
{
  ssStatus_t joinStatus;
  Waiter *waiter = NULL;
  ssBufDesc_t *bd = NULL;
  
  whiteboard_log_debug_fb();
  whiteboard_log_debug("Got Join request\n");
  waiter = waiter_new(client->fd);
  if(waiter)
    {
      if( listener_add_request(listener, (guchar *)parseSSAPmsg_get_nodeid( msg), parseSSAPmsg_get_msgnumber( msg), waiter ) < 0)
	{
	  whiteboard_log_debug("Could  not add new request w/ msgid : %d\n", parseSSAPmsg_get_msgnumber( msg));
	  waiter_destroy(waiter);
	}
      else
	{
	  const gchar *credentials = parseSSAPmsg_get_credentials(msg);
	  if(!credentials)
	    credentials = "";
	  whiteboard_log_debug("Sending join_req w/ key: %s\n", waiter->key);
	  if( sib_object_send_join( listener->object,  
				    (guchar *)parseSSAPmsg_get_spaceid(msg), 
				    (guchar *)parseSSAPmsg_get_nodeid(msg),
				    parseSSAPmsg_get_msgnumber(msg),
				    credentials ) <0)
	    {
	      whiteboard_log_debug("Could not send join request w/ msgid : %d\n", parseSSAPmsg_get_msgnumber( msg));
	      listener_remove_request(listener, (guchar *)parseSSAPmsg_get_nodeid( msg), parseSSAPmsg_get_msgnumber( msg));
	      joinStatus = ss_OperationFailed;
	      bd = ssBufDesc_new();
	      if( ss_StatusOK != ssBufDesc_CreateJoinResponse(bd, 
							      (guchar *)parseSSAPmsg_get_nodeid( msg ),
							      (guchar *)parseSSAPmsg_get_spaceid( msg ),
							      parseSSAPmsg_get_msgnumber( msg ),
							      joinStatus ) )
		{
		  ssBufDesc_free(&bd);
		}
	    }
	  else
	    {
	      
	      waiter_wait_for_finished(waiter);
	      if(waiter->rsp)
		{
		  bd = ssBufDesc_new();
		  if( ss_StatusOK != ssBufDesc_CreateJoinResponse(bd, 
								  (guchar *)parseSSAPmsg_get_nodeid(waiter->rsp),
								  (guchar *)parseSSAPmsg_get_spaceid(waiter->rsp),
								  parseSSAPmsg_get_msgnumber(waiter->rsp),
								  parseSSAPmsg_get_msg_status(waiter->rsp) ) )
		    {
		      ssBufDesc_free(&bd);
		    }
		}
	    }
	}
    }
  if(bd && waiter)
    {
      if(client_send_message(waiter->fd, 
			     (guchar *)ssBufDesc_GetMessage(bd), 
			     ssBufDesc_GetMessageLen(bd)) < 0 )
	{
	  whiteboard_log_debug("Send failed\n");
	}
      listener_remove_request(listener, (guchar *)parseSSAPmsg_get_nodeid( msg), parseSSAPmsg_get_msgnumber(msg));
      ssBufDesc_free(&bd);
    }

  whiteboard_log_debug_fe();
  return 0;
}

static int listener_handle_leave(Listener *listener, Client *client, NodeMsgContent_t *msg)
{
  ssStatus_t status;
  Waiter *waiter = NULL;
  ssBufDesc_t *bd = NULL;
  whiteboard_log_debug_fb();
  whiteboard_log_debug("Got Leave request\n");
  waiter = waiter_new(client->fd);
  if(waiter)
    {
      if( listener_add_request(listener, (guchar *)parseSSAPmsg_get_nodeid( msg), parseSSAPmsg_get_msgnumber( msg), waiter ) < 0)
	{
	  whiteboard_log_debug("Could not add new request w/ msgid : %d\n", parseSSAPmsg_get_msgnumber( msg));
	  waiter_destroy(waiter);
	}
      else
	{
	  whiteboard_log_debug("Sending leave_req w/ key: %s\n", waiter->key);
	  if( sib_object_send_leave( listener->object,  
				     (guchar *)parseSSAPmsg_get_spaceid(msg), 
				     (guchar *)parseSSAPmsg_get_nodeid(msg),
				     parseSSAPmsg_get_msgnumber(msg)) <0)
	    {
	      whiteboard_log_debug("Could not send leave request w/ msgid : %d\n", parseSSAPmsg_get_msgnumber( msg));
	      listener_remove_request(listener, (guchar *)parseSSAPmsg_get_nodeid( msg), parseSSAPmsg_get_msgnumber( msg));
	      status = ss_OperationFailed;
	      bd = ssBufDesc_new();
	      if( ss_StatusOK != ssBufDesc_CreateLeaveResponse(bd, 
							       (guchar *)parseSSAPmsg_get_nodeid(msg),
							       (guchar *)parseSSAPmsg_get_spaceid(msg),
							       parseSSAPmsg_get_msgnumber(msg),
							       status))
		{
		  ssBufDesc_free(&bd);
		}
	    }
	  else
	    {
	      waiter_wait_for_finished(waiter);
	      if(waiter->rsp)
		{
		  bd = ssBufDesc_new();
		  if( ss_StatusOK != ssBufDesc_CreateLeaveResponse(bd, 
								   (guchar *)parseSSAPmsg_get_nodeid(waiter->rsp),
								   (guchar *)parseSSAPmsg_get_spaceid(waiter->rsp),
								   parseSSAPmsg_get_msgnumber(waiter->rsp),
								   parseSSAPmsg_get_msg_status(waiter->rsp)) )
		    {
		      ssBufDesc_free(&bd);
		    }
		}
	    }
	}
    }
  if(bd && waiter)
    {
      if(client_send_message(waiter->fd, 
			     (guchar *)ssBufDesc_GetMessage(bd), 
			     ssBufDesc_GetMessageLen(bd)) < 0 )
	{
	  whiteboard_log_debug("Send failed\n");
	}
      listener_remove_request(listener, (guchar *)parseSSAPmsg_get_nodeid( msg), parseSSAPmsg_get_msgnumber(msg));
      ssBufDesc_free(&bd);
    }
  return 0;
  whiteboard_log_debug_fe();
}

static int listener_handle_insert(Listener *listener, Client *client, NodeMsgContent_t *msg)
{
  ssStatus_t status;
  Waiter *waiter = NULL;
  ssBufDesc_t *bd = NULL;
  whiteboard_log_debug("Got Insert request\n");
  waiter = waiter_new(client->fd);
  if(waiter)
    {
      if( listener_add_request(listener, (guchar *)parseSSAPmsg_get_nodeid( msg), parseSSAPmsg_get_msgnumber( msg), waiter ) < 0)
	{
	  whiteboard_log_debug("Could not add new request w/ msgid : %d\n", parseSSAPmsg_get_msgnumber( msg));
	  waiter_destroy(waiter);
	}
      else
	{
	  //EncodingType encoding = ( parseSSAPmsg_get_graphStyle(msg) == MSG_G_TMPL ? EncodingM3XML : EncodingInvalid);
	  EncodingType encoding;
	  if      (parseSSAPmsg_get_graphStyle(msg) == MSG_G_TMPL )
	  {	encoding = EncodingM3XML;
	  }
	  else if (parseSSAPmsg_get_graphStyle(msg) == MSG_G_RDF )
	  {	encoding = EncodingRDFXML;
	  }
	  else 
	  {	encoding =EncodingInvalid;
	  }



	  whiteboard_log_debug("Sending insert_req w/ key: %s, encoding: %d\n", waiter->key, encoding);
	  if(  (encoding == EncodingInvalid) || 
	      ( sib_object_send_insert( listener->object,  
					(guchar *) parseSSAPmsg_get_spaceid(msg), 
					(guchar *)parseSSAPmsg_get_nodeid(msg),
				        parseSSAPmsg_get_msgnumber(msg),
				        encoding,
					(guchar *)parseSSAPmsg_get_insert_graph(msg)) <0))
	    {
	      whiteboard_log_debug("Could not send insert request w/ msgid : %d\n", parseSSAPmsg_get_msgnumber( msg));
	      listener_remove_request(listener, (guchar *)parseSSAPmsg_get_nodeid( msg), parseSSAPmsg_get_msgnumber( msg));
	      if (encoding == EncodingInvalid) 
		status = ss_SIBFailNotImpl;

	      bd = ssBufDesc_new();
	      if( ss_StatusOK != ssBufDesc_CreateInsertResponse(bd, 
								(guchar *)parseSSAPmsg_get_nodeid(msg),
								(guchar *)parseSSAPmsg_get_spaceid(msg),
								parseSSAPmsg_get_msgnumber(msg),
								status,
								NULL))
		{
		  ssBufDesc_free(&bd);
		}
	    }
	  else
	    {
	      waiter_wait_for_finished(waiter);
	      if(waiter->rsp)
		{
		  bd = ssBufDesc_new();
		  if( ss_StatusOK != ssBufDesc_CreateInsertResponse(bd, 
								   (guchar *) parseSSAPmsg_get_nodeid(waiter->rsp),
								    (guchar *)parseSSAPmsg_get_spaceid(waiter->rsp),
								    parseSSAPmsg_get_msgnumber(waiter->rsp),
								    parseSSAPmsg_get_msg_status(waiter->rsp),
								    (guchar *)parseSSAPmsg_get_insert_graph(waiter->rsp)) )
		    {
		      ssBufDesc_free(&bd);
		    }
		}
	    }
	}
    }
  if(bd && waiter)
    {
      if(client_send_message(waiter->fd, 
			    (guchar *) ssBufDesc_GetMessage(bd), 
			     ssBufDesc_GetMessageLen(bd)) < 0 )
	{
	  whiteboard_log_debug("Send failed\n");
	}
      listener_remove_request(listener,(guchar *) parseSSAPmsg_get_nodeid( msg), parseSSAPmsg_get_msgnumber(msg));
      ssBufDesc_free(&bd);
    }

  whiteboard_log_debug_fe();
  return 0;
}
static int listener_handle_remove(Listener *listener, Client *client, NodeMsgContent_t *msg)
{
  ssStatus_t status;
  Waiter *waiter = NULL;
  ssBufDesc_t *bd = NULL;
  whiteboard_log_debug("Got Remove request\n");
  waiter = waiter_new(client->fd);
  if(waiter)
    {
      if( listener_add_request(listener, (guchar *)parseSSAPmsg_get_nodeid( msg), parseSSAPmsg_get_msgnumber( msg), waiter ) < 0)
	{
	  whiteboard_log_debug("Could not add new request w/ msgid : %d\n", parseSSAPmsg_get_msgnumber( msg));
	  waiter_destroy(waiter);
	}
      else
	{
	  //EncodingType encoding = ( parseSSAPmsg_get_graphStyle(msg) == MSG_G_TMPL ? EncodingM3XML : EncodingInvalid);

	  EncodingType encoding;
	  if  (parseSSAPmsg_get_graphStyle(msg) == MSG_G_TMPL )
	  {
		encoding = EncodingM3XML;
	  }
	  else if (parseSSAPmsg_get_graphStyle(msg) == MSG_G_RDF )
	  {
		encoding = EncodingRDFXML;
	  }
	  else 
	  {
		encoding =EncodingInvalid;
	  }

	  whiteboard_log_debug("Sending remove_req w/ key: %s, encoding: %d\n", waiter->key, encoding);
	  if(  (encoding == EncodingInvalid) || 
	      ( sib_object_send_remove( listener->object,  
					(guchar *)parseSSAPmsg_get_spaceid(msg), 
					(guchar *)parseSSAPmsg_get_nodeid(msg),
					parseSSAPmsg_get_msgnumber(msg),
					encoding,
					(guchar *) parseSSAPmsg_get_remove_graph(msg) ) <0))
	    {
	      whiteboard_log_error("Could not send remove request w/ msgid : %d\n", parseSSAPmsg_get_msgnumber( msg));
	      listener_remove_request(listener, (guchar *)parseSSAPmsg_get_nodeid( msg), parseSSAPmsg_get_msgnumber( msg));
	      

	      if( encoding == EncodingInvalid)
		status = ss_SIBFailNotImpl;
	      //else
		//status = ss_OperationFailed;

	      bd = ssBufDesc_new();
	      if( ss_StatusOK != ssBufDesc_CreateRemoveResponse(bd, 
								(guchar *)parseSSAPmsg_get_nodeid(msg),
								(guchar *)parseSSAPmsg_get_spaceid(msg),
								parseSSAPmsg_get_msgnumber(msg),
								status))
		{
		  ssBufDesc_free(&bd);
		}
	    }
	  else
	    {
	      waiter_wait_for_finished(waiter);
	      if(waiter->rsp)
		{
		  bd = ssBufDesc_new();
		  if( ss_StatusOK != ssBufDesc_CreateRemoveResponse(bd, 
								    (guchar *)parseSSAPmsg_get_nodeid(waiter->rsp),
								    (guchar *)parseSSAPmsg_get_spaceid(waiter->rsp),
								    parseSSAPmsg_get_msgnumber(waiter->rsp),
								    parseSSAPmsg_get_msg_status(waiter->rsp) ) )
		    {
		      ssBufDesc_free(&bd);
		    }
		}
	    }
	}
    }
  if(bd && waiter)
    {
      if(client_send_message(waiter->fd, 
			     (guchar *)ssBufDesc_GetMessage(bd), 
			     ssBufDesc_GetMessageLen(bd)) < 0 )
	{
	  whiteboard_log_debug("Send failed\n");
	}
      listener_remove_request(listener, (guchar *)parseSSAPmsg_get_nodeid( msg), parseSSAPmsg_get_msgnumber(msg));
      ssBufDesc_free(&bd);
    }

  whiteboard_log_debug_fe();
  return 0;
}
static int listener_handle_update(Listener *listener, Client *client, NodeMsgContent_t *msg)
{
  ssStatus_t status;
  Waiter *waiter = NULL;
  ssBufDesc_t *bd = NULL;
  whiteboard_log_debug("Got update request\n");
  waiter = waiter_new(client->fd);
  if(waiter)
    {
      if( listener_add_request(listener, (guchar *)parseSSAPmsg_get_nodeid( msg), parseSSAPmsg_get_msgnumber( msg), waiter ) < 0)
	{
	  whiteboard_log_debug("Could not add new request w/ msgid : %d\n", parseSSAPmsg_get_msgnumber( msg));
	  waiter_destroy(waiter);
	}
      else
	{
	  //EncodingType encoding = ( parseSSAPmsg_get_graphStyle(msg) == MSG_G_TMPL ? EncodingM3XML : EncodingInvalid);
	  EncodingType encoding;
	  whiteboard_log_debug("Sending update_req w/ key: %s \n", waiter->key);


	  if  (parseSSAPmsg_get_graphStyle(msg) == MSG_G_TMPL )
	  {
		encoding = EncodingM3XML;
	  }
	  else if (parseSSAPmsg_get_graphStyle(msg) == MSG_G_RDF )
	  {
		encoding = EncodingRDFXML;
	  }
	  else 
	  {
		encoding =EncodingInvalid;
	  }


	  if( (encoding == EncodingInvalid) || 
	      ( sib_object_send_update( listener->object,  
					(guchar *)parseSSAPmsg_get_spaceid(msg), 
					(guchar *)parseSSAPmsg_get_nodeid(msg),
					parseSSAPmsg_get_msgnumber(msg),
					encoding,
					(guchar *)parseSSAPmsg_get_insert_graph(msg),
					(guchar *)parseSSAPmsg_get_remove_graph(msg)) ) <0)
	    {
	      whiteboard_log_debug("Could not send update request w/ msgid : %d\n", parseSSAPmsg_get_msgnumber( msg));
	      listener_remove_request(listener, (guchar *)parseSSAPmsg_get_nodeid( msg), parseSSAPmsg_get_msgnumber( msg));

	      if( encoding == EncodingInvalid)
		status = ss_SIBFailNotImpl;
	      //else
		//status = ss_OperationFailed;

	      bd = ssBufDesc_new();
	      if( ss_StatusOK != ssBufDesc_CreateUpdateResponse(bd, 
								(guchar *)parseSSAPmsg_get_nodeid(msg),
								(guchar *)parseSSAPmsg_get_spaceid(msg),
								parseSSAPmsg_get_msgnumber(msg),
								status,
								NULL))
		{
		  ssBufDesc_free(&bd);
		}
	    }
	  else
	    {
	      waiter_wait_for_finished(waiter);
	      if(waiter->rsp)
		{
		  bd = ssBufDesc_new();
		  if( ss_StatusOK != ssBufDesc_CreateUpdateResponse(bd, 
								  (guchar *)parseSSAPmsg_get_nodeid(waiter->rsp),
								  (guchar *)parseSSAPmsg_get_spaceid(waiter->rsp),
								    parseSSAPmsg_get_msgnumber(waiter->rsp),
								    parseSSAPmsg_get_msg_status(waiter->rsp),
								    (guchar *)parseSSAPmsg_get_M3XML(waiter->rsp)) )
		    {
		      ssBufDesc_free(&bd);
		    }
		}
	    }
	}
    }
  if(bd && waiter)
    {
      if(client_send_message(waiter->fd, 
			     (guchar *)ssBufDesc_GetMessage(bd), 
			     ssBufDesc_GetMessageLen(bd)) < 0 )
	{
	  whiteboard_log_debug("Send failed\n");
	}
      listener_remove_request(listener,(guchar *) parseSSAPmsg_get_nodeid( msg), parseSSAPmsg_get_msgnumber(msg));
      ssBufDesc_free(&bd);
    }

  whiteboard_log_debug_fe();
  return 0;
}

static int listener_handle_query(Listener *listener, Client *client, NodeMsgContent_t *msg)
{
  ssStatus_t status = ss_StatusOK;
  Waiter *waiter = NULL;
  ssBufDesc_t *bd = NULL;
  whiteboard_log_debug("Got query request\n");
  waiter = waiter_new(client->fd);
  if(waiter)
    {
      if( listener_add_request(listener, (guchar *)parseSSAPmsg_get_nodeid( msg), parseSSAPmsg_get_msgnumber( msg), waiter ) < 0)
	{
	  whiteboard_log_debug("Could not add new request w/ msgid : %d\n", parseSSAPmsg_get_msgnumber( msg));
	  waiter_destroy(waiter);
	}
      else
	{
	  QueryType qtype;
	  switch(parseSSAPmsg_get_queryStyle( msg) )
	    {
	    case   MSG_Q_NSET:
	    case   MSG_Q_N3: 
	      qtype = QueryTypeInvalid;
	      break;
	    case   MSG_Q_TMPL: 
	      qtype=QueryTypeTemplate;
		break;
	    case   MSG_Q_SPRQL:
	      qtype=QueryTypeSPARQLSelect;
		break;
	    case   MSG_Q_WQL_VALUES:
	      qtype=QueryTypeWQLValues;
		break;
	    case   MSG_Q_WQL_NODETYPES:
	      qtype=QueryTypeWQLNodeTypes;
		break;
	    case   MSG_Q_WQL_RELATED:
	      qtype=QueryTypeWQLRelated;
		break;
	    case   MSG_Q_WQL_ISTYPE:
	      qtype=QueryTypeWQLIsType;
		break;
	    case   MSG_Q_WQL_ISSUBTYPE:
	      qtype=QueryTypeWQLIsSubType;
		break;
	    default:
	      qtype = QueryTypeInvalid;
	      break;
	    }
	  whiteboard_log_debug("Sending query_req w/ key: %s, qtype: %d\n", waiter->key, qtype);
	  if( (qtype == QueryTypeInvalid) || 
	      ( sib_object_send_query( listener->object,  
						    (guchar *)parseSSAPmsg_get_spaceid(msg), 
						    (guchar *)parseSSAPmsg_get_nodeid(msg),
						    parseSSAPmsg_get_msgnumber(msg),
						    qtype,
						    (guchar *)parseSSAPmsg_get_M3XML(msg) )  <0) )
	    {
	      whiteboard_log_debug("Could send query request w/ msgid : %d\n", parseSSAPmsg_get_msgnumber( msg));
	      listener_remove_request(listener, (guchar *)parseSSAPmsg_get_nodeid( msg), parseSSAPmsg_get_msgnumber( msg));
	      if( qtype == QueryTypeInvalid)
		status = ss_SIBFailNotImpl;
	      else
		status = ss_OperationFailed;

	      bd = ssBufDesc_new();
	      if( ss_StatusOK != ssBufDesc_CreateQueryResponse(bd, 
								(guchar *)parseSSAPmsg_get_nodeid(msg),
								(guchar *)parseSSAPmsg_get_spaceid(msg),
								parseSSAPmsg_get_msgnumber(msg),
								status,
								NULL))
		{
		  ssBufDesc_free(&bd);
		}
	    }
	  else
	    {
	      waiter_wait_for_finished(waiter);
	      if(waiter->rsp)
		{
		  bd = ssBufDesc_new();
		  if( ss_StatusOK != ssBufDesc_CreateQueryResponse(bd, 
								  (guchar *)parseSSAPmsg_get_nodeid(waiter->rsp),
								  (guchar *)parseSSAPmsg_get_spaceid(waiter->rsp),
								  parseSSAPmsg_get_msgnumber(waiter->rsp),
								   parseSSAPmsg_get_msg_status(waiter->rsp),
								   (guchar *)parseSSAPmsg_get_M3XML(waiter->rsp)) )
		    {
		      ssBufDesc_free(&bd);
		    }
		}
	    }
	}
    }
  if(bd && waiter)
    {
      if(client_send_message(waiter->fd, 
			     (guchar *)ssBufDesc_GetMessage(bd), 
			     ssBufDesc_GetMessageLen(bd)) < 0 )
	{
	  whiteboard_log_debug("Send failed\n");
	}
      listener_remove_request(listener, (guchar *)parseSSAPmsg_get_nodeid( msg), parseSSAPmsg_get_msgnumber(msg));
      ssBufDesc_free(&bd);
    }

  whiteboard_log_debug_fe();
  return 0;
}
static int listener_handle_subscribe(Listener *listener, Client *client, NodeMsgContent_t *msg)
{

  ssStatus_t status = ss_StatusOK;
  Waiter *waiter = NULL;
  Waiter *subwaiter = NULL;
  ssBufDesc_t *bd = NULL;
  gboolean subscription_finished = FALSE;
  whiteboard_log_debug("Got subscribe request\n");
  waiter = waiter_new(client->fd);

  int optval=0;
  int optlen;
  int errorsock;
  
  gint n;

  char* nodeid=	NULL;
  char* spaceid= NULL;
  char* subid=	NULL;

  if(waiter)
    {
      if( listener_add_request(listener, (guchar *)parseSSAPmsg_get_nodeid( msg), parseSSAPmsg_get_msgnumber( msg), waiter ) < 0)
	{
	  whiteboard_log_debug("Could not add new request w/ msgid : %d\n", parseSSAPmsg_get_msgnumber( msg));
	  waiter_destroy(waiter);
	}
      else
	{
	  QueryType qtype;
	  switch(parseSSAPmsg_get_queryStyle( msg) )
	    {
	    case   MSG_Q_NSET:
	    case   MSG_Q_N3: 
	      qtype = QueryTypeInvalid;
	      break;
	    case   MSG_Q_TMPL: 
	      qtype=QueryTypeTemplate;
		break;
	    case   MSG_Q_SPRQL:
	      qtype=QueryTypeSPARQLSelect;
		break;
	    case   MSG_Q_WQL_VALUES:
	      qtype=QueryTypeWQLValues;
		break;
	    case   MSG_Q_WQL_NODETYPES:
	      qtype=QueryTypeWQLNodeTypes;
		break;
	    case   MSG_Q_WQL_RELATED:
	      qtype=QueryTypeWQLRelated;
		break;
	    case   MSG_Q_WQL_ISTYPE:
	      qtype=QueryTypeWQLIsType;
		break;
	    case   MSG_Q_WQL_ISSUBTYPE:
	      qtype=QueryTypeWQLIsSubType;
		break;
	    default: 
	      qtype = QueryTypeInvalid;
	      break;
	    }
	  whiteboard_log_debug("Sending subscribe_req w/ key: %s, qtype: %d\n", waiter->key, qtype);
	  if( (qtype == QueryTypeInvalid) || 
	      ( sib_object_send_subscribe( listener->object,  
					   (guchar *)parseSSAPmsg_get_spaceid(msg), 
					   (guchar *)parseSSAPmsg_get_nodeid(msg),
					   parseSSAPmsg_get_msgnumber(msg),
					   qtype,
					   (guchar *)parseSSAPmsg_get_M3XML(msg) )  <0) )
	    {
	      whiteboard_log_debug("Could not send subscribe request w/ msgid : %d\n", parseSSAPmsg_get_msgnumber( msg));
	      listener_remove_request(listener, (guchar *)parseSSAPmsg_get_nodeid( msg), parseSSAPmsg_get_msgnumber( msg));
	      if( qtype == QueryTypeInvalid)
		status = ss_SIBFailNotImpl;
	      else
		status = ss_OperationFailed;

	      bd = ssBufDesc_new();
	      if( ss_StatusOK != ssBufDesc_CreateSubscribeResponse(bd, 
								   (guchar *)parseSSAPmsg_get_nodeid(msg),
								   (guchar *)parseSSAPmsg_get_spaceid(msg),
								   parseSSAPmsg_get_msgnumber(msg),
								   status,
								   NULL,
								   NULL))
		{
		  ssBufDesc_free(&bd);
		}
	      subscription_finished = TRUE;
	    }
	  else
	    {
	      waiter_wait_for_finished(waiter);
	      if(waiter->rsp)
		{
		  subwaiter = waiter_new(client->fd);
		  bd = ssBufDesc_new();
		  if( listener_add_subscription(listener,  (guchar *)parseSSAPmsg_get_subscriptionid(waiter->rsp),subwaiter) < 0)
		    {
		      
		      whiteboard_log_debug("Could not add new subscription w/ id : %s\n", parseSSAPmsg_get_subscriptionid(waiter->rsp));		      
		      waiter_destroy(subwaiter);
		      if( ss_StatusOK != ssBufDesc_CreateSubscribeResponse(bd, 
									   (guchar *)parseSSAPmsg_get_nodeid(msg),
									   (guchar *)parseSSAPmsg_get_spaceid(msg),
									   parseSSAPmsg_get_msgnumber(msg),
									   status,
									   NULL,
									   NULL))
			{
			  ssBufDesc_free(&bd);
			}
		    }		  
		  else if( ss_StatusOK != ssBufDesc_CreateSubscribeResponse(bd, 
									    (guchar *)parseSSAPmsg_get_nodeid(waiter->rsp),
									    (guchar *)parseSSAPmsg_get_spaceid(waiter->rsp),
									    parseSSAPmsg_get_msgnumber(waiter->rsp),
									    parseSSAPmsg_get_msg_status(waiter->rsp),
									    (guchar *)parseSSAPmsg_get_subscriptionid(waiter->rsp),
									    (guchar *)parseSSAPmsg_get_M3XML(waiter->rsp)) )
		    {
		      ssBufDesc_free(&bd);
		    }

		  //////////FIXED PARAMETERS//////////
		  nodeid = malloc(strlen(parseSSAPmsg_get_nodeid(waiter->rsp))+1);
		  strncpy(nodeid, parseSSAPmsg_get_nodeid(waiter->rsp),strlen(parseSSAPmsg_get_nodeid(waiter->rsp)) );
		  nodeid[strlen(parseSSAPmsg_get_nodeid(waiter->rsp))] = '\0';

		  spaceid = malloc(strlen(parseSSAPmsg_get_spaceid(waiter->rsp))+1);
		  strcpy(spaceid, parseSSAPmsg_get_spaceid(waiter->rsp) );
		  spaceid[strlen(parseSSAPmsg_get_spaceid(waiter->rsp))] = '\0';

		  subid = malloc(strlen(parseSSAPmsg_get_subscriptionid(waiter->rsp))+1);
		  strncpy(subid, parseSSAPmsg_get_subscriptionid(waiter->rsp),strlen(parseSSAPmsg_get_subscriptionid(waiter->rsp)) );
		  subid[strlen(parseSSAPmsg_get_subscriptionid(waiter->rsp))] = '\0';

		  ////////////////////////////////////
		
		}
	    }
	}
    }
  if(bd && waiter && waiter->rsp)
    {
      errorsock=getsockopt(subwaiter->fd, SOL_SOCKET, SO_ERROR, &optval, &optlen);  
      if (optval!=0)
      		{
		  whiteboard_log_debug("Send failed\n");
		  subscription_finished = TRUE;
     		}	
      else if( client_send_message(waiter->fd, 
				      (guchar *)ssBufDesc_GetMessage(bd), 
				      ssBufDesc_GetMessageLen(bd)) < 0 )
		{
		  whiteboard_log_debug("Send failed\n");
		  subscription_finished = TRUE;
		}

      listener_remove_request(listener, (guchar *)parseSSAPmsg_get_nodeid( msg), parseSSAPmsg_get_msgnumber(msg));
      ssBufDesc_free(&bd);


      while ( !subscription_finished  )
	{
	  whiteboard_log_debug("Waiting subscription_ind\n");
	  waiter_wait_for_finished( subwaiter );
	  whiteboard_log_debug("got something \n");


	  bd = ssBufDesc_new();
	  if(subwaiter->rsp)
	    {
	      if( ( parseSSAPmsg_get_name(subwaiter->rsp) == MSG_N_SUBSCRIBE)  &&
		  ( parseSSAPmsg_get_type(subwaiter->rsp) == MSG_T_IND)  && 
		  ( ss_StatusOK == ssBufDesc_CreateSubscriptionIndMessage(bd, 
									  (guchar *)parseSSAPmsg_get_nodeid(subwaiter->rsp),
									  (guchar *)parseSSAPmsg_get_spaceid(subwaiter->rsp),
									  parseSSAPmsg_get_msgnumber(subwaiter->rsp),
									  parseSSAPmsg_get_update_sequence (subwaiter->rsp),
									  (guchar *)parseSSAPmsg_get_subscriptionid(subwaiter->rsp),
								     (guchar *)parseSSAPmsg_get_results_added(subwaiter->rsp), 
								     (guchar *)parseSSAPmsg_get_results_removed(subwaiter->rsp) ) ))
		{
		  whiteboard_log_debug("Got Subscription ind, id: %s\n",parseSSAPmsg_get_subscriptionid(subwaiter->rsp));

		  errorsock=getsockopt(subwaiter->fd, SOL_SOCKET, SO_ERROR, &optval, &optlen);  
		  if (optval!=0)
		    {
		      whiteboard_log_debug("Send failed by optval\n");

		      ///////////////////AUTO UNSUB////////////////////
		      if( sib_object_send_unsubscribe(listener->object,  
							   (guchar *)parseSSAPmsg_get_spaceid(subwaiter->rsp), 
							   (guchar *)parseSSAPmsg_get_nodeid(subwaiter->rsp),
							   parseSSAPmsg_get_msgnumber(subwaiter->rsp),
							   (guchar *)parseSSAPmsg_get_subscriptionid(subwaiter->rsp)) <0)
			 {
			    whiteboard_log_debug("Could not send unsubscribe request w/ msgid : %d\n", parseSSAPmsg_get_msgnumber(msg ));
			 }
		      ///////////////////AUTO UNSUB////////////////////

		      listener_remove_subscription(listener, (guchar *)parseSSAPmsg_get_subscriptionid(subwaiter->rsp));
		      subscription_finished = TRUE;
		    }
		  else if( client_send_message(subwaiter->fd, 
					  (guchar *)ssBufDesc_GetMessage(bd), 
					  ssBufDesc_GetMessageLen(bd)) < 0 )
		    {
		      whiteboard_log_debug("Send failed\n");

		      ///////////////////AUTO UNSUB////////////////////
		      if( sib_object_send_unsubscribe(listener->object,  
							   (guchar *)parseSSAPmsg_get_spaceid(subwaiter->rsp), 
							   (guchar *)parseSSAPmsg_get_nodeid(subwaiter->rsp),
							   parseSSAPmsg_get_msgnumber(subwaiter->rsp),
							   (guchar *)parseSSAPmsg_get_subscriptionid(subwaiter->rsp)) <0)
			 {
			    whiteboard_log_debug("Could not send unsubscribe request w/ msgid : %d\n", parseSSAPmsg_get_msgnumber(msg ));
			 }

		      ///////////////////AUTO UNSUB////////////////////
		      listener_remove_subscription(listener, (guchar *)parseSSAPmsg_get_subscriptionid(subwaiter->rsp));
		      subscription_finished = TRUE;

		    }
		  else
		    {
		      nodemsgcontent_free( &(subwaiter->rsp) );
		    }
		    
		}
	      else if ( (parseSSAPmsg_get_name(subwaiter->rsp) == MSG_N_UNSUBSCRIBE)  &&
			( parseSSAPmsg_get_type(subwaiter->rsp) == MSG_T_CNF)  && 
			( ss_StatusOK == ssBufDesc_CreateUnsubscribeResponse(bd, 
									     (guchar *)parseSSAPmsg_get_nodeid(subwaiter->rsp),
									     (guchar *)parseSSAPmsg_get_spaceid(subwaiter->rsp),
									     parseSSAPmsg_get_msgnumber(subwaiter->rsp),
									     parseSSAPmsg_get_msg_status(subwaiter->rsp),
									     (guchar *)parseSSAPmsg_get_subscriptionid(subwaiter->rsp) )))
		{
		  whiteboard_log_debug("Got Unsubscribe cnf, id %s\n",parseSSAPmsg_get_subscriptionid(subwaiter->rsp));

		  errorsock=getsockopt(subwaiter->fd, SOL_SOCKET, SO_ERROR, &optval, &optlen);  
		  if (optval!=0)
		    {
			 whiteboard_log_debug("Send failed by optval\n");	
		    }
		  else if( client_send_message(subwaiter->fd, 
					  (guchar *)ssBufDesc_GetMessage(bd), 
					  ssBufDesc_GetMessageLen(bd)) < 0 )
		    {
		      whiteboard_log_debug("Send failed\n");
		    }
		  subscription_finished = TRUE;
		  listener_remove_subscription(listener, (guchar *)parseSSAPmsg_get_subscriptionid(subwaiter->rsp));
		}
	      else
		{
		  whiteboard_log_debug("Got invalid response , id %s\n",parseSSAPmsg_get_subscriptionid(subwaiter->rsp));
		  listener_remove_subscription(listener, (guchar *)parseSSAPmsg_get_subscriptionid(subwaiter->rsp));
		  subscription_finished = TRUE;
		}
	    }
	  else
	    {
		//////One byte SOCKET EXPLORER/////////
		n=send(subwaiter->fd, " ", 1, 0);
		///////////////////////////////////////

		errorsock=getsockopt(subwaiter->fd, SOL_SOCKET, SO_ERROR, &optval, &optlen);  
		whiteboard_log_debug("Socket broken=%d \n",optval);


		//printf (" spaceid %s \n nodeid %s \n subid %s\n",spaceid,nodeid,subid);

		if ((optval!=0)||(n==-1))
		{
		      whiteboard_log_debug("\n Socket broken, i'll survive ;)\n");
		      
		      ///////////////////AUTO UNSUB/////////////////// 
		      if( sib_object_send_unsubscribe(listener->object,  
							   (guchar *)spaceid, 
							   (guchar *)nodeid,
							    1,
							   (guchar *)subid) <0)
			 {
			    whiteboard_log_debug("Could not send unsubscribe request w/ msgid : %d\n",1);
			 }
		      ///////////////////AUTO UNSUB////////////////////		
		      
		      listener_remove_subscription(listener, subid);
		      subscription_finished = TRUE;  
		      break;

		}
		//////////////////////////////////////////////////////
	      
	    }
	  ssBufDesc_free(&bd);
	}      
    }


  free (nodeid);
  free (subid);
  free (spaceid);

  whiteboard_log_debug_fe();
  return 0;
}

static int listener_handle_unsubscribe(Listener *listener, Client *client, NodeMsgContent_t *msg)
{
  ssStatus_t status;
  Waiter *waiter = NULL;
  ssBufDesc_t *bd = NULL;
  whiteboard_log_debug("Got unsubscribe request\n");
  waiter = waiter_new(client->fd);
  if(waiter)
    {
      if( listener_add_request(listener, (guchar *)parseSSAPmsg_get_nodeid( msg), parseSSAPmsg_get_msgnumber( msg), waiter ) < 0)
	{
	  whiteboard_log_debug("Could not add new request w/ msgid : %d\n", parseSSAPmsg_get_msgnumber( msg));
	  waiter_destroy(waiter);
	}
      else
	{
	  waiter->subscription_id = (guchar *)g_strdup(parseSSAPmsg_get_subscriptionid(msg));
	  whiteboard_log_debug("Sending unsubscibe_req w/ key: %s, subid: %s\n", waiter->key, waiter->subscription_id);
	  if( sib_object_send_unsubscribe( listener->object,  
					   (guchar *)parseSSAPmsg_get_spaceid(msg), 
					   (guchar *)parseSSAPmsg_get_nodeid(msg),
					   parseSSAPmsg_get_msgnumber(msg),
					   waiter->subscription_id) <0)
	    {
	      whiteboard_log_debug("Could not send unsubscribe request w/ msgid : %d\n", parseSSAPmsg_get_msgnumber( msg));
	      listener_remove_request(listener,(guchar *) parseSSAPmsg_get_nodeid( msg), parseSSAPmsg_get_msgnumber( msg));
	      status = ss_OperationFailed;

	      bd = ssBufDesc_new();
	      if( ss_StatusOK != ssBufDesc_CreateUnsubscribeResponse(bd, 
								     (guchar *)parseSSAPmsg_get_nodeid(msg),
								     (guchar *)parseSSAPmsg_get_spaceid(msg),
								     parseSSAPmsg_get_msgnumber(msg),
								     status,
								     parseSSAPmsg_get_subscriptionid(msg)))
		{
		  ssBufDesc_free(&bd);
		}
	    }
	  else
	    {
	      waiter_wait_for_finished(waiter);
	      if(waiter->rsp)
		{
		  bd = ssBufDesc_new();
		  if( ss_StatusOK != ssBufDesc_CreateUnsubscribeResponse(bd, 
									 (guchar *)parseSSAPmsg_get_nodeid(waiter->rsp),
									 (guchar *)parseSSAPmsg_get_spaceid(waiter->rsp),
									 parseSSAPmsg_get_msgnumber(waiter->rsp),
									 parseSSAPmsg_get_msg_status(waiter->rsp),
									 (guchar *)parseSSAPmsg_get_subscriptionid(waiter->rsp)))
		    {
		      ssBufDesc_free(&bd);
		    }
		}
	    }
	}
    
      if(bd) // there is a response message.
	{
	  if(client_send_message(waiter->fd, 
				 (guchar *)ssBufDesc_GetMessage(bd), 
				 ssBufDesc_GetMessageLen(bd)) < 0 )
	    {
	      whiteboard_log_debug("Send failed\n");
	    }
	  ssBufDesc_free(&bd);
	}
      
      listener_remove_request(listener, (guchar *)parseSSAPmsg_get_nodeid( msg), parseSSAPmsg_get_msgnumber(msg));
      
    }
  else
    {
      whiteboard_log_debug("Waitress, we are you\n");
    }

  whiteboard_log_debug_fe();
  return 0;
}

SibObject *create_control()
{
  whiteboard_log_debug_fb();
  SibObject *control = 	SIB_OBJECT(sib_object_new(NULL,
						  "sib_tcp_cc_id",
						  NULL,
						  "SIB Access",
						  "Not yet done"));
  g_return_val_if_fail(control!=NULL, NULL);
  
  g_signal_connect(G_OBJECT(control),
		   SIB_OBJECT_SIGNAL_REFRESH,
		   (GCallback) refresh_cb,
		   NULL);

  g_signal_connect(G_OBJECT(control),
		   SIB_OBJECT_SIGNAL_HEALTHCHECK,
		   (GCallback) healthcheck_cb,
		   NULL);
  
  g_signal_connect(G_OBJECT(control),
		   SIB_OBJECT_SIGNAL_SHUTDOWN,
		   (GCallback) shutdown_cb,
		   NULL);
  
  g_signal_connect(G_OBJECT(control),
		   SIB_OBJECT_SIGNAL_REGISTER_SIB,
		   (GCallback) register_sib_cb,
		   NULL);
  whiteboard_log_debug_fe();
  
  return control;  
}

void refresh_cb(SibObject* context,
		gpointer user_data)
{
  whiteboard_log_debug_fb();

  whiteboard_log_debug_fe();
}

void shutdown_cb(SibObject* context,
		 gpointer user_data)
{
  whiteboard_log_debug_fb();

  whiteboard_log_debug_fe();
}

void healthcheck_cb(SibObject* context,
		    gpointer user_data)
{
  whiteboard_log_debug_fb();

  whiteboard_log_debug_fe();
}

void register_sib_cb(SibObject* context,
		     SibObjectHandle *handle,
		     gchar *uri,
		     gpointer user_data)
{
  gint ret = -1;
  whiteboard_log_debug_fb();

  g_idle_add(create_listener_idle, NULL);

  ret = 0;
  sib_object_send_register_sib_return(handle, ret);
  whiteboard_log_debug("Listener created, sending register_sib_return\n");
  
  whiteboard_log_debug_fe();
}

static void handle_join_cnf( SibObject *context,
			     guchar *spaceid,
			     guchar *nodeid,
			     gint msgnum,
			     gint success,
			     guchar *credentials,
			     gpointer user_data)
{
  Listener *listener = (Listener *)user_data;
  Waiter *waiter = NULL;
  whiteboard_log_debug_fb();
  whiteboard_log_debug("Join_cnf(msgnum %d, nodeid: %s), success %d\n", msgnum, nodeid, success);
  waiter = listener_get_request(listener, nodeid, msgnum);
  if(waiter)
    {
      g_mutex_lock(waiter->lock);
      waiter->rsp = nodemsgcontent_new_join_rsp(spaceid, nodeid, msgnum, success);
      waiter_completed(waiter);
    }
  else
    {
      whiteboard_log_debug("Waiter w/ msg id: %d not found\n", msgnum);
    }
  
  whiteboard_log_debug_fe();
}

static void handle_leave_cnf( SibObject *context,
			      guchar *spaceid,
			      guchar *nodeid,
			      gint msgnum,
			      gint success,
			      gpointer user_data)
{
  Listener *listener = (Listener *)user_data;
  Waiter *waiter = NULL;
  whiteboard_log_debug_fb();
  whiteboard_log_debug("leave_cnf (msgnum %d, nodeid: %s), success %d\n", msgnum, nodeid, success);
  waiter = listener_get_request(listener, nodeid, msgnum);
  if(waiter)
    {
      g_mutex_lock(waiter->lock);
      waiter->rsp = nodemsgcontent_new_leave_rsp(spaceid, nodeid, msgnum, success);
      waiter_completed(waiter);
    }
  else
    {
      whiteboard_log_debug("Waiter w/ msg id: %d not found\n", msgnum);
    }
  
  whiteboard_log_debug_fe();
}

static void handle_insert_cnf( SibObject *context,
			       guchar *spaceid,
			       guchar *nodeid,
			       gint msgnum,
			       gint success,
			       guchar *bNodes,
			       gpointer user_data)
{
  Listener *listener = (Listener *)user_data;
  Waiter *waiter = NULL;
  whiteboard_log_debug_fb();
  whiteboard_log_debug("insert_cnf (msgnum %d, nodeid: %s), success %d\n", msgnum, nodeid, success);  
  waiter = listener_get_request(listener, nodeid, msgnum);
  if(waiter)
    {
      g_mutex_lock(waiter->lock);
      waiter->rsp = nodemsgcontent_new_insert_rsp(spaceid, nodeid, msgnum, success, bNodes);
      waiter_completed(waiter);
    }
  else
    {
      whiteboard_log_debug("Waiter w/ msg id: %d not found\n", msgnum);
    }
  
  whiteboard_log_debug_fe();
}

static void handle_remove_cnf( SibObject *context,
			       guchar *spaceid,
			       guchar *nodeid,
			       gint msgnum,
			       gint success,
			       gpointer user_data)
{
  Listener *listener = (Listener *)user_data;
  Waiter *waiter = NULL;
  whiteboard_log_debug_fb();
  whiteboard_log_debug("remove_cnf (msgnum %d, nodeid: %s), success %d\n", msgnum, nodeid, success);
  waiter = listener_get_request(listener, nodeid, msgnum);
  if(waiter)
    {
      g_mutex_lock(waiter->lock);
      waiter->rsp = nodemsgcontent_new_remove_rsp(spaceid, nodeid, msgnum, success);
      waiter_completed(waiter);
    }
  else
    {
      whiteboard_log_debug("Waiter w/ msg id: %d not found\n", msgnum);
    }
  
   whiteboard_log_debug_fe();
}

static void handle_update_cnf( SibObject *context,
			       guchar *spaceid,
			       guchar *nodeid,
			       gint msgnum,
			       gint success,
			       guchar *bNodes,
			       gpointer user_data)
{
  Listener *listener = (Listener *)user_data;
  Waiter *waiter = NULL;
  whiteboard_log_debug_fb();
  whiteboard_log_debug("update_cnf (msgnum %d, nodeid: %s), success %d\n", msgnum, nodeid, success);
  waiter = listener_get_request(listener, nodeid, msgnum);
  if(waiter)
    {
      g_mutex_lock(waiter->lock);
      waiter->rsp = nodemsgcontent_new_update_rsp(spaceid, nodeid, msgnum, success, bNodes);
      waiter_completed(waiter);
    }
  else
    {
      whiteboard_log_debug("Waiter w/ msg id: %d not found\n", msgnum);
    }
  
  whiteboard_log_debug_fe();
}

static void handle_query_cnf( SibObject *context,
			      guchar *spaceid,
			      guchar *nodeid,
			      gint msgnum,
			      gint success,
			      guchar *results,
			      gpointer user_data)
{
  Listener *listener = (Listener *)user_data;
  Waiter *waiter = NULL;
  whiteboard_log_debug_fb();
  whiteboard_log_debug("query_cnf (msgnum %d, nodeid: %s), success %d\n", msgnum, nodeid, success);
  waiter = listener_get_request(listener, nodeid, msgnum);
  if(waiter)
    {
      g_mutex_lock(waiter->lock);
      waiter->rsp = nodemsgcontent_new_query_rsp(spaceid, nodeid, msgnum, success, results);
      waiter_completed(waiter);
    }
  else
    {
      whiteboard_log_debug("Waiter w/ msg id: %d not found\n", msgnum);
    }
  
  whiteboard_log_debug_fe();
}

static void handle_subscribe_cnf( SibObject *context,
				  guchar *spaceid,
				  guchar *nodeid,
				  gint msgnum,
				  gint success,
				  guchar *subscription_id,
				  guchar *results,
				  gpointer user_data)
{
  Listener *listener = (Listener *)user_data;
  Waiter *waiter = NULL;
  whiteboard_log_debug_fb();
  whiteboard_log_debug("subscribe_cnf (msgnum %d, nodeid: %s), subscription_id: %s, success %d\n", msgnum, nodeid, subscription_id, success);
  waiter = listener_get_request(listener,nodeid, msgnum);
  if(waiter)
    {
      g_mutex_lock(waiter->lock);
      waiter->rsp = nodemsgcontent_new_subscribe_rsp(spaceid, nodeid, msgnum, success, subscription_id, results);
      waiter_completed(waiter);
    }
  else
    {
      whiteboard_log_debug("Waiter w/ msg id: %d not found\n", msgnum);
    }
  
  whiteboard_log_debug_fe();
}

static void handle_subscription_ind( SibObject *context,
				     guchar *spaceid,
				     guchar *nodeid,
				     gint msgnum,
				     gint seqnum,
				     guchar *subscription_id,
				     guchar *results_new,
				     guchar *results_obsolete,
				     gpointer user_data)
{
  Listener *listener = (Listener *)user_data;
  Waiter *waiter = NULL;
  whiteboard_log_debug_fb();
  whiteboard_log_debug("subscription_ind (msgnum %d, nodeid: %s), seqnum: %d, id: %s\n", msgnum, nodeid, seqnum, subscription_id);
  waiter = listener_get_subscription(listener, subscription_id);
  if(waiter)
    {
      g_mutex_lock(waiter->lock);
      waiter->rsp = nodemsgcontent_new_subscription_ind(spaceid, nodeid, msgnum, seqnum, subscription_id, results_new, results_obsolete);
      waiter_completed(waiter);
    }
  else
    {
      whiteboard_log_debug("Waiter w/ subscription id: %s not found\n", subscription_id);
    }
  
  whiteboard_log_debug_fe();
}

static void handle_unsubscribe_cnf( SibObject *context,
				    guchar *spaceid,
				    guchar *nodeid,
				    gint msgnum,
				    gint status,
				    guchar *subid,
				    gpointer user_data)
{
  Listener *listener = (Listener *)user_data;
  Waiter *waiter = NULL;
  Waiter *subwaiter = NULL;
  whiteboard_log_debug_fb();
  whiteboard_log_debug("unsubscription_cnf (msgnum %d, nodeid: %s), id: %s\n", msgnum, nodeid, subid);
  waiter = listener_get_request(listener, nodeid, msgnum);
  if(waiter)
    {
      subwaiter = listener_get_subscription(listener, subid);
      if(subwaiter)
	{
	  g_mutex_lock(subwaiter->lock);
	  subwaiter->rsp = nodemsgcontent_new_unsubscribe_rsp(spaceid, nodeid, msgnum, status, subid);
	  waiter_completed(subwaiter);
	}
      else
	{
	  whiteboard_log_debug("unsubscription_cnf, didnot find subwaiter w/ id: %s\n", subid);
	  waiter->rsp = nodemsgcontent_new_unsubscribe_rsp(spaceid, nodeid, msgnum, ss_OperationFailed, subid);
	}
      g_mutex_lock(waiter->lock);
      waiter_completed(waiter);
    }
  else
    {
      whiteboard_log_debug("Waiter w/ id: %d not found\n", msgnum);
    }
  
  whiteboard_log_debug_fe();
}

static void handle_unsubscribe_ind( SibObject *context,
				    guchar *spaceid,
				    guchar *nodeid,
				    gint msgnum,
				    gint status,
				    guchar *subscription_id,
				    gpointer user_data)
{
  whiteboard_log_debug_fb();

  whiteboard_log_debug_fe();
}

static void handle_leave_ind( SibObject *context,
			      guchar *spaceid,
			      guchar *nodeid,
			      gint msgnum,
			      gint status,
			      gpointer user_data)
{
  whiteboard_log_debug_fb();

  whiteboard_log_debug_fe();
}

gboolean create_control_idle(gpointer data)
{ 
  whiteboard_log_debug_fb();

  if(!control)
    {
      control = create_control();
      if(!control)
	{
	  whiteboard_log_debug("Could not create control connection\n");
	  g_main_loop_quit(mainloop);
	}
    }
  whiteboard_log_debug_fe();
    
  return FALSE;
}

gboolean create_listener_idle(gpointer data)
{ 
  whiteboard_log_debug_fb();

  if(!listener)
    {
      listener = listener_new();
      if(!listener)
	{ 
	  whiteboard_log_debug("Could not create listener, exiting...\n");
	  g_main_loop_quit(mainloop);
	}
    } 
  whiteboard_log_debug_fe();
  
  return FALSE;
}

static Waiter *waiter_new(int fd)
{
  whiteboard_log_debug_fb();
  Waiter *self = g_try_new0(Waiter,1);
  if(!self)
    {
      whiteboard_log_error("waiter_new(): memory alloc error\n");
    }
  else
    {
      self->lock = g_mutex_new();
      self->cond = g_cond_new();
      self->fd = fd;
      self->req_ready = FALSE;
    }
  whiteboard_log_debug_fe();
  return self;
}

int waiter_wait_for_finished(Waiter *self)
{
  whiteboard_log_debug_fb();

  whiteboard_log_debug("waiter_wait_for_finished: entering g_cond_wait w/ key: %s\n",(self->key ? self->key : self->subscription_id) );
  g_mutex_lock(self->lock);
  while( !self->req_ready )
    g_cond_wait( self->cond, self->lock);

  self->req_ready=FALSE;

  whiteboard_log_debug("waiter_wait_for_finished: g_cond_wait finished w/ key: %s\n",self->key);

  g_mutex_unlock(self->lock);
  whiteboard_log_debug_fe();
  return 0;
}

gint waiter_timed_wait_for_finished(Waiter *self, guint timeout_usec)
{
  GTimeVal timeout;
  gint ret = -1;
  whiteboard_log_debug_fb();
  
  whiteboard_log_debug("waiter_wait_for_finished: entering g_cond_wait w/ key: %s\n",(self->key ? self->key : self->subscription_id) );
  g_mutex_lock(self->lock);
  
  g_get_current_time(&timeout);
  g_time_val_add( &timeout, timeout_usec );

  while( !self->req_ready && (ret == -1) )
    ret = (g_cond_timed_wait( self->cond, self->lock, &timeout) ? 1:0) ;

  self->req_ready=FALSE;

  whiteboard_log_debug("waiter_timed_wait_for_finished: g_cond_wait finished w/ key: %s\n",(self->key ? self->key : self->subscription_id) );
  if(ret==0)
    whiteboard_log_debug("Timeout\n");

  g_mutex_unlock(self->lock);
  whiteboard_log_debug_fe();
  return ret;
}

void waiter_completed(Waiter *self)
{
  whiteboard_log_debug_fb(); 
  /* note: self->lock must be locked prior calling waiter_completed */

  whiteboard_log_debug("waiter_completed: g_cond_signal w/ key: %s\n", (self->key ? self->key : self->subscription_id) );

  self->req_ready = TRUE;
  g_cond_signal( self->cond );
  g_mutex_unlock(self->lock);
  whiteboard_log_debug_fe();
}

void waiter_destroy(Waiter *self)
{
  whiteboard_log_debug_fb(); 

  //  waiter_completed(self);
  g_mutex_lock(self->lock);
  g_mutex_unlock(self->lock);
  g_cond_free(self->cond);
  g_mutex_free(self->lock);

  if(self->rsp)
    nodemsgcontent_free( &self->rsp);  

  if(self->subscription_id)
    g_free(self->subscription_id);

  g_free(self);
  whiteboard_log_debug_fe();
}

static gint listener_add_request( Listener *self, guchar *nodeid, int msgid, Waiter *waiter)
{
  gint ret = -1;
  guchar *key = NULL;
  //  gchar *key = g_new0(gchar,  strlen(nodeid) + 1 + 5);
  //g_swhiteboard_log_debug(key,"%s_%5d", nodeid, msgid);
  
  whiteboard_log_debug_fb(); 
  g_mutex_lock(self->reqlock);
  key = get_request_key(nodeid, msgid);
  if( g_hash_table_lookup(self->requests , (gpointer)key ) == NULL)
    {
      g_hash_table_insert( self->requests, (gpointer)key, (gpointer)waiter);
      waiter->key=key;
      ret= 0;
    }
  else
    {
      g_free(key);
    }
  g_mutex_unlock(self->reqlock);
  whiteboard_log_debug_fe();
  return ret;
}

static guchar *get_request_key( guchar *nodeid, int msgid)
{
  GString *gkey = NULL;
  GString *gnum = NULL;
  guchar c;
  guchar *key = NULL;
  gkey = g_string_new( (gchar *)nodeid);
  gkey = g_string_append(gkey,"_");
  
  gnum = g_string_new(NULL);
  while( msgid )
    {
      c = (msgid % 10)+'0';
      gnum = g_string_prepend_c(gnum, c );
      msgid = msgid/10;
    }
  gkey = g_string_append(gkey, gnum->str);
  key = (guchar *)gkey->str;
  g_string_free(gnum, TRUE);
  g_string_free(gkey, FALSE);
  return key;
}

static Waiter *listener_get_request( Listener *self, guchar *nodeid, int msgid)
{
  Waiter *waiter  = NULL;
  guchar *key = get_request_key(nodeid, msgid);
  whiteboard_log_debug_fb(); 
  
  g_mutex_lock(self->reqlock);
  waiter = g_hash_table_lookup(self->requests , (gpointer)key );
  g_mutex_unlock(self->reqlock);
  g_free(key);
  whiteboard_log_debug_fe();
  return waiter;
}

static gint listener_remove_request( Listener *self, guchar *nodeid, int msgid)
{
  gint ret = -1;
  guchar *key = get_request_key(nodeid, msgid);
  whiteboard_log_debug_fb(); 
  g_mutex_lock(self->reqlock);
  if( g_hash_table_lookup(self->requests , (gpointer)key ) !=NULL)
    {
      g_hash_table_remove( self->requests, (gpointer)key);
      ret = 0;
    }
  g_mutex_unlock(self->reqlock);
  g_free(key);
  whiteboard_log_debug_fe();
  return ret;
}


static gint listener_add_subscription( Listener *self, guchar *subid, Waiter *waiter)
{
  gint ret = -1;
  guchar *key = NULL;
  whiteboard_log_debug_fb(); 
  g_mutex_lock(self->sublock);
  if( g_hash_table_lookup(self->subscriptions , (gpointer)subid ) == NULL)
    {
      key = (guchar *)g_strdup( (gchar *)subid);
      g_hash_table_insert( self->subscriptions, (gpointer) key, (gpointer)waiter);
      waiter->key=key;
      ret= 0;
    }
  g_mutex_unlock(self->sublock);
  whiteboard_log_debug_fe();
  return ret;
}

static Waiter *listener_get_subscription( Listener *self, guchar *subid)
{
  Waiter *waiter  = NULL;
  whiteboard_log_debug_fb(); 
  g_mutex_lock(self->sublock);
  waiter = g_hash_table_lookup(self->subscriptions , (gpointer)subid );
  g_mutex_unlock(self->sublock);
  whiteboard_log_debug_fe();
  return waiter;
}

static gint listener_remove_subscription( Listener *self, guchar *subid)
{
  gint ret = -1;
  whiteboard_log_debug_fb(); 
  g_mutex_lock(self->sublock);
  if( g_hash_table_lookup(self->subscriptions , (gpointer) subid ) !=NULL)
    {
      g_hash_table_remove( self->subscriptions, (gpointer) subid);
      ret = 0;
    }
  g_mutex_unlock(self->sublock);
  whiteboard_log_debug_fe();
  return ret;
}

gint client_send_message( int fd, guchar *buf, gint len)
{
  gint total = 0;        // how many bytes we've sent
  gint bytesleft = len; // how many we have left to send
  gint n;
  whiteboard_log_debug_fb();
  whiteboard_log_debug("Sending request: %s", buf);

  ///
  gint optval=0;
  gint optlen;
  gint errorsock;
  ///

  while(total < len)
    {
      ///
      errorsock=getsockopt(fd, SOL_SOCKET, SO_ERROR, &optval, &optlen);  
      if(optval != 0)
	{
     	  whiteboard_log_debug("Problems");
	  whiteboard_log_debug_fe();
	  return -1;
        }
      ///

      n = send(fd, buf+total, bytesleft, 0);
      if(n == -1)
	{
     	  whiteboard_log_debug("Problems");
	  whiteboard_log_debug_fe();
	  return -1;
	}
      total += n;
      bytesleft -= n;
    }

  whiteboard_log_debug("Ended transiction");
  whiteboard_log_debug_fe();
  return 0;
}

void parse_args(int argc, char * argv[])
{
  int i;

 
  for (i = 1; i < argc; i++)
    {
      if( (strcmp(argv[i], "--port") == 0) ||
	  (strcmp(argv[i], "-p") == 0) ) 
        {
	  /* port number */
	  if( i+1 >= argc )
	    {
	      print_usage(argv[0], 1);
	      
	    }
	  else
	    {
	      port = atoi( argv[++i] );
	      fprintf(stdout,"Launching in port %d \n",port);
	    }
	}
      if (strcmp(argv[i], "-sub-wake-fr") == 0) 
        {
	  /* port number */
	  if( i+1 >= argc )
	    {
	      print_usage(argv[0], 1);
	 
	    }
	  else
	    {
	      subwakefr = (int)(atoi( argv[++i] ));
	      fprintf(stdout,"Subscription explorer every %d sec \n",subwakefr);
	    }
	}
      if( (strcmp(argv[i], "--help") == 0) ||
	       (strcmp(argv[i], "-h") == 0) )
	{     /* Show help: */
	  puts("SIB-TCP \n");
	  printf("Usage: %s [OPTIONS] \n\n", argv[0]);
	  puts("Options:\n"
	       "  -h, --help          Show this screen.\n\n"
	       "  -p, --port	PORT  Listen to port PORT. Default 10010.\n"
	       "  -sub-wake-fr	TIME  Set frequency time (s) for exploring \n"
               "		      open subscriptions status.\n"
	       "		      With TIME at -1 exploring subscriptions \n"
	       "		      method is turned off.Default TIME=10(s). \n\n"
	       );
	  exit(0);
	}

    }
}

void print_usage(char * prog, int ret)
{
  FILE * fi;


  /* Determine which stream to write to: */

  if (ret == 0)
    fi = stdout;
  else
    fi = stderr;


  /* Display the usage message: */

  fprintf(fi, "Usage: %s [--port PORT] | --help \n",
          prog);


  /* Quit! */

  exit(ret);
}

