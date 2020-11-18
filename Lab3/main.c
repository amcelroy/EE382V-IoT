#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdbool.h>

//Inspired by:
//https://stackoverflow.com/questions/22077802/simple-c-example-of-doing-an-http-post-and-consuming-the-response/22135885
//https://stackoverflow.com/questions/5444197/converting-host-to-ip-by-sockaddr-in-gethostname-etc
int main(int argc, char **argv){
	struct addrinfo hints, *res;

	char read_buffer[1024];
	char write_buffer[64];
	char buffer[128];

    //get host info, make socket and connect it
    memset(&hints, 0, sizeof hints);
    hints.ai_family=AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    getaddrinfo("amcelroy.dyndns.org","80", &hints, &res);
    printf(res->ai_addr->sa_data);
    fflush(stdout);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0){
    	return -1;
    }

    struct hostent *server = gethostbyname("amcelroy.dyndns.org");

    struct sockaddr_in serv_addr;

    uint8_t *address_as_uint8 = (uint8_t*)(server->h_addr_list[0]);
    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "Address is: %i.%i.%i.%i\r\n", address_as_uint8[0], address_as_uint8[1], address_as_uint8[2], address_as_uint8[3]);
    printf(buffer);
    fflush(stdout);

    memset(&serv_addr,0 , sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(80); //Converts to big endian
    memcpy(&serv_addr.sin_addr, server->h_addr_list[0], server->h_length);

    int c = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    if(c){
    	return -1;
    }

    char write_msg[] = "GET /snapshot HTTP/1.1\r\n\r\n";
    memcpy(write_buffer, write_msg, strlen(write_msg));
    c = write(sockfd, write_buffer, strlen(write_msg));

    FILE *f = fopen("snapshot.jpg", "wb");
    bool jpeg_start = false;
    memset(read_buffer, 0, sizeof(read_buffer));
    while(recv(sockfd, read_buffer, sizeof(read_buffer), 0)){
		if(jpeg_start == false){
			char *ptr = strstr(read_buffer, "\r\n\r\n");
			if(ptr){
				int offset = ptr - read_buffer + 4;
				fwrite(&read_buffer[offset], 1, sizeof(read_buffer) - offset, f);
		    	jpeg_start = true;
			}
		}else{
			char jpeg_end[3] = {0xFF, 0xD9};
			char *ptr = strstr(read_buffer, jpeg_end);
			if(ptr){
				int offset = ptr - read_buffer + 1;
				fwrite(read_buffer, 1, offset, f);
			}else{
				fwrite(read_buffer, 1, sizeof(read_buffer), f);
			}
		}
    	memset(read_buffer, 0, sizeof(read_buffer));
    }
    fclose(f);

    shutdown(sockfd, 0);

	return 1;
}
