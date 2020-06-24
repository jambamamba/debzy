#include "TcpServer.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <dirent.h>

void mylog(const char *fmt, ...)
{
    char tbuffer [128];
    time_t t = time(0);
    tm * timeinfo = localtime ( &t );
    strftime (tbuffer, 80,"%y-%m-%d %H:%M:%S", timeinfo);

    char buffer1[1024];
    char buffer2[1024+128];
    va_list args;
    va_start(args, fmt);
    vsprintf (buffer1, fmt, args);
    sprintf(buffer2, "%s - %s\n", tbuffer, buffer1);

    FILE* fp = fopen("/tmp/log.txt", "a+t");
    fwrite(buffer2, strlen(buffer2), 1, fp);
    fclose(fp);

    va_end(args);
}


#include <string>
#include <functional>
namespace {
void processFilesInDir(const std::string &path,
                       std::function<void (const std::string &)>cb)
{
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir (path.c_str())) == NULL)
    {
        return;
    }
    /* print all the files and directories within directory */
    while ((ent = readdir (dir)) != NULL)
    {
        if(strcmp(ent->d_name, "..") == 0 ||
                strcmp(ent->d_name, ".") == 0)
        {
            continue;
        }
        std::string buff(path);
        buff.append("/").append(ent->d_name);

        processFilesInDir(buff, cb);

        mylog ("%s", buff.c_str());
        lchown(buff.c_str(), 0, 0);
        cb("chown root:root " + buff + "\n");
    }
    mylog ("%s", path.c_str());
    lchown(path.c_str(), 0, 0);
    cb("chown root:root " + path + "\n");

    closedir (dir);
}
void handleCmd(const std::string &&msg,
               std::function<void (const std::string &)>cb)
{
    std::size_t found = msg.find(':');
    if (found == std::string::npos) { return; }

    std::string name = msg.substr(0, found);
    std::string value = msg.substr(found + 1, std::string::npos);

    mylog("name: %s, value: %s", name.c_str(), value.c_str());

    if(name.compare("chown") == 0)
    {
        processFilesInDir(value, cb);
        mylog("ran cmd");
    }
    if(name.compare("quit") == 0)
    {
        cb("quit");
    }
}
}//namespace

TcpServer::TcpServer()
{

}

void TcpServer::run() const
{
    mylog("run");

    struct sockaddr_in sa;
    int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (SocketFD == -1) {
      mylog("cannot create socket");
      exit(EXIT_FAILURE);
    }

    memset(&sa, 0, sizeof sa);

    sa.sin_family = AF_INET;
    const int DEFAULT_PORT = 55555;
    sa.sin_port = htons(DEFAULT_PORT);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(SocketFD,(struct sockaddr *)&sa, sizeof sa) == -1) {
      mylog("bind failed");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }

    if (listen(SocketFD, 10) == -1) {
      mylog("listen failed");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }

    for (;;) {
      int ConnectFD = accept(SocketFD, NULL, NULL);

      if (0 > ConnectFD) {
        mylog("accept failed");
        close(SocketFD);
        exit(EXIT_FAILURE);
      }
      mylog("connected");

      while(true){
          char buff[4096] = {0};
          // perform read write operations ...
          read(ConnectFD, buff, sizeof(buff));
          mylog("read: %s", buff);
          if(strncmp(buff, "quit", 4) == 0) { break; }
          handleCmd(buff, [ConnectFD](const std::string &msg){
              write(ConnectFD, msg.c_str(), msg.size());
          });
          write(ConnectFD, "done", 4);
      }
      if (shutdown(ConnectFD, SHUT_RDWR) == -1) {
        mylog("shutdown failed");
        close(ConnectFD);
        close(SocketFD);
        exit(EXIT_FAILURE);
      }
      write(ConnectFD, "closed", 6);
      close(ConnectFD);
      mylog("closed connection");
      break;
    }

    close(SocketFD);
    mylog("end server");
}
