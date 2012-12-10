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
#include <unistd.h>
#include <time.h>
#include "sock.monita.h"

#define PAKAI_DEBUG			1
#define JEDA	2

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

void sig_pipe(int signum)	{
	printd("==========> %s:%d, iI: %d\r\n", __FUNCTION__, signum, iI);
	buka_soket_satuan(iI);
	signal(SIGPIPE, sig_pipe);
}

int parsing_konfig(char *s)	{
	int i=0;
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
			printf("======= ALOKASI MEMORI SUMBER dan DATA ========\r\n");
			ipsumber = malloc(sumber.jmlSumber * sizeof(struct t_ipsumber));
			data_f = malloc (sumber.jmlSumber * PER_SUMBER * sizeof(float));
			idData = malloc (sumber.jmlSumber * PER_SUMBER * sizeof(int));
			if (ipsumber==NULL)	{
				printf(" --- ERROR MALLOC sumber !!!\r\n ---\r\n");
				return 0;
			}
			if (data_f==NULL)	{
				printf(" --- ERROR MALLOC data_f !!!\r\n ---\r\n");
				return 0;
			}
			if (idData==NULL)	{
				printf(" --- ERROR MALLOC id_konfig !!!\r\n ---\r\n");
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
			ipsumber[iI].no = i;
			strcpy(ipsumber[iI].ip, b); printd("%s\r\n", b); 
			iI++;
			//printd("i: %d, iI: %d\r\n", i, iI);
		}
		if (!strcmp(a,"idsumber"))	{
			//printd("iI: %d \r\n", iI);
			i=0;
			pch = strtok (b,",");
			while (pch != NULL)	{
				idData[(iI-1)*PER_SUMBER+i] = atoi(pch); 		//printd ("%s\n",pch);
				pch = strtok (NULL, ",");
				i++;
			}
		}
	}
	return 1;
}

void cek_konfig()	{
	int i=0, j;
	printf("serialport: %s\r\n", com_mod.comSer);
	printf("baudrate  : %d\r\n", com_mod.baud);
	printf("socket    : %d\r\n", sumber.socket);
	printf("Jml Smbr  : %d\r\n", sumber.jmlSumber);
	for(i=0; i<sumber.jmlSumber; i++)	{
		printf("%3d. IP: %s\r\n", ipsumber[i].no, ipsumber[i].ip);
	}
	for(i=0; i<sumber.jmlSumber; i++)	{
		for (j=0; j<PER_SUMBER; j++)	{
			printf("id[%2d]: %4d --- ", i*PER_SUMBER+j, idData[i*PER_SUMBER+j]);
			if (!((j+1)%5))	printf("\r\n");
		}
	}
}

int akses_file_konfig()	{
	int i=0, max=100;
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
	printd("tutup FILE\r\n");
	return i;
}

void init_var()		{
	iI = 0;
	aa = 02;
	bb = 1;
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
				printd("Connected buka mandiri: %d\r\n", ipsumber[i].socket_desc);
				ipsumber[i].stat_konek = 1;
				ipsumber[i].socket = 2;
			} else {
				printd("Tidak nyambung !!! TUTUP SOKET %d !!!\r\n", ipsumber[i].socket_desc);
				ipsumber[i].stat_konek = 0;
				ipsumber[i].jeda = JEDA;
				close(ipsumber[i].socket_desc);
			}
	}
	return 1;
}

int buka_soket()	{
	int i;
	//int socket_desc;
	struct sockaddr_in server_m[sumber.jmlSumber];
	
	printd("===> %s masuk...\r\n", __FUNCTION__);
	
	for (i=0; i<sumber.jmlSumber; i++)	{
		ipsumber[i].socket_desc = socket(AF_INET , SOCK_STREAM , 0);
		printd("socket desc: %d\r\n", ipsumber[i].socket_desc);
		ipsumber[i].socket = 1;
		if (ipsumber[i].socket_desc == -1)	{
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
	return 1;
}

int ambil_data_satuan(int no)	{
	struct t_xdata st_data;
	char message[20] , s_reply[300];

	iI = no;		strcpy(s_reply, "");
	//printd(" %s masuk\r\n", __FUNCTION__);
	int jmlNSock, i;

	sprintf(message, "sampurasun%d", 1);
	//printf("\r\n------------%s ... no: %d, sock desc[%d]: %d, pesan: %s\r\n", __FUNCTION__, no, i, ipsumber[i].socket_desc, message);

	jmlNSock = send(ipsumber[no].socket_desc, message, strlen(message), 0);
	if (jmlNSock < 0)    {
        printf("Send failed");
        return 1;
    }
    
    if (jmlNSock>0)	{
		jmlNSock = recv(ipsumber[no].socket_desc, s_reply, 300 , 0);
		if( jmlNSock <= 0)    {
			printf("recv failed");
			//sleep(2);
			return 0;
		}
		printd("Reply received: %d/%d/%d\n", jmlNSock, sizeof(st_data), sizeof(data_f));
		//printd(s_reply);
		
		memcpy ( &st_data, &s_reply, jmlNSock );
		memcpy( (char *) &data_f[no], st_data.buf, PER_SUMBER*sizeof(float) );
		
		printd("==> no: %d === %s >> urut:%d >> flag:%d >> alamat:%d\r\n", no, st_data.mon, st_data.nomer, st_data.flag, st_data.alamat);
		for(i=0; i<PER_SUMBER; i++)		{
			printd("%.1f ", data_f[no*PER_SUMBER+i]);
		}
		printf("*");
		return 1;
	}
	printf("keluar JALUR %s tutup soket !!!\r\n", __FUNCTION__);
	close(ipsumber[no].socket_desc);
	//sleep(3);
	return 0;
}

void ambil_data()	{
	int i=0;
	printd("  ---%3d/%d : Ambil DATA: %d\r\n", ++iI, sumber.jmlSumber, aa++);
	for (i=0; i<sumber.jmlSumber; i++)	{
		printd(" %d/%d --> \"%s\" ||", ipsumber[i].no, sumber.jmlSumber, ipsumber[i].ip);
		if (ipsumber[i].stat_konek==1)	{
			ambil_data_satuan(i);
			//parsing_data_satuan(i);
			//buka_sendiri(i);
		}
		if (ipsumber[i].jeda>0)	{
			printd(" %%%%%%%% Turunkan JEDA %d -->!\r\n", i, ipsumber[i].jeda);
			ipsumber[i].jeda--;
		} else {
			if (ipsumber[i].stat_konek==0)	{
				printd(" ---- COBA BUKA SOKET LAGI %d !!!\r\n", i);
				buka_soket_satuan(i);
			}
		}
	}
	
}

void simpan_ke_file()	{
	char isifile[1024], perk[20];
	int i;
	printf("_______%s: %d === %d\r\n", __FUNCTION__, PER_SUMBER*sumber.jmlSumber, bb++);
	pFile = fopen ("datamon.txt","a+");
	if (pFile!=NULL)	{
		for (i=0; i<(PER_SUMBER*sumber.jmlSumber); i++)	{
			
			//if (idData[i]>0)	
			{
				sprintf(perk, "%.1f ", data_f[i]);
				printf("idData[%d]: %.2f ", idData[i], data_f[i]);
			}
			strcat(isifile, perk);
		}
		strcat(isifile, "\r\n");
		printf("\r\n");
		//printf("isiFIle: %s\r\n", isifile);
		//*/
		//strcat(isifile, "qwertyu\r\n");
		fputs (isifile,pFile);
		fclose (pFile);
	}
}

int main(int argc , char *argv[])	{
	int i, j, k;

	signal(SIGINT, siginthandler);
	//signal(SIGQUIT, siginthandler);
	signal(SIGPIPE, sig_pipe);
	
	init_var();
	
	i = akses_file_konfig();	printd(" Akses konfig selesai %d\r\n", i);
	cek_konfig();	printf(" Cek konfig selesai %d/%d\r\n", iI, sumber.jmlSumber);
	buka_soket();
	iI = 0;
	
	#if 0
	buka_sendiri(0);
	#endif
	
	printf("Parent PID(%d): Mulai sedot !!...\n", getpid());
	
	#if 1

	while(1)	{
		ambil_data();
		usleep(1000);
		if (j>100)	{
			//printf("Restart i\r\n");
			simpan_ke_file();
			j=0;
		}
		j++;
		//delay(1000);
	}
	#endif
	printf("keluar LOOP !!!\r\n");
	siginthandler(1);
	printf("keluar MAIN !!!\r\n");
	
	return 0;
}

int buka_sendiri(int i)	{
	struct t_xdata st_data;
	char message[20] , s_reply[300];
	int jmlNSock;
	
	sprintf(message, "sampurasun%d", 1);
	printf("\r\n------------%s ... no: %d, sock desc[%d]: %d, pesan: %s\r\n", __FUNCTION__, i, i, ipsumber[i].socket_desc, message);
	if( send(ipsumber[i].socket_desc , message , strlen(message) , 0) < 0)    {
        puts("Send failed");
        return 1;
    }
    printd("Data Send\n");
	
	jmlNSock = recv(ipsumber[i].socket_desc, s_reply , 300 , 0);
    if( jmlNSock < 0)    {
        puts("recv failed");
    }
	
	printd("Reply received: %d/%d/%d\n", jmlNSock, sizeof(st_data), sizeof(data_f));
    printd("%s\r\n", s_reply);
	
	memcpy ( &st_data, &s_reply, jmlNSock );
	memcpy( (char *) &data_f[0], st_data.buf, PER_SUMBER*sizeof(float) );
    
    printd("==> %s >> urut:%d >> flag:%d >> alamat:%d\r\n", st_data.mon, st_data.nomer, st_data.flag, st_data.alamat);
	
	for(i=0; i<PER_SUMBER; i++)		{
		printd("%.2f ", data_f[i]);
	}
	printd("\r\n");
	
	printf("Rehat dulu !!!\r\n");   
	close(ipsumber[i].socket_desc);
	sleep(1);
	return 0;

}
