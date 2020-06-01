#include <stdio.h> //stderr
#include <sys/socket.h>
#include <netinet/in.h> //IPPROTO_TCP
#include <string.h>
#include <unistd.h> 
#include <stdlib.h> // exit 사용
#include <arpa/inet.h>

#define MSG_SIZE 4096

void error_print(char *error_msg);
void request_print(char *response_msg, char *request_msg);
void request_file(char *response_msg, char *request_msg);
int main(int argc, char *argv[]){
    if (argc < 2) { // 인자가 충분치 않다.
        fprintf(stderr, "Argument is out of quntity.\n"); // 에러문 출력후 클라이언트 종료
        exit(0);
    }

    int port_number = atoi(argv[1]); // 포트번호를 정수로 변환 



    int server_socket_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP); // client와 똑같은 socket 생성
    if( server_socket_fd < 0 ){ // 소켓을 만들기 실패하면 종료
        error_print("Fail to open socket.\n"); 
    }

    struct sockaddr_in server_address, client_addr; // 소켓 주소를 담는 구조체, 서버와 클라이언트 2개 필요하다

    memset (&server_address, 0, sizeof(server_address)); // 구조체를 0으로 초기화
    server_address.sin_family = AF_INET; // IPv4로 설정
    server_address.sin_port = htons(port_number); // htons함수를 통해 포트번호를 네트워크 바이트 순서로 변환해준다.
    server_address.sin_addr.s_addr = htonl(INADDR_ANY); // p주소를 자동으로 대입해주는 함수 사용 long형이라 htonl사용

    //소켓에 포트번호 배정
    if ( bind( server_socket_fd, (struct sockaddr *)&server_address, sizeof(server_address) ) < 0) {
        error_print("error to bind");
    }

    /* 
        첫번쨰 인자 : 서버 소켓 디스크립터
        두번째 인자 : 서버의 주소 정보
        세번째 인자 : 서버으 주소 정보의 크기
        성공시  0 실패시 -1 반환
    */
    printf("server is waiting......\n");
    listen(server_socket_fd, 7); // 소켓의 연결 대기열 만들어서 대기상태. 연결 요청 대기 함수.
    // backlog는 요청 대기 큐의 크기 
    
    int socket_fd;
    //요청이 들어오면 받아 줘야 한다.
    socklen_t req_client = sizeof(client_addr); // 클라이언트 주소 정보 길이를 미리 지정해준다.
    
    
    // char client_address[20]; 
    // inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, client_address, sizeof(client_address)); // 클라이언트 주소 정보
    char *request_msg; // 보낼 메시지
    char *response_msg; // 받은 메시지
    request_msg = malloc( (size_t)MSG_SIZE ); // 256 바이트만큼 공간 할당.
    response_msg = malloc( (size_t)MSG_SIZE ); // 256 바이트만큼 공간 할당.
    
    

    while(1){
        memset(response_msg, 0 ,MSG_SIZE);
        memset(request_msg, 0 ,MSG_SIZE);       
        if( (socket_fd = accept( server_socket_fd, (struct sockaddr *)&client_addr, &req_client)) < 0){
            error_print("Fail to accpet");
        }
            /*
                첫번째 인자 : 서버 소켓 디스크립터
                두번쨰 인자 : 클라이언트 주소 정보
                세번쨰인자 : 클라이언트 주소 정보 크기
            */
            // 새 소켓 디스크립터 생성해서 클라이언트의 request를 받는다.
        printf("connection is successful : address: %s, port = % d\n", 
            inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        
        // if( recv(socket_fd, request_msg, sizeof(request_msg), 0 ) < 0 ){
        //     error_print("receive error");
        // }
        if( read(socket_fd, request_msg, MSG_SIZE) < 0 ){
            error_print("Fail to read");
        }
        
        
        // strcpy(response_msg, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-length: 40\r\n\r\n<h1>hello world</h1>");
        // strcat(response_msg, "<h>abcdefglkqwed</h>");
        // if ( send(socket_fd, response_msg, strlen(response_msg), 0) < 0 ){
        //     error_print("send error");
        // }
        // printf("%s\n",request_msg);
        // printf("%s\n",response_msg);
        // request_print(response_msg, request_msg);
        request_file(response_msg, request_msg);
        if ( write(socket_fd, response_msg, strlen(response_msg))  < 0 ) { 
            error_print("Fail to writing to socket.");
        }

        close(socket_fd);
        printf("-----------------------------------------------\n");
    }

    free(response_msg);
    free(request_msg);

    close(server_socket_fd);
    return 0;
}


void request_print(char *response_msg, char *request_msg){
    char proto[] = "HTTP/1.1 200 OK\r\n";
    char content_type[] = "Content-Type: text/html\r\n";
    // char content_length[] = "Content-length: 4096\r\n\r\n";
    printf("\n%s\n",request_msg);
    char *p = strtok(request_msg, "\n");
    char *req = malloc(sizeof(char)*MSG_SIZE);
    memset(req,0,sizeof(char)*MSG_SIZE);
    while( p!= NULL){
        strcat(req,p);
        strcat(req,"<br>");
        p = strtok(NULL, "\n");
    }
    int content_length = 7 + strlen(req);
    sprintf(response_msg, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-length: %d\r\n\r\n<h>%s</h>",content_length,req);
}

void request_file(char *response_msg, char *request_msg){
    FILE* openfile;
    char method[10];
    char filename[30]; // '/' 로 잘라서 request 분석해보장
    strcpy(method, strtok(request_msg, "/")); //method(GET)얻을 것으로 예상. 근데 정확히 "GET "을 얻을것으로 예상
    strcpy(filename, strtok(NULL,"/")); // file name  ex) index.html
    strtok(filename, " ");
    if( strcmp(filename, "favicon.ico" ) == 0) { return ;} // segmantation 오류 임시 방편
    char proto[] = "HTTP/1.1 200 OK\r\n";
    char content_type[] = "Content-Type: text/html\r\n"; // filename을 strtok해서 얻어야 될듯 일단 html로 고정시키자.
    // printf("%s\n",filename);
    openfile = fopen(filename, "r"); // read로 열자.
    char *req = malloc(sizeof(char)*MSG_SIZE);
    int idx = 0;
    int c;
    while( (c = fgetc(openfile)) != EOF ){
        req[idx++] = c;
    }
    fclose(openfile);

    int content_length = strlen(req);
    sprintf(response_msg, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-length: %d\r\n\r\n<h>%s</h>",content_length, req);

}

void error_print(char *error_msg){
    perror(error_msg);
    exit(1);
}
