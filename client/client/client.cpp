// client.cpp: определяет точку входа для приложения.
//

#include "client.h"

using namespace std;
#define BUF_SIZE 1024
struct pollfd fds[2]; 

bool Proof_Enter(char* buffer) {									//Валидация строки
	unsigned short count = 0;
	for (int i = 0; i < strlen(buffer); i++) {
		if (buffer[i] == ' ' || buffer[i] == '#') {
			count++;
		}
	}
	if (count != 1) {
		return false;
	}
	return true;
}

void Proof_Conect(int conect, string func) {						//обработка ошибок при подключении к серверу
	if (conect < 0) {
		const char* error = func.c_str();
		perror(error);
		exit(1);
	}
}
void ReEnter_Message(int client,char* buffer) {						//функция для переписывания строчки если она не валидна
	while (!Proof_Enter(buffer)) {
		cout << "You entered the dont valid line" << endl;
		cin.getline(buffer, BUF_SIZE);
	}
}
void Wait_Message() {												//функция для ожидания сообщения от сервера
	while (!(fds[0].revents & POLLIN)) {
		poll(fds, 2, 0);
		if (fds[1].revents & POLLIN) {
			cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			cout << "wait answer server" << endl;
			fds[1].revents = 0;
		}
	}
}
void Send_Message(int client) {										//функция для общения с сервером
	char* buffer = new char[BUF_SIZE];
	strcpy(buffer, "file not found");
	
	while (!strcmp(buffer, "file not found")) {						//Будем вводить путь и имя файла, пока не найдем файл
		memset(buffer, 0, BUF_SIZE);
		cin.getline(buffer, BUF_SIZE);								// введем имя файла и слово которое ищем
		ReEnter_Message(client, buffer);							//Проверим валидность введенной строчки и если она не валидна перезапишем
		short sen = send(client, buffer, BUF_SIZE, MSG_NOSIGNAL);	//отправка имени на сервер
		if (sen < 0 || !strcmp(buffer,"#")) {						//если ошибка отправки или мы ввели # то программа завершается
			cout << "Server break connect" << endl;
			break;
		}
		memset(buffer, 0, BUF_SIZE);
		int ret = poll(fds, 2, 0);									// слушаеи файловые дескрипторы
		if (ret == -1 ) {											//если ошибка
			cout << "Erro accept data" << endl;
			exit(1);
		}
		else if (ret > 0) {											//если пришло сообщение от сервера или во время ожидания пользователь ввел команду в терминал
			if (fds[0].revents & POLLIN) {							//пришло сообщение
				recv(client, buffer, BUF_SIZE, MSG_DONTWAIT);
				fds[0].revents = 0;
			}
			else {													//пользователь ввел команду
				Wait_Message();										//ожидание сообщения
				recv(client, buffer, BUF_SIZE, MSG_DONTWAIT);
				fds[0].revents = 0;
			}
		}
		else{														//если сообщение от сервера не приходило
			Wait_Message(); 
			recv(client, buffer, BUF_SIZE, MSG_DONTWAIT);
			fds[0].revents = 0;
		}

		cout << buffer << endl;
	}
}
int main(int argc, char* argv[])
{
	if (argc != 3) {													//Проверка количества аргументов переданное через терминал
		cout << "incorrect number of arguments entered" << endl;
		exit(1);
	}
	int client;
	struct addrinfo server_address, * res;
	struct sockaddr_storage their_addr;
	

	cout << "If you want exit, enter  #" << endl;
	memset(&server_address, 0, sizeof server_address);

	server_address.ai_socktype = SOCK_STREAM;
	server_address.ai_family = AF_INET;
	server_address.ai_protocol = IPPROTO_TCP;
	getaddrinfo(argv[1], argv[2], &server_address, &res);
	client = socket(PF_INET, SOCK_STREAM, 0);
	Proof_Conect(client, "socket");

	fds[0].fd = client;
	fds[1].fd = 0;
	fds[1].events = POLLIN;
	fds[0].events = POLLIN;

	short conec = connect(client, res->ai_addr, res->ai_addrlen);		//Подключение к серверу
	Proof_Conect(conec, "connect");
	auto Chat_Server = async(launch::deferred, Send_Message, client);	//Асинхронная функция для работы с сервером
	Chat_Server.wait();
	return 0;
}