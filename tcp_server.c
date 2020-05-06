//USER MODE: 
//COMPILING: gcc tcp_server.c -o exeServer -lcurl -ljson-c
//EXECUTING: ./exeServer

//LIVRARIAS
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <curl/curl.h>
#include <json-c/json.h>


//DEFINES
#define SERVER_PORT 9220
#define SUB_PORT 9221
#define BUF_SIZE 512
#define BUFF_BUFFER 5000

void media();

//ESTRUTURAS
typedef struct estatistica{ 
    float callsduration;	
    float callsmade;
    float callsmissed;
    float callsreceived;
    float smsreceived;
    float smssent;
}estatistica;

typedef struct sub{     
	int subCallsduration;
	int subCallsmade;
	int subCallsmissed;
	int subCallsreceived;
	int subSMSreceived;
	int subSMSsent;
}sub;


//VARIAVEIS GLOBAIS
int fd, fd1, client, client1;
int client_addr_size;
struct json_object *jobj_array, *jobj_obj;
struct estatistica *estatisticas;
struct estatistica *estatisticasTemp;
struct sub *subs;
struct sockaddr_in addr, client_addr;

struct string {
	char *ptr;
	size_t len;
};


void erro(char *msg){
	printf("Erro: %s\n", msg);
	exit(-1);
}

//CLEANUP
void terminus(int signum) {

	close(client);
	close(client1);
	close(fd);
	close(fd1);
	exit(0);
}

//FUNCOES DADAS PARA TRATAR ISABELA
//Write function to write the payload response in the string structure
size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s)
{
	size_t new_len = s->len + size*nmemb;
	s->ptr = realloc(s->ptr, new_len + 1);
	if (s->ptr == NULL) {
		fprintf(stderr, "realloc() failed\n");
		exit(EXIT_FAILURE);
	}
	memcpy(s->ptr + s->len, ptr, size*nmemb);
	s->ptr[new_len] = '\0';
	s->len = new_len;

	return size*nmemb;
}

//Initilize the payload string
void init_string(struct string *s) {
	s->len = 0;
	s->ptr = malloc(s->len + 1);
	if (s->ptr == NULL) {
		fprintf(stderr, "malloc() failed\n");
		exit(EXIT_FAILURE);
	}
	s->ptr[0] = '\0';
}

//Get the Data from the API and return a JSON Object
struct json_object *get_student_data()
{
	struct string s;
	struct json_object *jobj;

	//Intialize the CURL request
	CURL *hnd = curl_easy_init();

	//Initilize the char array (string)
	init_string(&s);

	curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "GET");
	//To run on department network uncomment this request and comment the other
	//curl_easy_setopt(hnd, CURLOPT_URL, "http://10.3.4.75:9014/v2/entities?options=keyValues&type=student&attrs=activity,calls_duration,calls_made,calls_missed,calls_received,department,location,sms_received,sms_sent&limit=1000");
        //To run from outside
	curl_easy_setopt(hnd, CURLOPT_URL, "http://socialiteorion2.dei.uc.pt:9014/v2/entities?options=keyValues&type=student&attrs=activity,calls_duration,calls_made,calls_missed,calls_received,department,location,sms_received,sms_sent&limit=1000");

	//Add headers
	struct curl_slist *headers = NULL;
	headers = curl_slist_append(headers, "cache-control: no-cache");
	headers = curl_slist_append(headers, "fiware-servicepath: /");
	headers = curl_slist_append(headers, "fiware-service: socialite");

	//Set some options
	curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, writefunc); //Give the write function here
	curl_easy_setopt(hnd, CURLOPT_WRITEDATA, &s); //Give the char array address here

	//Perform the request
	CURLcode ret = curl_easy_perform(hnd);
	if (ret != CURLE_OK){
		fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(ret));

		/*jobj will return empty object*/
		jobj = json_tokener_parse(s.ptr);

		/* always cleanup */
		curl_easy_cleanup(hnd);
		return jobj;

	}
	else if (CURLE_OK == ret) {
		jobj = json_tokener_parse(s.ptr);
		free(s.ptr);

		/* always cleanup */
		curl_easy_cleanup(hnd);
		return jobj;
	}

}

//SETUP DO CONTEUDO DA ISABELA
void setupIsabela(){

 	struct json_object *jobj_array, *jobj_obj;
  	struct json_object *jobj_object_id, *jobj_object_type, *jobj_object_activity, *jobj_object_location, *jobj_object_latlong, 	*jobj_object_callsduration, 
  	*jobj_object_callsmade, *jobj_object_callsmissed, *jobj_object_callsreceived, *jobj_object_department, *jobj_object_smsreceived, 	*jobj_object_smssent;
	enum json_type type = 0;
	int arraylen = 0;
	int i;
  
    //Get the student data
	jobj_array = get_student_data();
  
	//Get array length
	arraylen = json_object_array_length(jobj_array);
  
	//Example of howto retrieve the data
	for (i = 0; i < arraylen; i++) {
	//get the i-th object in jobj_array
		jobj_obj = json_object_array_get_idx(jobj_array, i);
	    //get the name attribute in the i-th object
	    jobj_object_id = json_object_object_get(jobj_obj, "id");
	    jobj_object_type = json_object_object_get(jobj_obj, "type");
	    jobj_object_activity = json_object_object_get(jobj_obj, "activity");
	    jobj_object_location = json_object_object_get(jobj_obj, "location");
	    jobj_object_callsduration = json_object_object_get(jobj_obj, "calls_duration");
	    jobj_object_callsmade = json_object_object_get(jobj_obj, "calls_made");
    	jobj_object_callsmissed = json_object_object_get(jobj_obj, "calls_missed");
	    jobj_object_callsreceived= json_object_object_get(jobj_obj, "calls_received");
	    jobj_object_department = json_object_object_get(jobj_obj, "department");
	    jobj_object_smsreceived = json_object_object_get(jobj_obj, "sms_received");
	    jobj_object_smssent = json_object_object_get(jobj_obj, "sms_sent");
    
  	    //print out the name attribute
	    printf("id=%s\n", json_object_get_string(jobj_object_id));
	    printf("type=%s\n", json_object_get_string(jobj_object_type));
	    printf("activity=%s\n", json_object_get_string(jobj_object_activity));
	    printf("location=%s\n", json_object_get_string(jobj_object_location));
	    printf("Calls duration(s)=%s\n", json_object_get_string(jobj_object_callsduration));
	    printf("Calls made=%s\n", json_object_get_string(jobj_object_callsmade));
	    printf("Calls missed=%s\n", json_object_get_string(jobj_object_callsmissed));
	    printf("Calls received=%s\n", json_object_get_string(jobj_object_callsreceived));
	    printf("Department=%s\n", json_object_get_string(jobj_object_department));
   	    printf("Sms received=%s\n", json_object_get_string(jobj_object_smsreceived));
	    printf("Sms sent=%s\n", json_object_get_string(jobj_object_smssent));
	    printf("\n");
	}
}

void process_client(int client_fd, struct sockaddr_in client){
	struct json_object *jobj_array, *jobj_obj;
  	struct json_object *jobj_object_id, *jobj_object_type, *jobj_object_activity, *jobj_object_location, *jobj_object_latlong, *jobj_object_callsduration, 
  *jobj_object_callsmade, *jobj_object_callsmissed, *jobj_object_callsreceived, *jobj_object_department, *jobj_object_smsreceived, *jobj_object_smssent;

  	jobj_array = get_student_data();
 
  	int arraylen = json_object_array_length(jobj_array);enum json_type type = 0;

	int nread = 0;
	char buffer[BUF_SIZE], bufferReply[BUF_SIZE];
	char buffer2[BUFF_BUFFER];
	char opcao1[BUF_SIZE]; //opcao pro server saber o que devolver apos menu 1
	char opcao2[BUF_SIZE]; //opcao pro server saber o que devolver	apos menu 2
  	char ip[BUF_SIZE], idCliente[BUF_SIZE]; //SÓ PRA IDs COM 4 CARACTERES <-
	int IDclienteIndex, count=0;

	//ENVIAR BOM DIA
  	snprintf(bufferReply, sizeof(bufferReply), "Bem Vindo chefe\n");
  	write(client_fd, bufferReply, BUF_SIZE);
  	usleep(500);

	//LEITURA DO ID E VALIDAÇAO
	read(client_fd, bufferReply, BUF_SIZE);
	int valid = 0; //validar com um true ou false
	arraylen = json_object_array_length(jobj_array);
   	int indexClientAtual = 0;
   	//printf("%d\n", arraylen);
	for (int i = 0; i < arraylen; i++) {
  	 	jobj_obj = json_object_array_get_idx(jobj_array, i);
  	 	jobj_object_id = json_object_object_get(jobj_obj, "id");
  	 	if( strcmp(json_object_get_string(jobj_object_id),bufferReply) == 0 ){
			valid++;
			indexClientAtual = i;
		}		
  	}

	//envio da validacao, zero se invalido
	if(valid==0){
		write(client_fd,"Cliente Invalido\n", BUF_SIZE);
		printf("Cliente Invalido\n");
	}
  	else{
		write(client_fd,"Cliente Valido. A fazer login...\n", BUF_SIZE);
		printf("Cliente Valido\n");
	}

	//leitura da opcao
	read(client_fd, bufferReply, BUF_SIZE);
	strcpy(opcao1,bufferReply);

	//CICLO MENU2
	jobj_obj = json_object_array_get_idx(jobj_array, indexClientAtual); //para saber a info do cliente atual
	while(strcmp(opcao1,"5")!=0){ //NAO ESQUECER NOVAMENTE OPCAO DO MENU 1

		if(strcmp(opcao1,"1")==0){
			jobj_object_id = json_object_object_get(jobj_obj, "id");
	    	jobj_object_type = json_object_object_get(jobj_obj, "type");
	   		jobj_object_activity = json_object_object_get(jobj_obj, "activity");
	    	jobj_object_location = json_object_object_get(jobj_obj, "location");
	    	jobj_object_callsduration = json_object_object_get(jobj_obj, "calls_duration");
	    	jobj_object_callsmade = json_object_object_get(jobj_obj, "calls_made");
    	    jobj_object_callsmissed = json_object_object_get(jobj_obj, "calls_missed");
	    	jobj_object_callsreceived= json_object_object_get(jobj_obj, "calls_received");
	    	jobj_object_department = json_object_object_get(jobj_obj, "department");
	    	jobj_object_smsreceived = json_object_object_get(jobj_obj, "sms_received");
	    	jobj_object_smssent = json_object_object_get(jobj_obj, "sms_sent");
	    	sprintf(buffer2, "id -> %s\ntype -> %s\nactivity -> %s\nlocation -> %s\nCalls duration(s) -> %s\nCalls made -> %s\nCalls missed -> %s\nCalls received -> %s\nDepartment -> %s\nSms received -> %s\nSms sent -> %s\n", json_object_get_string(jobj_object_id), json_object_get_string(jobj_object_type), json_object_get_string(jobj_object_activity), json_object_get_string(jobj_object_location), json_object_get_string(jobj_object_callsduration), json_object_get_string(jobj_object_callsmade), json_object_get_string(jobj_object_callsmissed), json_object_get_string(jobj_object_callsreceived), json_object_get_string(jobj_object_department), json_object_get_string(jobj_object_smsreceived), json_object_get_string(jobj_object_smssent));
	    	write(client_fd, buffer2, BUFF_BUFFER);
		}

		if(strcmp(opcao1,"2")==0){	
			media();
			sprintf(buffer2,"Calls duration(s) -> %f\nCalls made -> %f\nCalls missed -> %f\nCalls received -> %f\nSms received -> %f\nSms sent -> %f\n",estatisticas->callsduration, estatisticas->callsmade, estatisticas->callsmissed, estatisticas->callsreceived, estatisticas->smsreceived, estatisticas->smssent);
			write(client_fd, buffer2, BUFF_BUFFER);	   
		}

	read(client_fd, bufferReply, BUF_SIZE);
	strcpy(opcao1,bufferReply); //NOVA opcao no menu 1

	}

	if(strcmp(opcao1, "5")==0){
		printf("Cliente fez logout.\n");
		exit(0);
	}
}

void notify(char * string, float media, float mediaAnt){

	char buff[BUF_SIZE];
	write(client1, "NOTIFICATION", BUF_SIZE);
	sprintf(buff, "%.3f", media);
	write(client1, string, BUF_SIZE);
	write(client1, buff, BUF_SIZE);
}

void media(){

	float soma = 0;

	struct json_object *jobj_array, *jobj_obj;
  	struct json_object *jobj_object_id, *jobj_object_type, *jobj_object_activity, *jobj_object_location, *jobj_object_latlong, *jobj_object_callsduration, 
  *jobj_object_callsmade, *jobj_object_callsmissed, *jobj_object_callsreceived, *jobj_object_department, *jobj_object_smsreceived, *jobj_object_smssent;


  	jobj_array = get_student_data();
 
  	int arraylen = json_object_array_length(jobj_array);
  
  	for (int i=0; i<arraylen; i++){
  		jobj_obj = json_object_array_get_idx(jobj_array, i);
  		jobj_object_callsduration = json_object_object_get(jobj_obj, "calls_duration");
		if(json_object_get_string(jobj_object_callsduration)==NULL){
			continue;
	    }
  		int callsduration = atoi(json_object_get_string(jobj_object_callsduration));
    	soma+=callsduration;
  	}
  	
  	estatisticas->callsduration = soma/arraylen;
  	soma=0;

  	for (int i=0; i<arraylen; i++){
  		jobj_obj = json_object_array_get_idx(jobj_array, i);
  		jobj_object_callsmade = json_object_object_get(jobj_obj, "calls_made");
		  if(json_object_get_string(jobj_object_callsmade)==NULL){
			continue;
	    }
  		int callsmade = atoi(json_object_get_string(jobj_object_callsmade));
    	soma+=callsmade;
 	 }

  	estatisticas->callsmade = soma/arraylen;
  	soma=0;


  	for (int i=0; i<arraylen; i++){
  		jobj_obj = json_object_array_get_idx(jobj_array, i);
  		jobj_object_callsmissed = json_object_object_get(jobj_obj, "calls_missed");
		if( json_object_get_string(jobj_object_callsmissed)==NULL){
			continue;
	    }
  		int callsmissed = atoi(json_object_get_string(jobj_object_callsmissed));
    	soma+=callsmissed;
  	}

  	estatisticas->callsmissed = soma/arraylen;
  	soma=0;

  	for (int i=0; i<arraylen; i++){
  		jobj_obj = json_object_array_get_idx(jobj_array, i);
  		jobj_object_callsreceived = json_object_object_get(jobj_obj, "calls_received");
		if(json_object_get_string(jobj_object_callsreceived)==NULL){
			continue;
	    }
  		int callsreceived = atoi(json_object_get_string(jobj_object_callsreceived));
    	soma+=callsreceived;
  	}
  	estatisticas->callsreceived = soma/arraylen;
  	soma = 0;

  	for (int i=0; i<arraylen; i++){
  		jobj_obj = json_object_array_get_idx(jobj_array, i);
  		jobj_object_smsreceived = json_object_object_get(jobj_obj, "sms_received");
 		if(json_object_get_string(jobj_object_smsreceived)==NULL){
			continue;
	    }
  		int smsreceived = atoi(json_object_get_string(jobj_object_smsreceived));
    	soma+=smsreceived;
  	}

  	estatisticas->smsreceived = soma/arraylen;
  	soma=0;

  	for (int i=0; i<arraylen; i++){
  		jobj_obj = json_object_array_get_idx(jobj_array, i);
  		jobj_object_smssent = json_object_object_get(jobj_obj, "sms_sent");
		 if(json_object_get_string(jobj_object_smssent)==NULL){
			continue;
	    }
  		int smssent = atoi(json_object_get_string(jobj_object_smssent));
    	soma+=smssent;
  	}

  	estatisticas->smssent = soma/arraylen;
  	soma=0;
}


void checkMedia(){

 	float soma = 0;
  	struct json_object *jobj_array, *jobj_obj;
  	struct json_object *jobj_object_id, *jobj_object_type, *jobj_object_activity, *jobj_object_location, *jobj_object_latlong, *jobj_object_callsduration, 
  	*jobj_object_callsmade, *jobj_object_callsmissed, *jobj_object_callsreceived, *jobj_object_department, *jobj_object_smsreceived, *jobj_object_smssent;


  	jobj_array = get_student_data();
 
  	int arraylen = json_object_array_length(jobj_array);
  	estatisticasTemp->callsduration = estatisticas->callsduration;
  	estatisticasTemp->callsmissed = estatisticas->callsmissed;
  	estatisticasTemp->callsmade = estatisticas->callsmade;
  	estatisticasTemp->callsreceived = estatisticas->callsreceived;
  	estatisticasTemp->smsreceived = estatisticas->smsreceived;
  	estatisticasTemp->smssent = estatisticas->smssent;

	for (int i=0; i<arraylen; i++){
  		jobj_obj = json_object_array_get_idx(jobj_array, i);
  		jobj_object_callsduration = json_object_object_get(jobj_obj, "calls_duration");
  		if(json_object_get_string(jobj_object_callsduration)==NULL){
  			continue;
  		}
  		int callsduration = atoi(json_object_get_string(jobj_object_callsduration));
    	soma+=callsduration;
  	}
  	estatisticas->callsduration = soma/arraylen;
  	if(estatisticas->callsduration!=estatisticasTemp->callsduration){
  		notify("duracao das chamadas", estatisticas->callsduration, estatisticasTemp->callsduration);
  	}
  	soma=0;

  	for (int i=0; i<arraylen; i++){
  		jobj_obj = json_object_array_get_idx(jobj_array, i);
    	jobj_object_callsmade = json_object_object_get(jobj_obj, "calls_made");
    	if(json_object_get_string(jobj_object_callsmade)==NULL){
  			continue;
  		}
  		int callsmade = atoi(json_object_get_string(jobj_object_callsmade));
    	soma+=callsmade;
  	}
  	estatisticas->callsmade = soma/arraylen;
  	if(estatisticas->callsmade!=estatisticasTemp->callsmade){
  		notify("chamadas feitas", estatisticas->callsmade, estatisticasTemp->callsmade);
  	}
  	soma=0;

  	for (int i=0; i<arraylen; i++){
  		jobj_obj = json_object_array_get_idx(jobj_array, i);
    	jobj_object_callsmissed = json_object_object_get(jobj_obj, "calls_missed");
    	if(json_object_get_string(jobj_object_callsmissed)==NULL){
  			continue;
  		}
  		int callsmissed = atoi(json_object_get_string(jobj_object_callsmissed));
    	soma+=callsmissed;
  	}
  	estatisticas->callsmissed = soma/arraylen;
  	if(estatisticas->callsmissed!=estatisticasTemp->callsmissed){
  		notify("chamadas perdidas", estatisticas->callsmissed, estatisticasTemp->callsmissed);
  	}
  	soma=0;

  	for (int i=0; i<arraylen; i++){
  		jobj_obj = json_object_array_get_idx(jobj_array, i);
    	jobj_object_callsreceived = json_object_object_get(jobj_obj, "calls_received");
    	if(json_object_get_string(jobj_object_callsreceived)==NULL){
  			continue;
  		}
  		int callsreceived = atoi(json_object_get_string(jobj_object_callsreceived));
    	soma+=callsreceived;
  	}
  	estatisticas->callsreceived = soma/arraylen;
  	if(estatisticas->callsreceived!=estatisticasTemp->callsreceived){
  		notify("chamadas recebidas", estatisticas->callsreceived, estatisticasTemp->callsreceived);
  		}
  	soma=0;

  	for (int i=0; i<arraylen; i++){
  		jobj_obj = json_object_array_get_idx(jobj_array, i);
    	jobj_object_smsreceived = json_object_object_get(jobj_obj, "sms_received");
    	if(json_object_get_string(jobj_object_smsreceived)==NULL){
  			continue;
  		}
  		int smsreceived = atoi(json_object_get_string(jobj_object_smsreceived));
    	soma+=smsreceived;
  	}
  	estatisticas->smsreceived = soma/arraylen;
  	if(estatisticas->smsreceived!=estatisticasTemp->smsreceived){
  		notify("sms recebidas", estatisticas->smsreceived, estatisticasTemp->smsreceived);
  		}
  	soma=0;

  	for (int i=0; i<arraylen; i++){
  		jobj_obj = json_object_array_get_idx(jobj_array, i);
    	jobj_object_callsmissed = json_object_object_get(jobj_obj, "sms_sent");
    	if(json_object_get_string(jobj_object_smsreceived)==NULL){
  			continue;
  		}
  		int smssent = atoi(json_object_get_string(jobj_object_smssent));
    	soma+=smssent;
  	}
  	estatisticas->smssent = soma/arraylen;
  	if(estatisticas->smssent!=estatisticasTemp->smssent){
  			notify("sms enviadas", estatisticas->smssent, estatisticasTemp->smssent);
  		}
  	soma=0;

}

void subscriber_process(int client_fd, struct sockaddr_in client1){

	while(1){
    	checkMedia();
   	    sleep(1);
  }
  exit(0);
}

void sockets(){

  	bzero((void *) &addr, sizeof(addr));
  	addr.sin_family = AF_INET;
  	addr.sin_addr.s_addr = htonl(INADDR_ANY);
 	addr.sin_port = htons(SERVER_PORT);

 	//inicializar o primeiro socket
  	if(fork()==0){	
    		if ( (fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  	  		perror("na funcao socket");
    		if ( bind(fd,(struct sockaddr*)&addr,sizeof(addr)) < 0)
  	  		perror("na funcao bind");
    		if( listen(fd, 5) < 0)
    			perror("na funcao listen");
    	client_addr_size = sizeof(client_addr);
    	while (1) {
      	//clean finished child processes, avoiding zombies
      	//must use WNOHANG or would block whenever a child process was working
      	while(waitpid(-1,NULL,WNOHANG)>0);
      	//wait for new connection
      	client = accept(fd,(struct sockaddr *)&client_addr,(socklen_t *)&client_addr_size);
      	printf("Connected.\n");
        if (fork() == 0) {
        	signal(SIGINT, terminus);
       	  	process_client(client, (struct sockaddr_in)client_addr);
       	  	close(fd);
       	  	exit(0);
       	}
      	close(client);
      }
   	}

  //inicializar o segundo socket
  if(fork()==0){
    addr.sin_port = htons(SUB_PORT);

    if((fd1 = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        printf("Erro na funcao socket.\n");
    else
    	printf("Socket de subscricao criado com sucesso...\n");

    if(bind(fd1, (struct sockaddr*)&addr, sizeof(addr)) < 0)
        printf("Erro na funcao bind.\n");
    else
        printf("Binded.\n");

    if(listen(fd1, 5) < 0)
        printf("Erro na funcao listen.\n");
    else
        printf("Listening.\n");

      client_addr_size = sizeof(client_addr);

    while(1){

        while(waitpid(-1, NULL, WNOHANG)>0);

        //wait for new connection 
        client1 = accept(fd1, (struct sockaddr *)&client_addr, (socklen_t *)&client_addr_size);
        printf("Connected.\n");
       
          if(fork()==0){
          	signal(SIGINT, terminus);
            subscriber_process(client1, (struct sockaddr_in)client_addr);
            close(fd1);
            exit(0);
          }
         close(client1);
        
      }
    }
}

/////////////Alterei//////////////////////////

int main(){
	estatisticasTemp = (struct estatistica*)malloc(sizeof(struct estatistica));
	estatisticas = (struct estatistica*)malloc(sizeof(struct estatistica));
	signal(SIGINT, terminus);
	setupIsabela();
	media();
	sockets();		
	exit(0);
	return 0;
}





