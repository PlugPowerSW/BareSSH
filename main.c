#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#define BUFFER_SIZE 1024

typedef enum {
  INIT,
  WAIT_FOR_CONNECTION,
  HANDSHAKE,
  KEY_EXCHANGE,
  AUTHENTICATION,
  SESSION_ESTABLISHED,
  PROCESS_REQUEST,
  ERROR,
  CLOSE_CONNECTION
} State;

typedef struct {
  State current_state;
  int server_fd;
  int client_fd;
  struct sockaddr_in client_addr;
  socklen_t client_addr_len;
  char buffer[BUFFER_SIZE];
  ssize_t bytes_received;
} SSHServer;

void server_poll( SSHServer *server );

void init_server( SSHServer *server );
void check_for_connection( SSHServer *server );
void handle_handshake( SSHServer *server );
void handle_key_exchange( SSHServer *server );
void handle_authentication( SSHServer *server );
void init_established_session( SSHServer *server );
void process_request( SSHServer *server );
void handle_error( SSHServer *server );
void close_connection( SSHServer *server );

//small example main, this will eventually be whatever polling function you guys are using
int main( int argc, char *argv[] ) {
  SSHServer server = { .current_state = INIT };

  while(1)
  {
    server_poll(&server);
  }
  
  return 0;
}

void server_poll( SSHServer *server )
{
  //perform timeout checks here

  switch(server->current_state)
  {
    case INIT:
      init_server(server);
      break;
    case WAIT_FOR_CONNECTION:
      check_for_connection(server);
      break;
    case HANDSHAKE:
      handle_handshake(server);
      break;
    case KEY_EXCHANGE:
      handle_key_exchange(server);
      break;
    case AUTHENTICATION:
      handle_authentication(server);
      break;
    case SESSION_ESTABLISHED:
      init_established_session(server);
      break;
    case PROCESS_REQUEST:
      process_request(server);
      break;
    case ERROR:
      handle_error(server);
      break;
    case CLOSE_CONNECTION:
      close_connection(server);
      break;
  }
}

void init_server( SSHServer *server )
{
  server->server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server->server_fd == -1 )
  {
    printf("Socket creation failed: %s\n", strerror(errno));
    server->current_state = ERROR;
    return;
  }

  int reuse = 1;
  if (setsockopt(server->server_fd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) < 0)
  {
    printf("SO_REUSEPORT failed: %s\n", strerror(errno));
    server->current_state = ERROR;
    return;
  }

  struct sockaddr_in serv_addr = { .sin_family = AF_INET ,
                                   .sin_port = htons(4221),
                                   .sin_addr = { htonl(INADDR_ANY) },
  };

  if (bind(server->server_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) != 0)
  {
    printf("Bind failed: %s\n", strerror(errno));
    server->current_state = ERROR;
    return;
  }

  int connection_backlog = 5;
  if (listen(server->server_fd, connection_backlog) != 0)
  {
    printf("Listen failed: %s\n", strerror(errno));
    server->current_state = ERROR;
    return;
  }

  server->current_state = WAIT_FOR_CONNECTION;
}

void check_for_connection( SSHServer *server )
{
  server->client_addr_len = sizeof(server->client_addr);
  server->client_fd = accept(server->server_fd, (struct sockaddr*) &server->client_addr, &server->client_addr_len);
  if (server->client_fd == -1)
  {
    printf("Accept failed: %s\n", strerror(errno));
    server->current_state = ERROR;
  }
  else
  {
    printf("Client connected\n");
    server->current_state = HANDSHAKE;
  }
}

void handle_handshake( SSHServer *server )
{
  //Write code for this later once skeleton is fleshed out
  //Assume successful for now
  printf("Performing handshake...\n");
  server->current_state = KEY_EXCHANGE;
}

void handle_key_exchange( SSHServer *server )
{
  //Perform key exchange
  //Again, I'll just assume successful for now
  printf("Performing key exchange...\n");
  server->current_state = AUTHENTICATION;
}

void handle_authentication( SSHServer *server )
{
  //authenticate the client
  printf("Authenticating client...\n");
  server->current_state = SESSION_ESTABLISHED;
}

//this state might be dumb and redundant but I figured there might be some
//sort of initialization to do before we started directly processing requests
void init_established_session( SSHServer *server )
{
  printf("Session established. Ready to process requests...\n");
  server->current_state = PROCESS_REQUEST;
}

void process_request( SSHServer *server )
{
  //BUFFER_SIZE - 1 because I'm guaranteeing a null-terminated string for now.
  server->bytes_received = recv(server->client_fd, server->buffer, BUFFER_SIZE - 1, 0);
  if (server->bytes_received > 0)
  {
    server->buffer[server->bytes_received] = '\0';  // Null-terminate the received data
    //do something with received data
  } 
  else if (server->bytes_received == 0)
  {
    printf("Client disconnected\n");
    server->current_state = CLOSE_CONNECTION;
  }
  else
  {
    if (errno != EWOULDBLOCK && errno != EAGAIN)
    {
      printf("Read failed: %s\n", strerror(errno));
      server->current_state = ERROR;
    }
  }
}

void handle_error( SSHServer *server )
{
  //handle error, clean-up, etc
  //should probably add some sort of error code system but for now just treat
  //everything the same
  server->current_state = CLOSE_CONNECTION;
}

void close_connection( SSHServer *server )
{
  //lots of clean-up to do here eventually
  if (server->client_fd != -1) {
    close(server->client_fd);
    server->client_fd = -1;
  }
  server->current_state = WAIT_FOR_CONNECTION;
}
