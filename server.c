#include <stdio.h> //stderr
#include <sys/socket.h>
#include <netinet/in.h> //IPPROTO_TCP
#include <string.h>
#include <unistd.h> 
#include <stdlib.h> // exit 사용
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MSG_SIZE 100000000 // 최대 약 1억 byte크기의 파일이 전송될 수 있다. (헤더길이 때문에 1억byte보다 약간 더 작다.)


void request_handle(char *response_msg, char *request_msg, int *response_size); // 리퀘스트 처리 함수
void request_print(char *response_msg, char *request_msg, int *response_size); // Part1 요구사항 = 리퀘스트 메시지 출력
void request_file_handler(char *response_msg, char *request_msg, char *filename, int *response_size, char *content_type); // Part2 요구사항 = 파일 인식

void response_error(char *response_msg, int *response_size, int error_code); // HTTP 클라이언트 에러 처리

char *get_content_type(char *filename, char *file_extension); // 파일 확장자 및 content-type
char *http_status_code(int code); //  HTTP 상태

void error_print(char *error_msg); // 에러문 출력 및 서버 종료

int main(int argc, char *argv[]){
    char *request_msg  = malloc( sizeof(char)*MSG_SIZE);// 클라이언트에서 받은 메시지
    unsigned char *response_msg = malloc( sizeof(char)*MSG_SIZE); // 클라이언트로 보낼 메시지 // 바이너리 데이터가 들어갈 수 있으므로 Unsigned로 해준다.
    int response_size = 0; // 클라이언트로 보낼 메시지의 길이

    struct sockaddr_in server_address, client_addr; // 소켓 주소를 담는 구조체, 서버와 클라이언트 2개 필요하다
    int socket_fd; // 통신에 사용될 소켓 디스크립터.

    //명령 인수 검사
    if (argc < 2) { // 명령 인수 개수가 부족하면
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
    listen(server_socket_fd, 7); // 소켓의 연결 대기열 만들어서 대기상태. 연결 요청 대기 함수. // 요청 대기 큐는 7로 설정.
    
    //요청이 들어오면 받아 줘야 한다.
    socklen_t req_client = sizeof(client_addr); // 클라이언트 주소 정보 길이를 미리 지정해준다.
    
    while(1){
        memset(response_msg, 0 ,MSG_SIZE); // 클라이언트로 보낼 메시지 초기화
        memset(request_msg, 0 ,MSG_SIZE);   // 클라이언트에서 받은 메시지 초기화
        if( (socket_fd = accept( server_socket_fd, (struct sockaddr *)&client_addr, &req_client)) < 0){ // 요청 대기 큐에서 연결을 갖고와 연결된 소켓을 만든다.
            error_print("Fail to accpet");
        }
        printf("connection is successful : address: %s, port = % d\n", 
            inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port)); // 연결된 클라이언트의 주소와 포트 번호
        
       
        if( read(socket_fd, request_msg, MSG_SIZE) < 0 ){ // 클라이언트로 부터 메시지를 읽어 request_msg에 저장한다.
            error_print("Fail to read");
        }
        if(strlen(request_msg) == 0) { response_error(response_msg, &response_size, 402); } // 가끔씩 request길이가 0인경우가 생겨난다. segmantation fault를 피하기 위해 예외처리 해두었다.
        else{ request_handle(response_msg, request_msg, &response_size);} // request_handle() 함수를 request에 알맞는 response 메시지를 response_msg에 저장한다.

        if ( write(socket_fd, response_msg, response_size)  < 0 ) {  // response_msg를 클라이언트에 response_size 만큼 보낸다.
            error_print("Fail to writing to socket.");
        }
        close(socket_fd); // 연결된 소켓을 닫는다.
    }

    free(response_msg); //메모리 해제
    free(request_msg); // 메모리 해제
    close(server_socket_fd); //서버측 소켓을 닫아준다.
    return 0; // 프로그램 종료
}


void request_handle(char *response_msg, char *request_msg, int *response_size){ 
    char *request_msg_copy = malloc( sizeof(char)*MSG_SIZE); // strtok 때문에 복사본을 만든다.
    strcpy(request_msg_copy, request_msg); // 문자열 복사
    char *filename; // 파일 이름. 파일 이름 앞에 '/' 문자가 붙는다.
    char content_type[50]; // content-type
    char *file_extesion; // content-type을 구하기 위한 파일 확장자
    file_extesion = malloc(sizeof(char)*300);
    filename = malloc(sizeof(char)*600);
    strtok(request_msg_copy, " "); // GET method를 반환한다.
    strcpy(filename, strtok(NULL," ")); // 파일 이름을 저장한다.
    free(request_msg_copy); // 메모리 해제

    if (!strcmp(filename, "/") ){ // 파일 입력이 없는 경우에는
        request_print(response_msg, request_msg, response_size); // request message를 출력해준다. (프로젝트 Part1)
    }
    else{ // 입력받은 파일이 있을 경우
        strcpy(content_type, get_content_type(filename,file_extesion)); // 파일의 확장자 명과 content-type을 함수를 이용해 가져온다.
        if( !strcmp(content_type, "nomake") || access(filename+1,F_OK) < 0 ){ // 만약 인식하지 못하는 확장자명이거나 인식하지만 해당 파일이 존재하지 않는 경우에는
            response_error(response_msg, response_size, 404); // HTTP 404로 응답한다.
        }
        else{ // 아무런 이상이 없는 경우에는 
            request_file_handler(response_msg, request_msg, filename+1, response_size, content_type); // 함수를 이용해 response_msg를 만든다. // 파일이름앞의 '/'을 없애기 위해 +1 해주었다.
        }
    }
    free(filename); // 메모리 해제
    free(file_extesion); // 메모리 해제
}

void request_print(char *response_msg, char *request_msg, int *response_size){ // request message를 출력해주는 함수 (프로젝트 Part1)
    char *p = strtok(request_msg, "\n"); // request message를 html형태로 변환시키기 위해 줄바꿈 문자를 <br> 태그로 변경해 주었다.
    char *req = malloc(sizeof(char)*MSG_SIZE);
    memset(req,0,sizeof(char)*MSG_SIZE);
    while( p!= NULL){
        strcat(req,p);
        strcat(req,"<br>"); // 줄 바꿈 구현
        p = strtok(NULL, "\n");
    }
    int content_length = 7 + strlen(req); // content-length field를 위해 값을 구하였다.
    *response_size = sprintf(response_msg, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-length: %d\r\n\r\n<h>%s</h>",content_length,req); // response_msg를 만들고 response 메시지의 길이를 구했다.
    free(req); // 메모리 해제
}


void response_error(char *response_msg, int *response_size, int error_code){ //클라이언트에서 잘못 요청이 들어왔을 시 어떤 에러인지 보여주는 함수.
    char *status_str = http_status_code(error_code); // 에러 코드를 이용해 어떤 에러인지 문자열로 갖고온다.
    int content_length = 18+strlen(status_str); // content-length field를 위해 값을 구하였다.

    // response_msg를 만들고 response 메시지의 길이를 구했다. 어떤 에러인지 보여주기 위해 html로 response했다.
    *response_size = sprintf(response_msg, "HTTP/1.1 %d %s\r\nContent-Type: text/html\r\nContent-length: 27\r\n\r\n<h1>HTTP %d %s</h1>",error_code,status_str,error_code,status_str); 
}

void request_file_handler(char *response_msg, char *request_msg, char *filename, int *response_size, char *content_type){ // 파일이 요청되었다면 그에 해당되는 response를 해주는 함수.
    FILE *openfile; // 서버에서 여는 요청받은 파일
    if( (openfile = fopen(filename, "rb")) == NULL ){ // 파일이 열리지 않았다면
        response_error(response_msg, response_size, 403); // HTTP 403으로 응답한다.
        return ;
    }
    //파일 크기를 구하기 위한 과정
    fseek(openfile, 0, SEEK_END); // 파일 포인터를 맨 뒤로 옮긴다.
    int file_size = ftell(openfile); //현재의 위치 값
    fseek(openfile, 0, SEEK_SET); // 파일을 읽기위해 파일 포인터를 맨 앞으로 옮긴다.

    
    if(file_size > MSG_SIZE){ //만약 파일 크기가 전송 크기보다 더 크다면
        response_error(response_msg, response_size, 413); // HTTP 413으로 응답한다. (이 코드가 맞는지는 확실하지 않다.)
        fclose(openfile);
        return ;
    }

    int leng= sprintf(response_msg, "HTTP/1.1 200 OK\r\ncontent-Type: %s\r\ncontent-length: %d\r\n\r\n", content_type, file_size); // HTTP 헤더를 만든다.
    *response_size = file_size + leng; //HTTP헤더길이 + content-length = response message 길이
    unsigned char *send_buffer; // 파일을 읽을 버퍼
    send_buffer = malloc(MSG_SIZE); // 버퍼에 메모리 할당
    fread(send_buffer, file_size, 1, openfile); // openfile을 읽어 send_buffer에 저장한다.
    memcpy(response_msg+leng,  send_buffer, file_size); // HTTP 헤더에 읽은 파일 내용을 붙인다.
    free(send_buffer); // 메모리 해제
    fclose(openfile); // 파일 닫기
}

char* get_content_type(char *filename, char *file_extension){ //파일 이름으로 확장자와 content-type을 얻는다.
    if (!strcmp(filename, "/") ){ // 요청 받은 파일이 없을 경우
        return "text/html"; // request 파일을 출력해주기 위해 html형식을 반환한다.
    }

    char name[100]; // 파일 이름을 복사할 변수
    strcpy(name, filename); // 문자열 복사
    char *p = strchr(name,'.'); // 파일 이름에서 '.'을 찾는다.
    if ( p == NULL){ // '.' 이 없다.
        return "nomake"; // 파일 확장자가 없으면 인식할 수 없다.
    }
    if ( !strcmp(p, ".")){ // '.' 이후에 적혀있는 문자열이 없다
        return "nomake"; // 파일 확장자가 없으면 인식할 수 없다.
    }

    char file_type[20]; // 파일 확장자를 얻기 위한 문자열
     // 파일 이름 중간에 '.' 이 들어갈 수 있으므로 맨 마지막에 있는 '.'을 찾아준다.
    while( p != NULL){
        strcpy(file_type, p); 
        p = strchr(p+1, '.');
    }
    strcpy(file_extension, file_type+1); // file_type 맨 앞에 '.'문자가 있으므로 제외하고 파일 확장자만을 복사한다.

    //파일 확장자에 알맞는 content-type을 반환한다.
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
    return "nomake"; //지원하지 않는 확장자
}

void error_print(char *error_msg){ // 에러문을 출력하고 종료하는 함수
    perror(error_msg);
    exit(1);
}

char *http_status_code(int code){ // HTTP 에러 코드에 따른 에러문구를 얻기위한 함수
    switch(code){
        case 400:
            return "Bad Request";
            break;
        case 404:
            return "Not Found";
            break;
        case 403:
            return "Forbidden";
            break;
        case 413:
            return "Payload Too Large";
            break;
        default:
            return "I don't know"; // 일부만 구현했다.
    }
}