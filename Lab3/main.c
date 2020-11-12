#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

//Inspired by:
//https://stackoverflow.com/questions/22077802/simple-c-example-of-doing-an-http-post-and-consuming-the-response/22135885
//https://stackoverflow.com/questions/5444197/converting-host-to-ip-by-sockaddr-in-gethostname-etc
int main(int argc, char **argv){
	struct addrinfo hints, *res;

	char read_buffer[1024];
	char write_buffer[1024];

    //get host info, make socket and connect it
    memset(&hints, 0, sizeof hints);
    hints.ai_family=AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    getaddrinfo("amcelroy.dyndns.org","80", &hints, &res);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0){
    	return -1;
    }

    struct hostent *server = gethostbyname("amcelroy.dyndns.org");

    struct sockaddr_in serv_addr;
    memset(&serv_addr,0 , sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(80); //Converts to big endian
    memcpy(&serv_addr.sin_addr, server->h_addr_list[0], server->h_length);

    int c = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    if(c){
    	return -1;
    }

    memcpy(write_buffer, "GET / HTTP/1.1\r\n\r\n", strlen("GET / HTTP/1.1\r\n\r\n"));

    c = write(sockfd, write_buffer, strlen("GET / HTTP/1.1\r\n\r\n"));

    while(c != 0){
        c = read(sockfd, read_buffer, sizeof(read_buffer));
        printf(read_buffer);
    }

	return 1;
}
