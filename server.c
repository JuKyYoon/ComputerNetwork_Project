#include <stdio.h> //stderr
#include <sys/socket.h>
#include <netinet/in.h> //IPPROTO_TCP
#include <string.h>
#include <unistd.h> 
#include <stdlib.h> // exit 사용
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MSG_SIZE 4096

void error_print(char *error_msg);
void request_print(char *response_msg, char *request_msg, int *response_size);
void request_file(char *response_msg, char *request_msg, char *filename, char *file_extension, int *response_size);
void request_handle(char *response_msg, char *request_msg, int *response_size);

char *http_status_code(int code);
void response_error(char *response_msg, int *response_size, int error_code);

void request_file_handler(char *response_msg, char *request_msg, char *filename, int *response_size, char *content_type);
char* get_content_type(char *filename, char *file_extension);

int main(int argc, char *argv[]){
    
    char *request_msg  = malloc( sizeof(char)*100000);// 보낼 메시지
    unsigned char *response_msg = malloc( sizeof(char)*100000000); // 받은 메시지
    
    struct sockaddr_in server_address, client_addr; // 소켓 주소를 담는 구조체, 서버와 클라이언트 2개 필요하다
    int socket_fd;



    //명령 인수 검사
    if (argc < 2) { // 수가 부족하면
        fprintf(stderr, "Argument is not valid.\n"); // 에러문 출력후 
        exit(0); // 클라이언트 종료
    }


    int port_number = atoi(argv[1]); // 포트번호를 정수로 변환 
    int server_socket_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP); // client와 똑같은 socket 생성
    if( server_socket_fd < 0 ){ // 소켓을 만들기 실패하면 종료
        error_print("Fail to open socket.\n"); 
    }

    
    memset (&server_address, 0, sizeof(server_address)); // 구조체를 0으로 초기화
    server_address.sin_family = AF_INET; // IPv4로 설정
    server_address.sin_port = htons(port_number); // htons함수를 통해 포트번호를 네트워크 바이트 순서로 변환해준다.
    server_address.sin_addr.s_addr = htonl(INADDR_ANY); // p주소를 자동으로 대입해주는 함수 사용 long형이라 htonl사용

    //소켓에 포트번호 배정
    if ( bind( server_socket_fd, (struct sockaddr *)&server_address, sizeof(server_address) ) < 0) {
        error_print("error to bind");
    }

    //클라이언트 요청 대기
    printf("server is waiting......\n");
    listen(server_socket_fd, 7); // 소켓의 연결 대기열 만들어서 대기상태. 연결 요청 대기 함수.
    // backlog는 요청 대기 큐의 크기 
    
    
    //요청이 들어오면 받아 줘야 한다.
    socklen_t req_client = sizeof(client_addr); // 클라이언트 주소 정보 길이를 미리 지정해준다.
    int response_size = 0;
    
    // char client_address[20]; 
    // inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, client_address, sizeof(client_address)); // 클라이언트 주소 정보

    while(1){
        memset(response_msg, 0 ,100000000);
        memset(request_msg, 0 ,100000);   
        if( (socket_fd = accept( server_socket_fd, (struct sockaddr *)&client_addr, &req_client)) < 0){
            error_print("Fail to accpet");
        }
        printf("connection is successful : address: %s, port = % d\n", 
            inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        
       
        if( read(socket_fd, request_msg, MSG_SIZE) < 0 ){
            error_print("Fail to read");
        }
        if(strlen(request_msg) == 0) { response_error(response_msg, &response_size, 402); }
        else{ request_handle(response_msg, request_msg, &response_size);}

        if ( write(socket_fd, response_msg, response_size)  < 0 ) { 
            error_print("Fail to writing to socket.");
        }
        close(socket_fd);
    }

    free(response_msg);
    free(request_msg);

    close(server_socket_fd);
    return 0;
}


void request_handle(char *response_msg, char *request_msg, int *response_size){
    char *request_msg_copy = malloc( sizeof(char)*100000); // 복사본 생성
    strcpy(request_msg_copy, request_msg); // 가씀씩 request_msg 가 안 받아 와짐
    char *filename; // '/' 로 잘라서 request 분석해보장
    char content_type[50];
    char *file_extesion;
    file_extesion = malloc(sizeof(char)*300);
    filename = malloc(sizeof(char)*600);
    strtok(request_msg_copy, " ");
    strcpy(filename, strtok(NULL," ")); // file name  ex) '/index.html', '/'
    free(request_msg_copy);

    if (!strcmp(filename, "/") ){ // 입력받은 파일이 없을 경우
        request_print(response_msg, request_msg, response_size); // request 파일을 출력해준다.
    }
    else{ // 입력받은 파일이 있을 경우
        strcpy(content_type, get_content_type(filename,file_extesion)); // 파일의 확장자 명을 가져온다.
        if( !strcmp(content_type, "nomake") || access(filename+1,F_OK) < 0 ){ // 확장자 얻는 과정에서 이상이 있다. 혹은 해당 파일이 없다.
            response_error(response_msg, response_size, 404);
        }
        else{ // 아무런 이상이 없다.
            request_file_handler(response_msg, request_msg, filename+1, response_size, content_type);
        }
    }
}

void request_print(char *response_msg, char *request_msg, int *response_size){
    char *p = strtok(request_msg, "\n");
    char *req = malloc(sizeof(char)*MSG_SIZE);
    memset(req,0,sizeof(char)*MSG_SIZE);
    while( p!= NULL){
        strcat(req,p);
        strcat(req,"<br>"); // 줄 나누기 구현위해 사용.
        p = strtok(NULL, "\n");
    }
    int content_length = 7 + strlen(req); // 처음부터 req에 strcat <h> 하고 나중에 </h>붙이고 길이 구하면 더 간단해질듯.
    *response_size = sprintf(response_msg, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-length: %d\r\n\r\n<h>%s</h>",content_length,req);
    free(req);
}


void response_error(char *response_msg, int *response_size, int error_code){
    char *status_str = http_status_code(error_code);
    int content_length = 18+strlen(status_str);
    *response_size = sprintf(response_msg, "HTTP/1.1 %d %s\r\nContent-Type: text/html\r\nContent-length: 27\r\n\r\n<h1>HTTP %d %s</h1>",error_code,status_str,error_code,status_str); // 404출력
}

void request_file_handler(char *response_msg, char *request_msg, char *filename, int *response_size, char *content_type){
    FILE *openfile;
    if( (openfile = fopen(filename, "rb")) == NULL ){
        response_error(response_msg, response_size, 403);
        return ;
    }
    fseek(openfile, 0, SEEK_END);
    int file_size = ftell(openfile);
    fseek(openfile, 0, SEEK_SET);

    if(file_size > 100000000){
        response_error(response_msg, response_size, 400);
        fclose(openfile);
        return ;
    }

    int leng= sprintf(response_msg, "HTTP/1.1 200 OK\r\ncontent-Type: %s\r\ncontent-length: %d\r\n\r\n", content_type, file_size);
     *response_size = file_size + leng;
    unsigned char *send_buffer;
    send_buffer = malloc(100000000);
    fread(send_buffer, file_size, 1, openfile);
    memcpy(response_msg+leng,  send_buffer, file_size);
    free(send_buffer);
    fclose(openfile);
}




char* get_content_type(char *filename, char *file_extension){
    if (!strcmp(filename, "/") ){ // 입력받은 파일이 없을 경우
        return "text/html"; // request 파일을 출력해준다.
    }

    char name[100];
    strcpy(name, filename);
    char *p = strchr(name,'.');
    if ( p == NULL){
        return "nomake"; // 파일 확장자가 없다.
    }
    if ( !strcmp(p, ".")){
        return "nomake"; // .이후에 확장자가 없다.
    }

    char file_type[20]; // 파일 확장자를 얻기 위한 문자열
    while( p != NULL){ // 파일 중간에 .이 들어갈 수 있으므로 맨 마지막에 있는 .을 찾아준다.
        strcpy(file_type, p); 
        p = strchr(p+1, '.');
    }

    strcpy(file_extension, file_type+1);
    if(strcmp(file_type+1, "html") == 0){
        return "text/html";
    }
    if(strcmp(file_type+1, "mp3") == 0){
        return "audio/mpeg3";
    }
    if(!strcmp(file_type+1, "gif")){
        return "image/gif";
    }
    if(!strcmp(file_type+1, "jpg") || !strcmp(file_type+1, "jpeg")){
        return "image/jpeg";
    }
    if(!strcmp(file_type+1, "png")){
        return "image/png";
    }
    if(!strcmp(file_type+1, "pdf")){
        return "application/pdf";
    }
    return "nomake"; // 구현이 안 된것.
}

void error_print(char *error_msg){
    perror(error_msg);
    exit(1);
}

char *http_status_code(int code){
    if( code == 400){
        return "Bad Request";;
    }
    if (code == 404){
        return "Not Found";
    }
    if (code == 403){
        return "Forbidden";
    }
    else{
        return "I don't know";
    }
}