#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <dirent.h>

struct addrinfo * create_address_info(char * port){
	int status;
	struct addrinfo hints;
	struct addrinfo * res;
	
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if((status = getaddrinfo(NULL, port, &hints, &res)) != 0){
		fprintf(stderr,
				"getaddrinfo error: %s\nDid you enter the correct IP/Port?\n",
				gai_strerror(status));
		exit(1);
	}
	
	return res;
}

struct addrinfo * create_address_info_with_ip(char * ip_address, char * port){
	int status;
	struct addrinfo hints;
	struct addrinfo * res;
	
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	if((status = getaddrinfo(ip_address, port, &hints, &res)) != 0){
		fprintf(stderr,
				"getaddrinfo error: %s\nDid you enter the correct IP/Port?\n",
				gai_strerror(status));
		exit(1);
	}
	
	return res;
}

int create_socket(struct addrinfo * res){
	int sockfd;
	if ((sockfd = socket((struct addrinfo *)(res)->ai_family, res->ai_socktype, res->ai_protocol)) == -1){
		fprintf(stderr, "Error in creating socket\n");
		exit(1);
	}
	return sockfd;
}

void connect_socket(int sockfd, struct addrinfo * res){
	int status;
	if ((status = connect(sockfd, res->ai_addr, res->ai_addrlen)) == -1){
		fprintf(stderr, "Error in connecting socket\n");
		exit(1);
	}
}

void bind_socket(int sockfd, struct addrinfo * res){
	if (bind(sockfd, res->ai_addr, res->ai_addrlen) == -1) {
		close(sockfd);
		fprintf(stderr, "Error in binding socket\n");
		exit(1);
	}
}

void listen_socket(int sockfd){
	if(listen(sockfd, 5) == -1){
		close(sockfd);
		fprintf(stderr, "Error in listening on socket\n");
		exit(1);
	}
}

char ** create_string_array(int size){
	char ** array = malloc(size*sizeof(char *));
	for(int i = 0; i < size; i++){
		array[i] = malloc(100*sizeof(char));
		memset(array[i],0,sizeof(array[i]));
	}
	return array;
}

void delete_string_array(char ** array, int size){
	for (int i = 0; i < size; i++){
		free(array[i]);
	}
	free(array);
}

// make sure to reference http://stackoverflow.com/questions/4204666/how-to-list-files-in-a-directory-in-a-c-program
int get_available_files(char ** files){
	DIR * d;
	struct dirent * dir;
	d = opendir(".");
	int i = 0;
	if (d){
		while ((dir = readdir(d)) != NULL){
			strcpy(dir->d_name, files[i]);
			i++;
		}
		closedir(d);
	}
	return i;
}

int does_file_exist(char ** files, int num_files, char * filename){
	int file_exists = 0;
	for (int i = 0; i < num_files; i++){
		if(strcmp(files[i], filename) == 0){
			file_exists = 1;
		}
	}
	return file_exists;
}

//http://stackoverflow.com/questions/2014033/send-and-receive-a-file-in-socket-programming-in-linux-with-c-c-gcc-g
void send_file(char * ip_address, char * port, char * filename){
	// connect the data socket
	struct addrinfo * res = create_address_info_with_ip(ip_address, port);
	int data_socket = create_socket(res);
	connect_socket(data_socket, res);
	char buffer[1000];
	while (1) {
		// Read data into buffer.  We may not have enough to fill up buffer, so we
		// store how many bytes were actually read in bytes_read.
		int bytes_read = read(filename, buffer, sizeof(buffer));
		if (bytes_read == 0) // We're done reading from the file
			break;

		if (bytes_read < 0) {
			fprintf(stderr, "Error reading file\n");
		}

		// You need a loop for the write, because not all of the data may be written
		// in one call; write will return how many bytes were written. p keeps
		// track of where in the buffer we are, while we decrement bytes_read
		// to keep track of how many bytes are left to write.
		void *p = buffer;
		while (bytes_read > 0) {
			int bytes_written = write(data_socket, p, bytes_read);
			if (bytes_written <= 0) {
				fprintf(stderr, "Error writing to socket\n");
			}
			bytes_read -= bytes_written;
			p += bytes_written;
		}
		memset(buffer, 0, sizeof(buffer));
	}
	char * done_message = "done";
	send(data_socket, done_message, strlen(done_message),0);
	freeaddrinfo(res);
}

void send_directory(char * ip_address, char * port, char ** files, int num_files){
	// connect the data socket
	struct addrinfo * res = create_address_info_with_ip(ip_address, port);
	int data_socket = create_socket(res);
	connect_socket(data_socket, res);
	for (int i = 0; i < num_files; i++){
		send(data_socket, files[i], strlen(files[i]),0);
	}
	char * done_message = "done";
	send(data_socket, done_message, strlen(done_message),0);
	freeaddrinfo(res);
}

void handle_request(int new_fd, char * ip_address){
	printf("Incoming connection from %s\n", ip_address);
	// get the port number the client is expecting for a data connection
	char port[100];
	memset(port, 0, sizeof(port));
	recv(new_fd, port, sizeof(port)-1, 0);
	// get the command from the client
	char command[3];
	memset(command,0,sizeof(command));
	recv(new_fd, command, sizeof(command)-1, 0);
	char * ok_message = "ok";
	char * bad_message = "bad";
	if(strcmp(command,"l") == 0){
		// list files
		send(new_fd, ok_message, strlen(ok_message),0);
		printf("File list requested on port %s\n", port);
		char ** files = create_string_array(100);
		int num_files = get_available_files(files);
		send_directory(ip_address, port, files, num_files);
		delete_string_array(files,0);
	}
	else if(strcmp(command, "g") == 0){
		// get filename
		send(new_fd, ok_message, strlen(ok_message),0);
		char filename[100];
		memset(filename, 0, sizeof(filename));
		recv(new_fd, filename, sizeof(filename)-1,0);
		printf("File: %s requested on port %s\n", filename, port);
		char ** files = create_string_array(100);
		int num_files = get_available_files(files);
		int file_exists = does_file_exist(files, num_files, filename);
		if(file_exists){
			printf("File found, sending %s to client\n", filename);
			char * file_found = "File found";
			send(new_fd, file_found, strlen(file_found),0);
			send_file(ip_address, port, filename);
		}
		else{
			printf("File not found, sending error message to client\n");
			char * file_not_found = "File not found";
			send(new_fd, file_not_found, strlen(file_not_found), 0);
		}
		delete_string_array(files, 100);
	}
	else{
		send(new_fd, bad_message, strlen(bad_message), 0);
	}
}

void wait_for_connection(int sockfd){
	struct sockaddr_storage their_addr;
    socklen_t addr_size;
	int new_fd;
	while(1){
		addr_size = sizeof(their_addr);
		new_fd = accept(sockfd, (struct addrinfo *)&their_addr, &addr_size);
		if(new_fd == -1){
			// no incoming connection for now, keep waiting
			continue;
		}
		char ip_address[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, (struct sockaddr_in *)&their_addr, ip_address, INET_ADDRSTRLEN);
		handle_request(new_fd, ip_address);
		close(new_fd);
	}
}


int main(int argc, char *argv[]){
	if(argc != 2){
		fprintf(stderr, "Invalid number of arguments\n");
		exit(1);
	}
	printf("Server open on port %s\n", argv[1]);
	struct addrinfo * res = create_address_info(argv[1]);
	int sockfd = create_socket(res);
	bind_socket(sockfd, res);
	listen_socket(sockfd);
	wait_for_connection(sockfd);
	freeaddrinfo(res);
}

