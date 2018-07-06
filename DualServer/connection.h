#include <cstdint>
#include <cstring>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <string>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

class connection {
public:
    connection(int port);           // new connection object
    ~connection();                  // close connection and destroy
    ssize_t  open();                // open socket
    ssize_t  connect();             // accept client connection
    void     disconnect();          // close client and listener sockets
    void     free();                // free client connection
    ssize_t  read();                // read data into read_buffer
    ssize_t  send(size_t size);     // send data into send_buffer
    uint8_t* get_send_buffer();
    uint8_t* get_read_buffer();

    //static const size_t  BUFFER_SIZE = 1024;

private:
    // connection variables //
    int                 port;             // connection port
    int                 listener_fd;      // socket for listening
    int                 client_fd;        // client connection socket
    struct sockaddr_in  serv_addr;        // server address
    struct sockaddr_in  cli_addr;         // client address
    socklen_t           sin_size;
    fd_set              ready_fds;
    struct timeval      conn_timeout;
    struct timeval      comm_timeout;

    // read/write variables //
    ssize_t             nbytes;           // number of bytes read or sent
    uint8_t             send_buffer[BUFFER_SIZE];
    uint8_t             read_buffer[BUFFER_SIZE];

public:
    // state codes //
    static const ssize_t SUCCESS = 0;
    static const ssize_t ERR_SOCK_CREATE = -1;
    static const ssize_t ERR_SOCK_BIND = -2;
    static const ssize_t ERR_SOCK_CONNECT = -3;
    static const ssize_t ERR_NO_CONN = -4;
    static const ssize_t ERR_SEND = -5;
    static const ssize_t ERR_READ = -6;
    static const ssize_t NO_READ_MSG = -7;
    static const std::string  error_message(size_t status_code);
};
