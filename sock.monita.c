//-----------------//
// 6 Desember 2012 //
//  Afrendy  Bayu  //
//-----------------//


#include <stdio.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>	//inet_addr
#include "sock.monita.h"

#define PAKAI_DEBUG			1

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

int parsing_konfig(char *s)	{
	int i=0;
	//printf("p: %d\r\n", strlen(s));
	char a[20], b[128];
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
			ipsumber = (char *) malloc(sumber.jmlSumber * sizeof(ipsumber));
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
			printd("i: %d, iI: %d\r\n", i, iI);
			ipsumber[iI].no = i;
			strcpy(ipsumber[iI].ip, b); printd("%s\r\n", b); 
			iI++;
			printd("i: %d, iI: %d\r\n", i, iI);
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
	return i;
}

void init_var()		{
	iI = 0;
}

int buka_soket()	{
	int i, n;
	int socket_desc;
	struct sockaddr_in server_m[sumber.jmlSumber];
	
	printd("===> %s masuk...\r\n", __FUNCTION__);
	
	for (i=0; i<sumber.jmlSumber; i++)	{
		ipsumber[i].socket_desc = socket(AF_INET , SOCK_STREAM , 0);
		printd("socket desc: %d\r\n", ipsumber[i].socket_desc);
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
			} else
				ipsumber[i].socket = -1;
		}
	}
}

void ambil_data()	{
	int i=0;
	printf("  ---%3d/%d : Ambil DATA:", ++iI, sumber.jmlSumber);
	for (i=0; i<sumber.jmlSumber; i++)	{
		printf(" %d/%d --> %s ||", ipsumber[i].no, sumber.jmlSumber, ipsumber[i].ip);
	}
	printf("\r\n");
}

int main(int argc , char *argv[])	{
	int socket_desc;
	struct sockaddr_in server;
	char message[12] , s_reply[300];
	struct t_xdata st_data;
	int i, jmlNSock;

	
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
	free(ipsumber);		// --> free ipsumber kok ERROR !!!
	
	
	
	return 0;

	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	printd("Buat socket_desc: %d\r\n", socket_desc);
	if (socket_desc == -1)	{
		printd("Could not create socket");
	}

	server.sin_addr.s_addr = inet_addr("192.168.1.250");
	server.sin_family = AF_INET;
	server.sin_port = htons( 5001 );

	printd("Connect to remote server\r\n");
	if (connect(socket_desc , (struct sockaddr *)&server , sizeof(server)) < 0)
	{
		puts("connect error");
		return 1;
	}
	
	printd("Connected");
	
	sprintf(message, "sampurasun%d", 1);
	//message = "GET / HTTP/1.1\r\n\r\n";
	if( send(socket_desc , message , strlen(message) , 0) < 0)    {
        puts("Send failed");
        return 1;
    }
    printd("Data Send\n");

	//Receive a reply from the server
	jmlNSock = recv(socket_desc, s_reply , 2000 , 0);
    if( jmlNSock < 0)    {
        puts("recv failed");
    }
    printd("Reply received: %d/%d/%d\n", jmlNSock, sizeof(st_data), sizeof(data_f));
    printd(s_reply);
	memcpy ( &st_data, &s_reply, jmlNSock );
	memcpy( (char *) &data_f, st_data.buf, PER_SUMBER*sizeof(float) );
    
    printd("==> %s >> urut:%d >> flag:%d >> alamat:%d\r\n", st_data.mon, st_data.nomer, st_data.flag, st_data.alamat);
	for(i=0; i<PER_SUMBER; i++)		{
		printd("%.2f ", data_f[i]);
	}
	printd("\r\n");
	
	return 0;
}
