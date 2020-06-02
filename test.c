#include <stdio.h> //stderr
#include <sys/socket.h>
#include <netinet/in.h> //IPPROTO_TCP
#include <string.h>
#include <unistd.h> 
#include <stdlib.h> // exit 사용
#include <arpa/inet.h>


char* get_content_type(char *filename, char *file_extension);

void request_handle(char *response_msg, char *request_msg){
    char *request_msg_copy = request_msg; // 복사본 생성

    char method[10];
    char filename[30]; // '/' 로 잘라서 request 분석해보장
    char content_type[10];
    strcpy(method, strtok(request_msg_copy, "/")); //method(GET)얻을 것으로 예상. 근데 정확히 "GET "을 얻을것으로 예상
    strcpy(filename, strtok(NULL,"/")); // file name  ex) index.html
    strtok(filename, " ");

}





int main(){

    char request_msg_copy[1024] = "GET /serious.mp3 HTTP/1.1"; // 복사본 생성
    char method[10];
    char filename[30]; // '/' 로 잘라서 request 분석해보장
    char content_type[50];
    // strcpy(method, strtok(request_msg_copy, " ")); //method 'GET'
    printf("%s\n",strtok(request_msg_copy, " "));
    strcpy(filename, strtok(NULL," ")); // file name  ex) '/index.html', '/'

    char *file_extesion;
    file_extesion = malloc(sizeof(char)*30);

     if (!strcmp(filename, "/") ){ // 입력받은 파일이 없을 경우
        return 0;
    }
    else{ // 입력받은 파일이 있을 경우
        strcpy(content_type, get_content_type(filename, file_extesion)); // 파일의 확장자 명을 가져온다.
        if( !strcmp(content_type, "nomake")){ // 확장자 얻는 과정에서 이상이 있다.
            // char error_msg[20] = "HTTP 404 NOT FOUND";
            // int error_len = (int)strlen(error_msg) + 7;
            return 0;
        }
        else{ // 아무런 이상이 없다.
            printf("%s\n",file_extesion);
        }
    }
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
    char file_type[20];
    while( p != NULL){
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
    return "nomake"; // 구현이 안 된것.
}