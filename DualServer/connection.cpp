#include "connection.h"

// CONSTRUCTOR
connection::connection(int port) : port(port) {
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(this->port);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    // SETUP CONNECTION PARAMETERS
    signal(SIGPIPE, SIG_IGN);
    /*
    int flags = fcntl(client_fd, F_GETFL, 0);
    fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);
    flags = fcntl(listener_fd, F_GETFL, 0);
    fcntl(listener_fd, F_SETFL, flags | O_NONBLOCK);
    */
    comm_timeout.tv_sec = 0;
    comm_timeout.tv_usec = 10;
    conn_timeout.tv_sec = 2;
    conn_timeout.tv_usec = 0;

    //initialize the buffers
    bzero(send_buffer, BUFFER_SIZE);
    bzero(read_buffer, BUFFER_SIZE);
}

// DESTRUCTOR
connection::~connection() {
    this->disconnect();
}

// CREATE AND BIND SOCKET
ssize_t connection::open() {
    listener_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener_fd < 0) return ERR_SOCK_CREATE;

    if (bind(listener_fd, (struct sockaddr *) &serv_addr,
                sizeof(serv_addr)) < 0) return ERR_SOCK_BIND;

    listen(listener_fd, 1);
    return SUCCESS;
}

// OPEN AND WAIT FOR CLIENT CONNECTION
ssize_t connection::connect() {
    FD_ZERO(&ready_fds);
    FD_SET(listener_fd, &ready_fds);
    if(select(listener_fd + 1, &ready_fds, NULL, NULL, &conn_timeout) < 0) {
        return ERR_SOCK_CONNECT;
    }

    if (FD_ISSET(listener_fd, &ready_fds)) {
        client_fd = accept(listener_fd, (struct sockaddr *) &cli_addr, &sin_size);
        return (client_fd < 0) ? ERR_SOCK_CONNECT : SUCCESS;
    }

    return ERR_NO_CONN;
}

// CLOSE CLIENT AND LISTENER SOCKETS
void connection::disconnect() {
    close(client_fd);
    close(listener_fd);
}

// FREE CLIENT SOCKET FOR REUSE
void connection::free() {
    close(client_fd);
}

// SEND DATA TO CLIENT
ssize_t connection::send(size_t size) {
    FD_ZERO(&ready_fds);
    FD_SET(client_fd, &ready_fds);
    if(select(client_fd + 1, NULL, &ready_fds, NULL, &comm_timeout) < 0) {
        return ERR_SEND;
    }
    if (FD_ISSET(client_fd, &ready_fds)) {
        return write(client_fd, send_buffer, size);
    }
    return ERR_SEND;
}

// RECEIVE DATA FROM CLIENT
ssize_t connection::read() {
    FD_ZERO(&ready_fds);
    FD_SET(client_fd, &ready_fds);
    if(select(client_fd + 1, &ready_fds, NULL, NULL, &comm_timeout) < 0) {
        return ERR_READ;
    }
    if (FD_ISSET(client_fd, &ready_fds)) {
        return recv(client_fd, read_buffer, sizeof(read_buffer), 0);
    }
    return NO_READ_MSG;
}

// GET SEND AND READ BUFFERS
uint8_t* connection::get_send_buffer() {
    return send_buffer;
}

uint8_t* connection::get_read_buffer() {
    return read_buffer;
}

// CONVERT ERROR MESSAGE ID TO STRING
const std::string connection::error_message(size_t status_code) {
    switch (status_code) {
        case ERR_SOCK_CREATE:
            return "Error Creating Socket";
        case ERR_SOCK_BIND:
            return "Error Binding Socket";
        case ERR_SOCK_CONNECT:
            return "Error Accepting Connection From Client";
        case ERR_NO_CONN:
            return "No Client To Accept Connection From";
        case ERR_SEND:
            return "Error Sending Message";
        case ERR_READ:
            return "Error Reading Message";
        case NO_READ_MSG:
            return "No Data Available for Reading";
        default:
            return "Error";
    }
}
