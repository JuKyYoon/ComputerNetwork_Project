#include <stdio.h> //stderr
#include <sys/socket.h>
#include <netinet/in.h> //IPPROTO_TCP
#include <string.h>
#include <unistd.h> 
#include <stdlib.h> // exit 사용

#define MSG_SIZE 256

void error_print(char *error_msg);

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
    

    //요청이 들어오면 받아 줘야 한다.
    socklen_t req_client = sizeof(client_addr);
    // 클라이언트 주소 정보 길이를 미리 지정해준다.

    int socket_fd = accept( server_socket_fd, (struct sockaddr *)&client_addr, &req_client);
    /*
        첫번째 인자 : 서버 소켓 디스크립터
        두번쨰 인자 : 클라이언트 주소 정보
        세번쨰인자 : 클라이언트 주소 정보 길이
    */
    // 새 소켓 디스크립터 생성해서 클라이언트의 request를 받는다.

    printf("connection is successful\n");
    
    if( socket_fd < 0){
        error_print("Fail to accpet");
    }
    char *request_msg; // 보낼 메시지
    request_msg = malloc( (size_t)MSG_SIZE ); // 256 바이트만큼 공간 할당.
    char *response_msg; // 받은 메시지
    response_msg = malloc( (size_t)MSG_SIZE ); // 256 바이트만큼 공간 할당.

    while(1){
        memset(response_msg, 0 ,MSG_SIZE);
        memset(request_msg, 0 ,MSG_SIZE);

        if( read(socket_fd, response_msg, MSG_SIZE) < 0 ){
            error_print("Fail to read");
        }

        printf("to server from client : %s", response_msg);
        printf("enter the msg : ");
        fgets(request_msg, MSG_SIZE, stdin ); // 메시지 입력을 받는다.
        if ( write(socket_fd, request_msg, strlen(request_msg))  < 0 ) { 
            // write함수를 이용해 소켓에 메시지를 작성한다.
            error_print("Fail to writing to socket.");
        }
        // 오류가 없다면 메시지는 서버로 잘 전달되었을 것이다.
    }

    



    free(response_msg);
    free(request_msg);

    close(socket_fd);
    return 0;
}









void error_print(char *error_msg){
    perror(error_msg);
    exit(1);
}