#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>

#define BUF_SIZE 108
#define PORT 8000

void handle_error(char* msg){
  perror(msg);
  exit(EXIT_FAILURE);
}

void convert(uint8_t *buf, char *str, ssize_t size) {
  if (size % 2 == 0)
    size = size / 2 - 1;
  else
    size = size / 2;

  for (int i = 0; i < size; i++)
    sprintf(str + i * 2, "%02X", buf[i]);
}

void generate_structured_message(char* message){
  uint8_t message_type = 0;
  uint32_t ip_address = htonl(127 << 24 | 0 << 16 | 0 << 8 | 1);
  uint16_t port_number = htons(1234);

  uint8_t random_data[100];
  if(getentropy(random_data, sizeof(random_data))!=0){
    handle_error("getentropy");
  }
  char hex_string[sizeof(random_data) * 2 + 1];
  convert(random_data, hex_string, sizeof(random_data));

  memcpy(message, &message_type, sizeof(uint8_t));
  memcpy(message + 1, &ip_address, sizeof(uint32_t));
  memcpy(message + 1 + sizeof(uint32_t), &port_number, sizeof(uint16_t));
  strcpy(message + 1 + sizeof(uint32_t) + sizeof(uint16_t), hex_string);
}

int main(int argc, char* argv[]){
  if(argc!=3){
    handle_error("Please provide two arguments, <max seconds>, <server IP address>");
  }

  int seconds_to_run = atoi(argv[1]);
  if(seconds_to_run <= 0){
    handle_error("Max seconds to run must be > 0");
  }

  char* server_ip = argv[2];
  int client_socket;
  struct sockaddr_in server_addr;
  char buffer[BUF_SIZE];

  client_socket = socket(AF_INET, SOCK_STREAM, 0);
  if(client_socket == -1){
    handle_error("socket");
  }
  
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT);

  if(inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0){
    handle_error("inet_pton");
  }

  if(connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1){
    handle_error("connect");
  }

  time_t start_time = time(NULL);
  
  while((time(NULL) - start_time) < seconds_to_run){
    char message[BUF_SIZE] = {0};
    generate_structured_message(message);

    if(send(client_socket, message, BUF_SIZE, 0) == -1){
      handle_error("send");
    }

    printf("%-20s%-10d%s\n", server_ip, PORT, message + 7);
    usleep(1000000);
  }

  uint8_t termination_message = 1;
  if(send(client_socket, &termination_message, sizeof(termination_message), 0) == -1){
    handle_error("Send (termination)");
  }
  printf("Sent Message: %d\n", termination_message);

  close(client_socket);
  
  return 0;
}
