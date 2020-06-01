#include <stdio.h>
#include <sys/socket.h> // socket 함수 사용
#include <netinet/in.h> // IPPROTO_TCP 사용
#include <sys/types.h> // socket 함수 사용
#include <netdb.h>  // hostent 구조체 사용
void error_print(char *error_msg);

int main(int argc, char *argv[]){
    if (argc < 3) { // 인자가 충분치 않다.
        fprintf("Argument is out of quntity."); // 에러문 출력후 클라이언트 종료
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
        error_print("Fail to open socket."); 
    }

    struct hostent *server;
    server = gethostbyname(argv[1]); // 도메인 주소로 ip주소 얻기위해 api사용.
    /* return value : 
    */

    if ()
    

   



}

void error_print(char *error_msg){
    perror(error_msg);
    exit(1);
}
