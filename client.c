#include <stdio.h> // printf 사용
#include <stdlib.h> // exit 사용
#include <sys/socket.h> // socket 함수 사용
#include <netinet/in.h> // IPPROTO_TCP 사용
#include <sys/types.h> // socket 함수 사용
#include <netdb.h>  // hostent 구조체 사용
#include <string.h>
#include <unistd.h> // read, write

#include <arpa/inet.h> //inet_addr 사용
#define MSG_SIZE 256
void error_print(char *error_msg);

int main(int argc, char *argv[]){
    if (argc < 3) { // 인자가 충분치 않다.
        fprintf(stderr, "Argument is out of quntity.\n"); // 에러문 출력후 클라이언트 종료
        exit(0);
    }

    int port_number = atoi(argv[2]); // 포트번호를 정수로 변환 
    if( port_number < 0 ){ error_print("Not valid Port"); } // 인자로 받은 포트 값이 적절치 않으면 종료

    int client_socket_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP); // 새로운 소켓 생성
    /* PF_INET : IPv4 영역
     SOCK_STREAM : TCP 연결 통신 type
     IPPROTO_TCP : TCP 프로토콜
     return value : 1 실패 , 양의 정수 값 성공 
     */

    if( client_socket_fd < 0 ){ // 소켓을 만들기 실패하면 종료
        error_print("Fail to open socket.\n"); 
    }

    struct hostent *server;
     
    if ( (server = gethostbyname(argv[1])) == NULL) { 
        // 도메인 주소로 ip주소 얻기위해 API사용.
        // return value : 성공시 hostent구조체의 주소 값, 실패시 Null
        fprintf(stderr, "Host error\n");
        exit(0);
    }

    struct sockaddr_in server_address; // 소켓 주소를 담는 구조체
    memset (&server_address, 0, sizeof(server_address)); // 구조체를 0으로 초기화
    server_address.sin_family = AF_INET; // IPv4로 설정
    server_address.sin_port = htons(port_number); // htons함수를 통해 포트번호를 네트워크 바이트 순서로 변환해준다.
    // server_address.sin_addr.s_addr = server->h_addr; // 서버주소에 IP 저장
    // server_address.sin_addr.s_addr = inet_addr(server->h_addr); // 이거 오류남
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    // 서버와 연결!
    if( connect( client_socket_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0){
        error_print("Fail to connect");
    }

    /*connect 함수
        첫번쨰 인자는 소켓 디스크립터다.
        두번쨰 인자는 서버의 주소 정보이다
        세번쨰 인자는 서버의 주소 정보 크기이다.
        첫번째 인자의 소켓과 서버의 주소 정보에 해당되는 소켓이 연결
    */

    // 연결되었음
    char *request_msg; // 메시지가 담길 변수
    request_msg = malloc( (size_t)MSG_SIZE ); // 256 바이트만큼 공간 할당.
    char *response_msg;
    response_msg = malloc( (size_t)MSG_SIZE ); // 256 바이트만큼 공간 할당.

    while(1){
        memset(response_msg, 0 ,MSG_SIZE);
        memset(request_msg, 0 ,MSG_SIZE);
    

        printf("enter the msg : ");
        fgets(request_msg, MSG_SIZE, stdin ); // 메시지 입력을 받는다.


        if( strcmp(request_msg, "exit\n") == 0){
            break;
        }

        if ( write(client_socket_fd, request_msg, strlen(request_msg))  < 0 ) { 
            // write함수를 이용해 소켓에 메시지를 작성한다.
            error_print("Fail to writing to socket.");
        }
        // 오류가 없다면 메시지는 서버로 잘 전달되었을 것이다.


        if ( read( client_socket_fd, response_msg, MSG_SIZE) < 0 ){//소켓 디스크립터에서 읽기
            error_print("Fail to reading from socket.");
        } 
        printf("to client from server : %s", response_msg);

        
    }


    free(response_msg);
    free(request_msg);

    close(client_socket_fd);
    return 0;


}

void error_print(char *error_msg){
    perror(error_msg);
    exit(1);
}
