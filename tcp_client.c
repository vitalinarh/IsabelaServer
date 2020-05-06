//USER MODE: 
//COMPILING: gcc tcp_client.c -o exeCliente
//EXECUTING: ./exeCliente 

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <time.h> 
#include <sys/shm.h>

#define DADOS 10
#define BUF_SIZE 1024
#define BUFF_BUFFER 5000
#define SERVER_PORT "9220"
#define SUB_PORT "9221"


//VARIAVEIS
char clienteID[128];
int subscription; //0-False ou 1-True
int fd, fd1;
char buffer[BUF_SIZE];

typedef struct shm_subs{
	int subCallsduration;
	int subCallsmade;
	int subCallsmissed;
	int subCallsreceived;
	int subSMSreceived;
	int subSMSsent;
}shm_subs;

//shared memory
shm_subs* shm;
int shmid;

void erro(char *msg);

void notification(){
	char dados[BUF_SIZE];
	char media[BUF_SIZE];
	time_t timer;
    char t[26];
    struct tm* tm_info;

  	while(1){
  	time(&timer);
    tm_info = localtime(&timer);
    strftime(t, 26, "%H:%M ", tm_info);
    read(fd1, buffer, BUF_SIZE);
    if (strcmp("NOTIFICATION", buffer)==0){
        read(fd1, dados, BUF_SIZE);
        read(fd1, media, BUF_SIZE);
        if(strcmp(dados, "duracao das chamadas")==0){
        	if(shm->subCallsduration==1){
        		printf("\n%s-", t);
        		printf("|Notificação|: A média da duração das chamadas foi alterada para %s\n", media);
        	}
        }
        if(strcmp(dados, "chamadas feitas")==0){
        	if(shm->subCallsmade==1){
        		printf("\n%s-", t);
        		printf("|Notificação|: A média das chamadas feitas foi alterada para %s\n", media);
        	}
        }
        if(strcmp(dados, "chamadas perdidas")==0){
        	if(shm->subCallsmissed==1){
        		printf("\n%s-", t);
        		printf("|Notificação|: A média das chamadas perdidas foi alterada para %s\n", media);
        	}
        }
        if(strcmp(dados, "chamadas recebidas")==0){
        	if(shm->subCallsreceived==1){
        		printf("\n%s-", t);
        		printf("|Notificação|: A média das chamadas recebidas foi alterada para %s\n", media);
        	}
        }
        if(strcmp(dados, "sms recebidas")==0){
        	if(shm->subSMSreceived==1){
        		printf("\n%s-", t);
        		printf("|Notificação|: A média de sms recebidas foi alterada para %s\n", media);
        	}
        }
        if(strcmp(dados, "sms enviadas")==0){
        	if(shm->subSMSsent==1){
        		printf("\n%s-", t);
        		printf("|Notificação|: A média de sms enviadas foi alterada para %s\n", media);
        	}
        }    
    }
  }
}

//CLEANUP
int terminus() {
	shmdt(shm); //detach da shared memory principal
	shmctl(shmid,IPC_RMID,NULL); //remove da detached shared memory
	close(fd); //fechar o primeiro socket
	close(fd1); //fechar o segundo socket
	exit(0);
	return 0;
}

int menu(){

	system("clear");
	int n;
  	char opcao[5];
  	char opcao2[5];
  	char buffer2[BUFF_BUFFER];	

//MENU 1
  	printf("\t\t\t\t       ISABELA\n");
  	printf("\t\t\t\t\tMENU\n");
  	printf("\t\t\t ________________________________\n");
	printf("\t\t\t|                                |\n");
	printf("\t\t\t|1 -> VISUALIZAR DADOS PESSOAIS  |\n");
	printf("\t\t\t|                                |\n");
	printf("\t\t\t|2 -> VISUALIZAR ESTATÍSTICOS    |\n");
	printf("\t\t\t|                                |\n");
	printf("\t\t\t|3 -> SUBSCREVER                 |\n");
	printf("\t\t\t|                                |\n");
	printf("\t\t\t|4 -> CANCELAR SUBSCRIÇÃO        |\n");
	printf("\t\t\t|________________________________|\n");
	printf(" \n\t\t\t5 - EXIT\n                     ");
	printf("\nOPÇÃO:");
	scanf("%s",buffer);

	while(1>atoi(buffer) && 5>atoi(buffer)){
		printf("\nOpção inválida. Tente novamente\n");
		sleep(2);
		system("cls");
		menu();
	}

	write(fd, buffer, BUF_SIZE);

	switch(atoi(buffer)){
		case(1):				
			system("clear");
			//leitura da informaçao pessoal do cliente atual enviada pelo servidor
			read(fd, buffer2, BUFF_BUFFER);
			printf("%s\n", buffer2);
			sleep(5);
			menu();
		case(2):
			system("clear");
			//leitura da informaçao estatística dos clientes enviada pelo servidor
			read(fd, buffer2, BUFF_BUFFER);
			printf("%s\n",buffer2);
			sleep(5);
			menu();
		case(3):
			//menu para as subscrições
			system("clear");
			printf("\t\t\t\t  DESEJA\n");
  			printf("\t\t\t\tSUBSCREVER\n");
  			printf("\t\t\t ________________________________\n");
			printf("\t\t\t|                                |\n");
			printf("\t\t\t|1 -> DURAÇÃO DA CHAMADA  	 |\n");
			printf("\t\t\t|                                |\n");
			printf("\t\t\t|2 -> CHAMADAS FEITAS            |\n");
			printf("\t\t\t|                                |\n");
			printf("\t\t\t|3 -> CHAMADAS PERDIDAS          |\n");
			printf("\t\t\t|                                |\n");
			printf("\t\t\t|4 -> CHAMADAS RECEBIDAS         |\n");
			printf("\t\t\t|                                |\n");
			printf("\t\t\t|5 -> SMS RECEBIDAS              |\n");
			printf("\t\t\t|                                |\n");
			printf("\t\t\t|6 -> SMS ENVIADAS               |\n");
			printf("\t\t\t|                                |\n");
			printf("\t\t\t|7 -> TUDO                       |\n");
			printf("\t\t\t|________________________________|\n");
			printf(" \n\t\t\t8 - EXIT\n                     ");
			printf("\nOPÇÃO:");
			scanf("%s",opcao2);
		
			if(strcmp(opcao2,"1") == 0){
				shm->subCallsduration = 1;
				sleep(1);
				printf("Subscrição feita com sucesso!\n");
				sleep(1);
			}
			if(strcmp(opcao2,"2") == 0){
				shm->subCallsmade = 1;
				sleep(1);
				printf("Subscrição feita com sucesso!\n");
				sleep(1);
			}
			if(strcmp(opcao2,"3") == 0){
				shm->subCallsmissed = 1;
				sleep(1);
				printf("Subscrição feita com sucesso!\n");
				sleep(1);
			}
			if(strcmp(opcao2,"4") == 0){
				shm->subCallsreceived = 1;
				sleep(1);					
				printf("Subscrição feita com sucesso!\n");
				sleep(1);
			}
			if(strcmp(opcao2,"5") == 0){
				shm->subSMSreceived = 1;
				sleep(1);
				printf("Subscrição feita com sucesso!\n");
				sleep(1);
			}
			if(strcmp(opcao2,"6") == 0){
				shm->subSMSsent = 1;
				sleep(1);
				printf("Subscrição feita com sucesso!\n");
				sleep(1);
			}
			if(strcmp(opcao2,"7") == 0){
				shm->subCallsduration = 1;	
				shm->subCallsmade = 1;
				shm->subCallsmissed = 1;
				shm->subCallsreceived = 1;
				shm->subSMSreceived = 1;
				shm->subSMSsent = 1;
				sleep(1);
				printf("Subscrição feita com sucesso!\n");
				sleep(1);
			}
			if(strcmp(opcao2, "8")){
				menu();
				break;
			}
			sleep(2);
			menu();

		case(4):
			//menu plara cancelar as subscrições
			system("clear");
			printf("\t\t\t\t     DESEJA\n");
  			printf("\t\t\t\tCANCELAR SUBSCRIÇÃO\n");
  			printf("\t\t\t ________________________________\n");
			printf("\t\t\t|                                |\n");
			printf("\t\t\t|1 -> DURAÇÃO DA CHAMADA  	   |\n");
			printf("\t\t\t|                                |\n");
			printf("\t\t\t|2 -> CHAMADAS FEITAS            |\n");
			printf("\t\t\t|                                |\n");
			printf("\t\t\t|3 -> CHAMADAS PERDIDAS          |\n");
			printf("\t\t\t|                                |\n");
			printf("\t\t\t|4 -> CHAMADAS RECEBIDAS         |\n");
			printf("\t\t\t|                                |\n");
			printf("\t\t\t|5 -> SMS RECEBIDAS              |\n");
			printf("\t\t\t|                                |\n");
			printf("\t\t\t|6 -> SMS ENVIADAS               |\n");
			printf("\t\t\t|                                |\n");
			printf("\t\t\t|7 -> TUDO                       |\n");
			printf("\t\t\t|________________________________|\n");
			printf(" \n\t\t\t8 - EXIT\n                     ");
			printf("\nOPÇÃO:");
			scanf("%s",opcao2);
			
			if(strcmp(opcao2,"1") == 0){
				shm->subCallsduration = 0;
				sleep(1);
				printf("Cancelamento da subscrição feito com sucesso!\n");
				sleep(2);
			}
			if(strcmp(opcao2,"2") == 0){
				shm->subCallsmade = 0;
				sleep(2);
				printf("Cancelamento da subscrição feito com sucesso!\n");
				sleep(2);
			}

			if(strcmp(opcao2,"3") == 0){
				shm->subCallsmissed = 0;
				sleep(2);
				printf("Cancelamento da subscrição feito com sucesso!\n");
				sleep(2);
			}
				if(strcmp(opcao2,"4") == 0){
					shm->subCallsreceived = 0;
					sleep(1);
					printf("Cancelamento da subscrição feito com sucesso!\n");
					sleep(2);
				}
				if(strcmp(opcao2,"5") == 0){
					shm->subSMSreceived = 0;
					sleep(1);
					printf("Cancelamento da subscricão feito com sucesso!\n");
					sleep(2);
				}
				if(strcmp(opcao2,"6") == 0){
					shm->subSMSsent = 0;
					sleep(1);
					printf("Cancelamento da subscrição feito com sucesso!\n");
					sleep(2);
				}
				if(strcmp(opcao2,"7") == 0){
					shm->subCallsduration = 0;	
					shm->subCallsmade = 0;
					shm->subCallsmissed = 0;
					shm->subCallsreceived = 0;
					shm->subSMSreceived = 0;
					shm->subSMSsent = 0;
					sleep(1);
					printf("Cancelamento da subscrição feito com sucesso!\n");
					sleep(2);
				}
				if(strcmp(opcao2,"8") == 0){
					menu();
					break;
				}
				sleep(3);
				menu();
			case(5):

				system("clear");
				printf("Exiting...\n");
				terminus();
				return 0;

			default:
				system("clear");
				menu();
			}	
}

int main() { //int argc, char *argv[]

  	char endServer[100];
  	struct sockaddr_in addr;
  	struct hostent *hostPtr;
  	char opcao[5];
  	char opcao2[5];
	char buffer2[BUFF_BUFFER];


	//memória partilhada para as subscrições
	shmid = shmget(IPC_PRIVATE,sizeof(shm_subs),IPC_CREAT|0777);
	shm = (shm_subs*)shmat(shmid,NULL,0);

  	strcpy(endServer,"0");
  
 	if((hostPtr = gethostbyname(endServer)) == 0){
  		erro("Não consegui obter endereço");
 	}

  	bzero((void *) &addr, sizeof(addr));
  	addr.sin_family = AF_INET;
  	addr.sin_addr.s_addr = ((struct in_addr *)(hostPtr->h_addr))->s_addr;

	//PRIMEIRO SOCKET
 	addr.sin_port = htons((short) atoi(SERVER_PORT)); //checkar se isto é int ou string

  	if((fd = socket(AF_INET,SOCK_STREAM,0)) == -1)
		erro("socket");
  	if( connect(fd,(struct sockaddr *)&addr,sizeof (addr)) < 0)
	 erro("Connect");


//SEGUNDO SOCKET
  addr.sin_port = htons((short) atoi(SUB_PORT)); 

  if((fd1 = socket(AF_INET,SOCK_STREAM,0)) == -1)
   erro("socket");
  if( connect(fd1,(struct sockaddr *)&addr,sizeof (addr)) < 0)
   erro("Connect");

 	if (fork()==0){
  		notification();
 	}

 	//Bem vindo chefe
	read(fd, buffer, BUF_SIZE);
	printf("%s", buffer);
  	sleep(1);

	//INSERIR IDS VALIDOS
	printf("Exemplo de Ids Válidos: ISABELA_EXAMPLE_1, ISABELA_EXAMPLE_2\nINSERIR ID: ");
	scanf("%s",buffer);
	//enviar id para validaçao
	write(fd, buffer, BUF_SIZE);
	//leitura da validacao		
	read(fd, buffer, BUF_SIZE);
	printf("%s",buffer);
	if(strcmp(buffer,"Cliente Invalido\n")==0){
		exit(0); 
	}

	sleep(3);

 	menu();
}

void erro(char *msg) {
	printf("Erro: %s\n", msg);
	exit(-1);
}

