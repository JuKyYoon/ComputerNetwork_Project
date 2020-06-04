**Simple Web Server in C**
======================
 - Part1 : Web browser only see the HTTP request Message.
 - Part2 : Web browser can see the HTTP request Message, html, gif, jpg, pdf and download the mp3 file.


**Your Shell**
> make
> ./server {your port number}


----------

**Browser**

     http:\\localhost:{your port number}

or

    

    http:\\127.0.0.1:{your port number} 


----------

If you want to request the file, the server will response with the request.

    http:\\localhost:{your port number}/{file}
This web server only support the `html mp3 pdf gif jpg` Filename extension

