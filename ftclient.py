#!/bin/python
from socket import *
import sys

def check_args():
    # check to the amound of arguments
    if len(sys.argv) < 5 or len(sys.argv) > 6:
        print "Invalid number of args"
        exit(1)
    elif (sys.argv[1] != "flip1" and sys.argv[1] != "flip2" and sys.argv[1] != "flip3"):
        # check to make sure that we are on flip1-3
        print "Invalid server name"
        exit(1)
    elif (int(sys.argv[2]) > 65535 or int(sys.argv[2]) < 1024):
        # check to ensure a valid port number
        print "Invalid control port number"
        exit(1)
    elif (sys.argv[3] != "-l" and sys.argv[3] != "-g"):
        #check that the option is correct
        print "Invalid option"
        exit(1)
    elif (sys.argv[3] == "-l" and (int(sys.argv[4]) > 65535 or int(sys.argv[4]) < 1024)):
        # check valid port number for when there are 5 args
        print "Invalid control port number"
        exit(1)
    elif (sys.argv[3] == "-g" and (int(sys.argv[5]) > 65535 or int(sys.argv[5]) < 1024)):
        # check valid port number for when there are 6 args
        print "Invalid control port number"
        exit(1)

def setup_data_socket():
    # if there are 5 args, port arg is 4, otherwise 5
    if sys.argv[3] == "-l":
        portarg = 4
    elif sys.argv[3] == "-g":
        portarg = 5
    # create the server socket and accept a connection
    # as data socket
    serverport = int(sys.argv[portarg])
    serversocket = socket(AF_INET, SOCK_STREAM)
    serversocket.bind(('', serverport))
    serversocket.listen(1)
    data_socket, addr = serversocket.accept()
    return data_socket

def get_file_list(data_socket):
    # get the list of files
    # grab the first filename
    filename = data_socket.recv(100)
    # while we havent reached the end
    while filename != "done":
        # keep printing file names
        print filename
        filename = data_socket.recv(100)

def get_file(data_socket):
    # open a file for writing
    f = open(sys.argv[4],"w")
    # grab the bufferred output from the server
    buff = data_socket.recv(1000)
    # while we havent reached the end, write to the file
    while "__done__" not in buff:
        f.write(buff)
        buff = data_socket.recv(1000)

def get_my_ip():
    # helper method to get IP address
    # this is used since I was having trouble gettin ip in C
    # source below
    # http://stackoverflow.com/questions/24196932/how-can-i-get-the-ip-address-of-eth0-in-python
    s = socket(AF_INET, SOCK_DGRAM)
    s.connect(("8.8.8.8", 80))
    return s.getsockname()[0]

def exchange_information(clientsocket):
    # if there are 5 arguments the port num is 4
    # else it is five. print what the program is doing
    if sys.argv[3] == "-l":
        print "Requesting file list"
        portnum = 4
    elif sys.argv[3] == "-g":
        print "Reqesting file {}".format(sys.argv[4])
        portnum = 5
    # send port to send on
    clientsocket.send(sys.argv[portnum])
    # this fixes weird error where python sends all pieces on same buffer
    clientsocket.recv(1024)
    # send command
    if sys.argv[3] == "-l":
        clientsocket.send("l")
    else:
        clientsocket.send("g")
    # this fixes weird error where python sends all pieces on same buffer
    clientsocket.recv(1024)
    # send my ip
    clientsocket.send(get_my_ip())
    response = clientsocket.recv(1024)
    # if the server didn't recognize the command, say so
    if response == "bad":
        print "Server received an invalid command"
        exit(1)
    # if we are grabbing a file, check if it exists
    if sys.argv[3] == "-g":
        clientsocket.send(sys.argv[4])
        response = clientsocket.recv(1024)
        if response != "File found":
            print "Client responded with 'File not found message'"
            return
    # set up the data socket
    data_socket = setup_data_socket()
    #begin to receive data
    # if we are listing dir contents, call that method
    if sys.argv[3] == "-l":
        get_file_list(data_socket)
    elif(sys.argv[3] == "-g"): # otherwise get the file
        get_file(data_socket)
    # close the socket at the end
    data_socket.close()


def connect_to_server():
    # append the flip1-3 arg with the rest of the URL
    servername = sys.argv[1]+".engr.oregonstate.edu"
    # transform the port into an int
    serverport = int(sys.argv[2])
    # create a socket
    clientsocket = socket(AF_INET,SOCK_STREAM)
    # connect the socket
    clientsocket.connect((servername, serverport))
    return clientsocket

if __name__ == "__main__":
    # check the args
    check_args()
    # create a socket
    clientsocket = connect_to_server()
    # communicate with server
    exchange_information(clientsocket)
