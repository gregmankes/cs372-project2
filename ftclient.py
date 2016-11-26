from socket import *
import sys

def check_args():
    if len(sys.argv) < 5 or len(sys.argv) > 6:
        print "Invalid number of args"
        exit(1)
    # elif (sys.argv[1] != "flip1" and sys.argv[1] != "flip2" and sys.argv[1] != "flip3"):
    #     print "Invalid server name"
    #     exit(1)
    elif (int(sys.argv[2]) > 65535 or int(sys.argv[2]) < 1024):
        print "Invalid control port number"
        exit(1)
    elif (sys.argv[3] != "-l" and sys.argv[3] != "-g"):
        print "Invalid option"
        exit(1)
    elif (sys.argv[3] == "-l" and (int(sys.argv[4]) > 65535 or int(sys.argv[4]) < 1024)):
        print "Invalid control port number"
        exit(1)
    elif (sys.argv[3] == "-g" and (int(sys.argv[5]) > 65535 or int(sys.argv[5]) < 1024)):
        print "Invalid control port number"
        exit(1)

def setup_data_socket():
    if sys.argv[3] == "-l":
        portarg = 4
    elif sys.argv[3] == "-g":
        portarg = 5
    serverport = int(sys.argv[portarg])
    serversocket = socket(AF_INET, SOCK_STREAM)
    serversocket.bind(('', serverport))
    serversocket.listen(1)
    data_socket, addr = serversocket.accept()
    return data_socket

def get_file_list(data_socket):
    filename = data_socket.recv(100)
    while filename != "done":
        print filename
        filename = data_socket.recv(100)

def get_file(data_socket):
    f = open(sys.argv[4],"w")
    buff = data_socket.recv(1000)
    while "__done__" not in buff:
        f.write(buff)
        buff = data_socket.recv(1000)

def get_my_ip():
    s = socket(AF_INET, SOCK_DGRAM)
    s.connect(("8.8.8.8", 80))
    return s.getsockname()[0]

def exchange_information(clientsocket):
    if sys.argv[3] == "-l":
        print "Requesting file list"
        portnum = 4
    elif sys.argv[3] == "-g":
        print "Reqesting file {}".format(sys.argv[4])
        portnum = 5
    # send port to send on
    clientsocket.send(sys.argv[portnum])
    # send command
    clientsocket.send(sys.argv[3][1:])
    # send my ip
    clientsocket.send(get_my_ip())
    response = clientsocket.recv(1024)
    if response == "bad":
        print "Server received an invalid command"
        exit(1)
    if sys.argv[3] == "-g":
        clientsocket.send(sys.argv[4])
        response = clientsocket.recv(1024)
        if response != "File found":
            print "Client responded with 'File not found message'"
            return
    data_socket = setup_data_socket()
    #begin to receive data
    if sys.argv[3] == "-l":
        get_file_list(data_socket)
    elif(sys.argv[3] == "-g"):
        get_file(data_socket)
    # close the socket at the end
    data_socket.close()


def connect_to_server():
    #servername = sys.argv[1]+".engr.oregonstate.edu"
    servername = sys.argv[1]
    serverport = int(sys.argv[2])
    clientsocket = socket(AF_INET,SOCK_STREAM)
    clientsocket.connect((servername, serverport))
    return clientsocket

if __name__ == "__main__":
    check_args()
    clientsocket = connect_to_server()
    exchange_information(clientsocket)
