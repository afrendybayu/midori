//-----------------//
// 6 Desember 2012 //
//  Afrendy  Bayu  //
//-----------------//


#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>	//inet_addr
#include <signal.h>
#include "sock.monita.h"

#define PAKAI_DEBUG			1
#define JEDA	5

int printd(const char *format, ...)	{
#if (PAKAI_DEBUG==1)
	va_list arg;
	int done;

	va_start (arg, format);
	done = vfprintf (stdout, format, arg);
	va_end (arg);

	return done;
#endif
}

void siginthandler(int param)	{
	printf("\r\n ---- masuk Interupsi !!!!\r\n");
	free(data_f);
	free(ipsumber);
	printf(">>>>>>>>>>> Free Memory\r\n");
	sleep(1);
	exit(1);
}

int parsing_konfig(char *s)	{
	int i=0;
	//printf("p: %d\r\n", strlen(s));
	char a[50], b[128];
	const char *pch;
	pch = strchr(s, '#');
	if (pch!=NULL)	return 2;
	if (strlen(s)>1)	{
		sscanf(s,"%s = %s", a, b);
		printd("  ----> a:>>%s<<, b:>>%s<<\r\n", a, b);
		if (!strcmp(a,"serialport"))	{	printd("%s\r\n", b); strcpy(com_mod.comSer, b); }
		if (!strcmp(a,"baudrate"))	{		printd("%s\r\n", b); com_mod.baud = atoi(b); }
		if (!strcmp(a,"soket"))	{			printd("%s\r\n", b); sumber.socket = atoi(b); }
		
		if (!strcmp(a,"jmlsumber"))	{	
			printd("%s\r\n", b);  sumber.jmlSumber = atoi(b); 
			ipsumber = malloc(sumber.jmlSumber * sizeof(struct t_ipsumber));
			data_f = malloc (sumber.jmlSumber * PER_SUMBER * sizeof(float));
			if (ipsumber==NULL)	{
				printf(" --- ERROR MALLOC sumber !!!\r\n ---\r\n");
				return 0;
			}
		}
		if (!strcmp(a,"sumber"))	{
			printd("isinya: %s = %s\r\n", a, b);
			i=iI+1;
			if (i>sumber.jmlSumber)	{
				printf(" --- ERROR Sumber LEBIH !!!\r\n ---\r\n");
				return 0;
			}
			//printd("i: %d, iI: %d\r\n", i, iI);
			ipsumber[iI].no = i;
			strcpy(ipsumber[iI].ip, b); printd("%s\r\n", b); 
			iI++;
			//printd("i: %d, iI: %d\r\n", i, iI);
		}
		if (!strcmp(a,"idsumber"))	{
			
		}
	}
	return 1;
}

void cek_konfig()	{
	int i=0;
	printf("serialport: %s\r\n", com_mod.comSer);
	printf("baudrate  : %d\r\n", com_mod.baud);
	printf("socket    : %d\r\n", sumber.socket);
	printf("Jml Smbr  : %d\r\n", sumber.jmlSumber);
	for(i=0; i<sumber.jmlSumber; i++)	{
		printf("%3d. IP: %s\r\n", ipsumber[i].no, ipsumber[i].ip);
	}
}

int akses_file_konfig()	{
	int i=0, j, max=100;
	char line [100];
	pFile = fopen (FILENYA,"r");
	
	printd("===> %s masuk !!\r\n", __FUNCTION__);
	if (pFile!=NULL)	{
		while ( fgets ( line, sizeof line, pFile ) != NULL )	{
			if (!parsing_konfig(line)) return 0;
			if (i>max)	break;
			i++;
		}
		fclose(pFile);
	}
	printf("tutup FILE\r\n");
	//free(data_f);
	//free(ipsumber);
	//printf(">>>>>>>>>>> Free Memory\r\n");
	return i;
}

void init_var()		{
	iI = 0;
	aa = 02;
}

int buka_soket_satuan(int i)	{
	struct sockaddr_in server_m;
	printd("~~~~~ %s masuk: soket: %d/%d\r\n", __FUNCTION__, ipsumber[i].socket, ipsumber[i].socket_desc);
	switch (ipsumber[i].socket)	{
		case -1: // buka lokal socket
			printd(" +++++ buat socket dulu !!\r\n");
			ipsumber[i].socket_desc = socket(AF_INET , SOCK_STREAM , 0);
			printd("socket desc: %d\r\n", ipsumber[i].socket_desc);
			ipsumber[i].socket = 1;
		case 2:	 // konek socket ke tujuan
			printd(" +++++ coba buka soket %d/%d\r\n", i, ipsumber[i].socket_desc);
			server_m.sin_addr.s_addr = inet_addr(ipsumber[i].ip);
			server_m.sin_family = AF_INET;
			server_m.sin_port = htons( sumber.socket );
			ipsumber[i].socket = connect(ipsumber[i].socket_desc, (struct sockaddr *)&server_m , sizeof(server_m));
			if (ipsumber[i].socket >= 0)		{
				printd("Connected %d\r\n", ipsumber[i].socket);
				ipsumber[i].stat_konek = 1;
				ipsumber[i].socket = 2;
			} else {
				printd("Tidak nyambung !!!\r\n");
				ipsumber[i].stat_konek = 0;
				ipsumber[i].jeda = JEDA;
			}
	}
}

int buka_soket()	{
	int i, n;
	int socket_desc;
	struct sockaddr_in server_m[sumber.jmlSumber];
	
	printd("===> %s masuk...\r\n", __FUNCTION__);
	
	for (i=0; i<sumber.jmlSumber; i++)	{
		ipsumber[i].socket_desc = socket(AF_INET , SOCK_STREAM , 0);
		printd("socket desc: %d\r\n", ipsumber[i].socket_desc);
		ipsumber[i].socket = 1;
		if (socket_desc == -1)	{
			printd("Could not create socket");
		} else {
			server_m[i].sin_addr.s_addr = inet_addr(ipsumber[i].ip);
			server_m[i].sin_family = AF_INET;
			server_m[i].sin_port = htons( sumber.socket );
			
			printd("Connect to remote server\r\n");
			ipsumber[i].socket = connect(ipsumber[i].socket_desc, (struct sockaddr *)&server_m[i] , sizeof(server_m[i]));
			
			if (ipsumber[i].socket >= 0)		{
				printd("Connected %d\r\n", ipsumber[i].socket);
				ipsumber[i].stat_konek = 1;
				ipsumber[i].socket = 2;
			} else {
				//ipsumber[i].socket = -1;
				ipsumber[i].stat_konek = 0;
				ipsumber[i].jeda = JEDA;
			}
		}
	}
}

int ambil_data_satuan(int no)	{
	struct t_xdata st_data;
	char message[12] , s_reply[300];
	
	printf(" %s masuk\r\n", __FUNCTION__);
	int jmlNSock, i;
	sprintf(message, "sampurasun%d", 1);
	
	jmlNSock = send(ipsumber[no].socket_desc, message, strlen(message), 0);
	if( jmlNSock < 0 )    {
        printf("Send failed");
        sleep(2);
        //return 0;
    }
    printf(" %s masuk2: %d\r\n", __FUNCTION__, jmlNSock);
    printf("Data Send [%d]--> ", no);
    //sleep(2);
    if (jmlNSock>0)	{
    
		jmlNSock = recv(ipsumber[no].socket_desc, s_reply, 300 , 0);
		if( jmlNSock < 0)    {
			printf("recv failed");
			sleep(2);
			return 0;
		}
		printd("Reply received: %d/%d/%d\n", jmlNSock, sizeof(st_data), sizeof(data_f));
		printd(s_reply);
		
		memcpy ( &st_data, &s_reply, jmlNSock );
		memcpy( (char *) &data_f[no], st_data.buf, PER_SUMBER*sizeof(float) );
		
		printd("==> %s >> urut:%d >> flag:%d >> alamat:%d\r\n", st_data.mon, st_data.nomer, st_data.flag, st_data.alamat);
		for(i=0; i<PER_SUMBER; i++)		{
			//printd("%.2f ", data_f[i]);
		}
		//printd("-++-");
		return 1;
	}
	return 0;
}

void ambil_data()	{
	int i=0, flag=0;
	printd("  ---%3d/%d : Ambil DATA:\r\n", ++iI, sumber.jmlSumber);
	for (i=0; i<sumber.jmlSumber; i++)	{
		printd(" %d/%d --> %s ||", ipsumber[i].no, sumber.jmlSumber, ipsumber[i].ip);
		if (ipsumber[i].stat_konek==1)	{
			flag = ambil_data_satuan(i);
		}
		if (ipsumber[i].jeda>0)	{
			printf(" %%%%%%%% Turunkan JEDA !\r\n");
			ipsumber[i].jeda--;
		} else {
			if (ipsumber[i].stat_konek==0)	{
				printf(" ---- COBA BUKA SOKET LAGI !!!\r\n");
				buka_soket_satuan(i);
			}
		}
	}
	
}

int main(int argc , char *argv[])	{
	int socket_desc;
	struct sockaddr_in server;
	char message[12] , s_reply[300];
	struct t_xdata st_data;
	int i, jmlNSock;

	signal(SIGINT, siginthandler);
	signal(SIGQUIT, siginthandler);
	
	i = akses_file_konfig();
	printd(" Akses konfig selesai %d\r\n", i);
	
	cek_konfig();
	printf(" Cek konfig selesai %d/%d\r\n", iI, sumber.jmlSumber);
	
	buka_soket();
	iI = 0;
	while(1)	{
		sleep(1);
		ambil_data();
	}
	printf("keluar LOOP !!!\r\n");
	siginthandler(1);
	printf("keluar MAIN !!!\r\n");
	
	return 0;


	//Receive a reply from the server
	jmlNSock = recv(socket_desc, s_reply , 2000 , 0);
    if( jmlNSock < 0)    {
        puts("recv failed");
    }
    printd("Reply received: %d/%d/%d\n", jmlNSock, sizeof(st_data), sizeof(data_f));
    printd(s_reply);
	
	
	return 0;
}
