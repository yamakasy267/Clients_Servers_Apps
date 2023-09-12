// Test_task.cpp: определяет точку входа для приложения.
//

#include "Test_task.h"


using namespace std;
#define BUF_SIZE 1024

void Proof_Conect(int conect,string func) {			//обработка ошибок при создании подключения
	if (conect < 0) {
		const char* error = func.c_str();
		perror(error);
		exit(1);
	}
}

char* Search(char* buffer) {						//Функция для поиска слова в файле
	string str(buffer, strlen(buffer));
	int ind = str.find(' ');
	vector<string> buf;
	buf.push_back(str.substr(0,ind));
	buf.push_back(str.substr(ind + 1));
	ifstream in(buf[1]);
	string line;
	int counts = 0;
	if (in.is_open()) {								//Если находим файл то начинаем считать количество вхождения слова
		while (in>>line) {
			if (line == buf[0]) {
				counts++;
			}
		}
	}
	else {											//Если не нашли файл, то так и запишем в буфер
		strcpy(buffer, "file not found");
		return buffer;
	}
	str = to_string(counts);
	strcpy(buffer,str.c_str());
	return buffer;
}
void Accses_Stream(int server) {					//Функция для общения с клиеннтом
	char* buffer = new char[BUF_SIZE];
	strcpy(buffer, "file not found");

	while (!strcmp(buffer,"file not found")) {		//Пока не найдем файл будем получать от клиента имя файла и искать
		//memset(buffer, 0, BUF_SIZE);
		recv(server, buffer, BUF_SIZE, 0);
		if (!strcmp(buffer, "#")) {					//Заканчиваем сеанс если клиент присла знак окончания сессии 
			return ;
		}
		cout << buffer << endl;
		buffer = Search(buffer);					//Ищем файл
		short sen = send(server, buffer, BUF_SIZE, MSG_NOSIGNAL);//Отправляем результат клиенту
		if (sen < 0) {											//Заканчиваем сеанс если клиент вышел из сессии
			return ;
		}
	}
}

int main(int argc, char* argv[])
{
	if (argc != 3) {											//Проверка количества аргументов переданное через терминал
		cout << "incorrect number of arguments entered" << endl;
		exit(1);
	}
	int client,server;
	struct addrinfo server_address,*res;
	struct sockaddr_storage their_addr;
	socklen_t addr_size;
	short rec;

	client = socket(PF_INET, SOCK_STREAM, 0);					//Сокет для общения с клиентом
	setsockopt(client, 0, SO_REUSEADDR, 0, 0);
	Proof_Conect(client, "socket");
	memset(&server_address, 0, sizeof server_address);

	server_address.ai_socktype = SOCK_STREAM;
	server_address.ai_family = AF_INET;
	server_address.ai_protocol = IPPROTO_TCP;
	getaddrinfo(argv[1], argv[2], &server_address, &res);

	addr_size = sizeof their_addr;
	rec = bind(client, res->ai_addr,res->ai_addrlen);
	Proof_Conect(rec, "bind");
	listen(client, 20);														//Начинаем слушать сокет
	int flag = 1;
	while (flag) {
		server = accept(client, (struct sockaddr*)&their_addr, &addr_size); //Ждем подключения клиента
 		Proof_Conect(server, "accept");
		auto threads = async(launch::async, Accses_Stream, server);			//Создаем новый поток для общения с клиентом

	}
	return 0;
}
