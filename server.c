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
void request_html_file(char *response_msg, char *request_msg, char *filename, int *response_size);
void request_mp3_file(char *response_msg, char *request_msg, char *filename, int *response_size);
void request_image_file(char *response_msg, char *request_msg, char *filename, int *response_size);
char* get_content_type(char *filename, char *file_extension);

int main(int argc, char *argv[]){
    
    char *request_msg; // 보낼 메시지
    unsigned char *response_msg; // 받은 메시지
    
    struct sockaddr_in server_address, client_addr; // 소켓 주소를 담는 구조체, 서버와 클라이언트 2개 필요하다
    int socket_fd;
    request_msg = malloc( (size_t)MSG_SIZE *5); 
    response_msg = malloc( (size_t)100000000);


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
        memset(response_msg, 0 ,MSG_SIZE*5);
        memset(request_msg, 0 ,MSG_SIZE*5);   
        if( (socket_fd = accept( server_socket_fd, (struct sockaddr *)&client_addr, &req_client)) < 0){
            error_print("Fail to accpet");
        }

        
        printf("connection is successful : address: %s, port = % d\n", 
            inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        
       
        if( read(socket_fd, request_msg, MSG_SIZE) < 0 ){
            error_print("Fail to read");
        }
        printf("accept the request\n");
        // printf("%s\n",request_msg);
        
        request_handle(response_msg, request_msg, &response_size);
        
        printf("***&&&&&%d--\n", response_size);

        if ( write(socket_fd, response_msg, response_size)  < 0 ) { 
            error_print("Fail to writing to socket.");
        }
        close(socket_fd);
        printf("----------------\n");
        
    }

    free(response_msg);
    free(request_msg);

    close(server_socket_fd);
    return 0;
}


void request_handle(char *response_msg, char *request_msg, int *response_size){
    printf("first request handle function\n");
    char request_msg_copy[MSG_SIZE*5]; // 복사본 생성
    strcpy(request_msg_copy, request_msg);
    // printf("\n%s\n", request_msg_copy);
    printf("%s",request_msg_copy);
    printf("complete copy\n");
    char method[50];
    char *filename; // '/' 로 잘라서 request 분석해보장
    char content_type[50];
    char *file_extesion;
    file_extesion = malloc(sizeof(char)*30);
    filename = malloc(sizeof(char)*200);
    printf("waiting strcpy method\n");
    strcpy(method, strtok(request_msg_copy, " ")); //method 'GET'
    printf("complete method\n");
    strcpy(filename, strtok(NULL," ")); // file name  ex) '/index.html', '/'
    printf("complete filename\n");
    if (!strcmp(filename, "/") ){ // 입력받은 파일이 없을 경우
        request_print(response_msg, request_msg, response_size); // request 파일을 출력해준다.
    }
    else{ // 입력받은 파일이 있을 경우
        strcpy(content_type, get_content_type(filename,file_extesion)); // 파일의 확장자 명을 가져온다.
        if( !strcmp(content_type, "nomake") || access(filename+1,F_OK) < 0 ){ // 확장자 얻는 과정에서 이상이 있다. 혹은 해당 파일이 없다.
            // char error_msg[20] = "HTTP 404 NOT FOUND";
            // int error_len = (int)strlen(error_msg) + 7;
            *response_size = sprintf(response_msg, "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-length: 27\r\n\r\n<h1>HTTP 404 NOT FOUND</h1>"); // 404출력
            // *response_msg = 
            printf("@@@@@@@@@@@@@@@\n%s\n@@@@@@@@@@@@@@@@\n",response_msg);
        }
        else{ // 아무런 이상이 없다.
            printf("request file function %s \n", filename);
            request_file(response_msg, request_msg, filename+1, file_extesion, response_size); // 파일 확장자에 따라 파일을 처리해 준다.
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
    printf("@@@@@@@@@@@@@@@\n%s\n@@@@@@@@@@@@@@@@\n",response_msg);
}

void request_file(char *response_msg, char *request_msg, char *filename, char *file_extension, int *response_size){
    //맨 처음엔
    printf("request file : %s\n", file_extension);
    if(!strcmp(file_extension, "html")){
        request_html_file(response_msg, request_msg, filename, response_size);
    }
    else if(!strcmp(file_extension, "mp3")){
        request_mp3_file(response_msg, request_msg, filename, response_size);
    }
    else if(!strcmp(file_extension, "jpeg") || !strcmp(file_extension, "jpg")){
        printf("image request!!1\n");
        request_image_file(response_msg, request_msg, filename, response_size);
    }
}

void request_html_file(char *response_msg, char *request_msg, char *filename, int *response_size){
    FILE* openfile; // html 파일
    openfile = fopen(filename, "r"); // read로 열자.
    char *req = malloc(sizeof(char)*MSG_SIZE); // 메모리 아낄러면 response에 계속 strcat 하는게 제일 좋을 듯.
    int idx = 0, c;
    while( (c = fgetc(openfile)) != EOF ){
        req[idx++] = c;
    }
    req[idx] = 0;
    fclose(openfile);
    *response_size = sprintf(response_msg, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-length: %ld\r\n\r\n%s",strlen(req), req);
    printf("@@@@@@@@@@@@@@@\n%s\n@@@@@@@@@@@@@@@@\n",response_msg);
}

void request_mp3_file(char *response_msg, char *request_msg, char *filename, int *response_size){
    printf("audio function wwwww\n");
    FILE *openfile;
    if( (openfile = fopen(filename, "rb")) < 0){ exit(0); }
    printf("open the file : %s\n",filename);
    fseek(openfile, 0, SEEK_END);
    int file_size = ftell(openfile);
    
    fseek(openfile, 0, SEEK_SET);
    int leng= sprintf(response_msg, "HTTP/1.1 200 OK\r\ncontent-Type: audio/mpeg3\r\ncontent-length: %d\r\n\r\n", file_size);
    *response_size = file_size + leng;
    printf("Sending Picture as Byte Array %d\n", file_size);
    // // no link between BUFSIZE and the file size
    unsigned char read_buffer[1000];

    unsigned char *send_buffer;
    send_buffer = malloc(100000000);

    memset(send_buffer, 0 , sizeof(char)*100000000);
    int cout = 0;
    int total = 0;
    printf("!!!!%ld\n",strlen(send_buffer));
    // fread(send_buffer, file_size, 1, openfile);
    int i=0,j;
    int c = 100;
    printf("buffer size : %d\n", sizeof(send_buffer));

    // printf("%d %d\n",response_msg[leng], '\n'); //leng -1지점이 \n 이고 leng이 NULL

    while (feof(openfile) == 0){
        cout = fread(send_buffer, 100000, 10, openfile);
        total += cout;
        printf("%d | total %d\n", i, total);
        // if( i > 10900000){
        //     printf("%d | ", i);
        //     for(int k=0; k<16;k++){
        //         printf("%02X ", (int)read_buffer[k]);
        //     }
        //     printf("\n");
        // }
        
        strncat(response_size, read_buffer,1000000);
        i++;
    }
    printf("___%d $\n", total);

    printf("leng is %d\n",leng);
    printf("%02X\n",(int)send_buffer[0]);
    printf("%02X\n",(int)send_buffer[1]);
    printf("%02X\n",(int)send_buffer[file_size-2]);
    printf("%02X\n",(int)send_buffer[file_size-1]); // last data
    printf("%02X\n",(int)send_buffer[file_size]);
    printf("%02X\n",(int)send_buffer[file_size+1]);
    memcpy(response_msg+leng,  send_buffer, file_size);
    // strcat(request_msg, send_buffer);
    printf("!!!!%ld\n",strlen(send_buffer));

    fclose(openfile);
}

void request_image_file(char *response_msg, char *request_msg, char *filename, int *response_size){
    printf("image function wwwww\n");
    FILE *openfile;
    if( (openfile = fopen(filename, "rb")) < 0){ exit(0); }
    printf("open the file : %s\n",filename);
    fseek(openfile, 0, SEEK_END);
    int file_size = ftell(openfile);
    
    fseek(openfile, 0, SEEK_SET);
    int leng= sprintf(response_msg, "HTTP/1.1 200 OK\r\ncontent-Type: image/jpeg\r\ncontent-length: %d\r\n\r\n", file_size);
    *response_size = file_size + leng;
    printf("Sending Picture as Byte Array %d\n", file_size);
    // // no link between BUFSIZE and the file size
    unsigned char read_buffer[1000];

    unsigned char *send_buffer;
    send_buffer = malloc(sizeof(char)*100000000);

    memset(send_buffer, 0 , sizeof(char)*100000000);
    int cout = 0;
    int total = 0;
    printf("!!!!%ld\n",strlen(send_buffer));
    fread(send_buffer, file_size, 1, openfile);
    int i=0,j;
    int c = 100;
    printf("buffer size : %d\n", sizeof(send_buffer));

    printf("%d %d\n",response_msg[leng], '\n'); //leng -1지점이 \n 이고 leng이 NULL

    printf("leng is %d\n",leng);
    memcpy(response_msg+leng,  send_buffer, file_size);

    printf("!!!!%ld\n",strlen(send_buffer));

    fclose(openfile);
    
}

char* get_content_type(char *filename, char *file_extension){
    if (!strcmp(filename, "/") ){ // 입력받은 파일이 없을 경우
        return "requestprint"; // request 파일을 출력해준다.
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
    return "nomake"; // 구현이 안 된것.
}

void error_print(char *error_msg){
    perror(error_msg);
    exit(1);
}
