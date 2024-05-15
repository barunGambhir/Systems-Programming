#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <errno.h>
#include <stdint.h>

#define PORT 8000
#define MAX_EVENTS 10
#define BUF_SIZE 108

typedef struct{
  int fd;
  uint32_t ip;
  uint16_t port;
} Client;

void handle_error(char* msg){
  perror(msg);
  exit(EXIT_FAILURE);
}

void broadcast_message(Client clients[], int num_clients, int sender_fd, char* message){
  for(int j=0; j<num_clients; ++j){
    if(clients[j].fd != sender_fd){
      char message_to_send[BUF_SIZE];
      message_to_send[0] = 0;
      memcpy(message_to_send + 1, &(clients[sender_fd].ip), sizeof(uint32_t));
      memcpy(message_to_send + 1 + sizeof(uint32_t), &(clients[sender_fd].port), sizeof(uint16_t));
      strcpy(message_to_send + 1 + sizeof(uint32_t) + sizeof(uint16_t), message);

      if(send(clients[j].fd, message_to_send, strlen(message) + 7, 0) == -1){
        handle_error("send");
      }
    }
  }
}

int main(int argc, char *argv[]){
  if(argc != 2){
    handle_error("Please add one argument: max no. of clients");
  }
  int max_clients = atoi(argv[1]);
  if(max_clients <= 0){
    handle_error("max no. of clients should > 0");
  }

  int server_socket, epoll_fd;
  struct sockaddr_in server_addr;
  struct epoll_event event, events[MAX_EVENTS];
  char buffer[BUF_SIZE];
  Client clients[max_clients];
  int num_clients = 0;

  server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if(server_socket == -1){
    handle_error("socket");
  }

  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  server_addr.sin_port = htons(PORT);

  if(bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1){
    handle_error("bind");
  }

  if(listen(server_socket, max_clients) == -1){
    handle_error("listen");
  }

  epoll_fd = epoll_create1(0);
  if(epoll_fd == -1){
    handle_error("epoll_create1");
  }

  event.events = EPOLLIN;
  event.data.fd = server_socket;
  if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_socket, &event) == -1){
    handle_error("epoll_ctl: server_socket");
  }

  printf("Server started. Waiting for connections...\n");

  while(1){
    int num_fds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
    if(num_fds == -1){
      handle_error("epoll_wait");
    }

    for(int i=0; i<num_fds; ++i){
      if(events[i].data.fd == server_socket){
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        if(client_socket == -1){
          handle_error("accept");
        }

        if(num_clients < max_clients){
          event.events = EPOLLIN | EPOLLET;
          event.data.fd = client_socket;
          if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_socket, &event)==-1){
          handle_error("epoll_ctl: client_socket");
          }

          clients[num_clients].fd = client_socket;
          clients[num_clients].ip = client_addr.sin_addr.s_addr;
          clients[num_clients].port = client_addr.sin_port;

          printf("New client connected: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
          num_clients++;
        }

        else{
          printf("Maximum clients reached. Connection rejected.\n");
          close(client_socket);
        }
      }

      else{
        int client_fd = events[i].data.fd;
        int bytes_received = recv(client_fd, buffer, sizeof(buffer), 0);
        if(bytes_received <= 0){
          for(int j=0; j<num_clients; j++){
            if(clients[j].fd == client_fd){
              printf("Client disconnected: %s:%d\n", inet_ntoa(*(struct in_addr *)&clients[j].ip), ntohs(clients[j].port));
              close(client_fd);

              if(j!=num_clients-1){
                clients[j] = clients[num_clients-1];
              }
              num_clients--;
              break;
            }
          }
          continue;
        }

        uint8_t message_type = buffer[0];
        if(message_type == 0){
          uint32_t sender_ip;
          uint16_t sender_port;
          char received_message[BUF_SIZE];
          memcpy(&sender_ip, buffer + 1, sizeof(uint32_t));
          memcpy(&sender_port, buffer + 1 + sizeof(uint32_t), sizeof(uint16_t));
          strcpy(received_message, buffer + 1 + sizeof(uint32_t) + sizeof(uint16_t));

          broadcast_message(clients, num_clients, client_fd, received_message);
//          printf("%-20s%-10d%s\n", inet_ntoa(*(struct in_addr *)&sender_ip), ntohs(sender_port), received_message);
        }

        else if (message_type == 1){
          int num_termination_signals = 0;
          for(int j=0; j<num_clients; ++j){
            if(clients[i].fd == client_fd){
              num_termination_signals++;
            }
          }

          if(num_termination_signals == num_clients){
            //All clients have sent termination signals
            uint8_t termination_message = 1;
            for(int j=0; j<num_clients; ++j){
              if(clients[j].fd != server_socket){
                if(send(clients[j].fd, &termination_message, sizeof(uint8_t), 0) == -1){
                  handle_error("send");
                }
              }
            }

            printf("All connected clients sent type 1 message \n");
            printf("Terminating the server...\n");
            close(server_socket);
            return EXIT_SUCCESS;
          }
        }
      }
    }
  }
  close(server_socket);
  return 0;
}
