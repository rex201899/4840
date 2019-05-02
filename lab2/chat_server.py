#!/usr/bin/env python 
import socket, select, struct, fcntl


# Chat server used for CSEE 4840 Lab2 @ Columbia University
# Jian Lu
# Columbia University SEAS'13
# jl3946@columbia.edu
#
# Modifications by Stephen A. Edwards

# Broadcast chat messages to all connected clients
def broadcast_data(sock, message):
    # Do not send the message back to whoever sent it
    for socket in CONNECTION_LIST:
        try:
            if socket != server_socket and socket != sock:
                socket.send(message)
        except:
            continue


if __name__ == "__main__":

    # List to keep track of socket descriptors
    CONNECTION_LIST = []
    RECV_BUFFER = 4096  # Advisable to keep it as an exponent of 2
    PORT = 42000
    # HOST = '160.39.248.141'
    HOST = '160.39.249.162'
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server_socket.bind((HOST, PORT))
    server_socket.listen(10)

    # Add server socket to the list of readable connections
    CONNECTION_LIST.append(server_socket)

    print "## Listening on " + HOST + ":" + str(PORT)

    while 1:

        # Get the list of sockets that are ready to be read
        read_sockets, _, _ = select.select(CONNECTION_LIST, [], [])

        for sock in read_sockets:

            # A new connection through our listening socket?
            if sock == server_socket:
                sockfd, addr = server_socket.accept()
                (clientip, clientport) = addr
                CONNECTION_LIST.append(sockfd)
                print "## New Connection %s:%s" % (clientip, clientport)

                sockfd.send("Welcome to the CSEE 4840 Lab2 chat server")
                broadcast_data(sockfd, "## %s has joined\n" % clientip)

            # A packet from a client
            else:
                try:
                    data = sock.recv(RECV_BUFFER)
                    (clientip, clientport) = sock.getpeername()

                    # If there is data, someone sent us something
                    if data:
                        message = "<%s> " % clientip + data
                        broadcast_data(sock, message)
                        print message.strip()

                        # No data: the client closed the connection
                    else:
                        broadcast_data(sock, "## %s has left\n" % clientip)
                        print "## Closed connection %s:%s" % addr
                        sock.close()
                        CONNECTION_LIST.remove(sock)
                except:
                    continue
server_socket.close()
