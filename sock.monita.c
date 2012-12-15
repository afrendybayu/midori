//-----------------//
// 6 Desember 2012 //
//  Afrendy  Bayu  //
//-----------------//


#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <string.h>
#include <arpa/inet.h>	//inet_addr
#include <netdb.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <regex.h>
#include "sock.monita.h"
#include <pthread.h>
#include <dirent.h>
#include <string.h> //memset
#include <errno.h> //For errno - the error number
#include <netdb.h>	//hostent


#define PAKAI_DEBUG		
#define JEDA	2

pthread_mutex_t mxq = PTHREAD_MUTEX_INITIALIZER;

int printd(int prio, const char *format, ...)	{
#ifdef PAKAI_DEBUG
	va_list arg;

	printf("\033[32m");
	//printf("\033[7;#m");
	int done=0;
	if (g.debug==0) return 0;
	if (prio>=g.debug)	{
		va_start (arg, format);
		done = vfprintf (stdout, format, arg);
		va_end (arg);
	}
	printf("\033[0m");
	return done;
#endif
}

void sig_int(int param)	{
	char kata[100];
	struct stat sts;
	
	printf("\r\n ---- masuk Interupsi !!!!\r\n");
	
	pthread_mutex_unlock(&mxq); 
	printf("================ mutex unlock thread !!\r\n");
	
	if (g.st_thread == 1)	{
		if(pthread_join(kirim_thread, NULL)) {
			fprintf(stderr, "Error joining thread\n");
		} else {
			printf(" ====== berhasil JOIN Thread !!\r\n");
			g.st_thread = 0;
		}
	}
	
	free(data_f);
	free(ipsumber);
	printf(">>>>>>>>>>> Free Memory\r\n");	

	sprintf(kata, "%s%s", sumber.folder, PID_FILE);
	//st = stat (kata, &sts);
	//printf("st: %d\r\n", st);
	if (!stat (kata, &sts))	{
		printf ("Hapus File %s...", PID_FILE);
		if (remove(kata) == 0)
			printf(" TERHAPUS.\r\n");
		else 
			printf(" GAGAL.\r\n");
	}
	
	sleep(1);
	exit(1);
}

void sig_pipe(int signum)	{
	printd(1000, "===== PIPE =====> %s:%d, iI: %d\r\n", __FUNCTION__, signum, iI);
	buka_soket_satuan(iI);
	signal(SIGPIPE, sig_pipe);
}

void sig_keluar(int signum)	{
	printd(1000, "==========> %s KELUAR APLIKASI <=========\r\n", __FUNCTION__);
}

int waktu_atoi(char *waktu)	{
	int angka = atoi(waktu);
	if (strchr(waktu, 'm')!=NULL)     //return (angka);
		angka *= 60;
	if (strchr(waktu, 'h')!=NULL)     //return (angka);
		angka *= 3600;
	return angka;
}

int ip_valid(char *ip)	{
	int i=0;
	struct sockaddr_in sa;
	i = inet_pton(AF_INET, ip, &(sa.sin_addr));
	
	if (i==1)	return i;

	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_in *h;
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // use AF_INET6 to force IPv6
	hints.ai_socktype = SOCK_STREAM;

	if ( (rv = getaddrinfo( ip , "http" , &hints , &servinfo)) != 0)	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next)	{
		h = (struct sockaddr_in *) p->ai_addr;
		strcpy(ip, inet_ntoa( h->sin_addr ) );
	}
	
	printd(5, "ip: %s\r\n", ip);
	freeaddrinfo(servinfo); // all done with this structure
	return 0;
}

int parsing_konfig(char *s)	{
	int i=0;
	char a[50], b[128];
	const char *pch;
	pch = strchr(s, '#');
	if (pch!=NULL)	return 2;
	if (strlen(s)>1)	{
		sscanf(s,"%s = %s", a, b);
		printd(5, "  ----> a:>>%s<<, b:>>%s<<\r\n", a, b);
		if (!strcmp(a,"serialport"))	{	printd(5, "%s\r\n", b); strcpy(com_mod.comSer, b); }
		if (!strcmp(a,"baudrate"))	{		printd(5, "%s\r\n", b); com_mod.baud = atoi(b); }
		if (!strcmp(a,"soket"))	{			printd(5, "%s\r\n", b); sumber.socket = atoi(b); }
		
		if (!strcmp(a,"jmlsumber"))	{	
			printd(5, "%s\r\n", b);  sumber.jmlSumber = atoi(b); 
			printd(5, "======= ALOKASI MEMORI SUMBER dan DATA ========\r\n");
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
		if (!strcmp(a,"ipsumber"))	{
			printd(5, "isinya: %s = %s\r\n", a, b);
			i=iI+1;
			if (i>sumber.jmlSumber)	{
				printf(" --- ERROR Sumber LEBIH !!!\r\n ---\r\n");
				//return 0;
			} else {
				ipsumber[iI].no = i;
				ip_valid(b);
				strcpy(ipsumber[iI].ip, b); printd(5, "%s\r\n", b); 
				iI++;
			}
			//printd("i: %d, iI: %d\r\n", i, iI);
		}
		if (!strcmp(a,"idsumber"))	{
			printd(5, "iI: %d \r\n", iI);
			i=0;
			pch = strtok (b,",");
			while (pch != NULL)	{
				idData[(iI-1)*PER_SUMBER+i] = atoi(pch); 		//printd ("%s\n",pch);
				pch = strtok (NULL, ",");
				i++;
			}
		}
		
		if (!strcmp(a,"folderutama"))	{	printd(5, "%s\r\n", b); strcpy(sumber.folder, b); }
		if (!strcmp(a,"folderdata"))	{
			printd(5, "%s\r\n", b); strcpy(sumber.folderdata, b);
			sprintf(b, "%s%s", sumber.folder, sumber.folderdata);
			mkdir(b, 0755);
		}
		if (!strcmp(a,"file"))	{	printd(5, "%s\r\n", b); strcpy(sumber.file, b); }
		if (!strcmp(a,"periodesedot"))	{	printd(5, "%s\r\n", b); sumber.tSedot = waktu_atoi(b); }
		if (!strcmp(a,"periodefile"))	{	printd(5, "%s\r\n", b); sumber.tFile = waktu_atoi(b); }
		if (!strcmp(a,"periodekirim"))	{	printd(5, "%s\r\n", b); com_mod.tKirim = waktu_atoi(b); }
		if (!strcmp(a,"debug"))	{	printd(1000, "%s\r\n", b); g.debug = atoi(b); }
		if (!strcmp(a,"modul"))	{	printd(5, "%s\r\n", b); strcpy(g.modul, b); }

		if (!strcmp(a,"servertujuan"))	{	printd(5, "%s\r\n", b);
			strcpy(penerima.server, b); 
			ip_valid(b);
			strcpy(penerima.serverip, b); 
			//printf("IP server: %s\r\n", penerima.server);
		}
		
		if (!strcmp(a,"filetujuan"))	{	printd(5, "%s\r\n", b); strcpy(penerima.file, b); }
		if (!strcmp(a,"httppost"))	{	printd(5, "%s\r\n", b); 
			if ( (!strcmp(b,"yes")) || (!strcmp(b,"ya")) || (!strcmp(b,"1")) )
					penerima.httppost = 1;
			else 	penerima.httppost = 0;
		}
		if (!strcmp(a,"porthttppost"))	{	printd(5, "%s\r\n", b); penerima.porthttppost = atoi(b); }
		if (!strcmp(a,"ftp"))	{	printd(5, "%s\r\n", b); 
			if ( (!strcmp(b,"yes")) || (!strcmp(b,"ya")) || (!strcmp(b,"1")) )
					penerima.ftp = 1;
			else 	penerima.ftp = 0;
		}
	}
	return 1;	
}

void cek_konfig()	{
	int i=0, j;
	printf("\033[32m ++++++++    %s      ++++++++\r\n", APLIKASI);
	printf(" ==============================================================\r\n");
	printf("MODUL      : %s\r\n", g.modul);
	printf("SERIALPORT : %s\r\n", com_mod.comSer);
	printf("  baudrate : %d\r\n", com_mod.baud);
	printf("  socket   : %d\r\n", sumber.socket);
	printf("  Folder   : %s\r\n", sumber.folder);
	printf("  File     : %s\r\n", sumber.file);
	printf("t sedot    : %d\r\n", sumber.tSedot);
	printf("t File     : %d\r\n", sumber.tFile);
	printf("t kirim    : %d\r\n", com_mod.tKirim);
	printf("SERVER     : %s : %s\r\n", penerima.server, penerima.serverip);
	printf("  file     : %s\r\n", penerima.file);
	printf("  httppost : %s\r\n", (penerima.httppost==1)?"YA":"Tidak");
	printf("  port     : %d\r\n", penerima.porthttppost);
	printf("  ftp      : %s\r\n", (penerima.ftp==1)?"YA":"Tidak");
	printf("DEBUG      : %d\r\n", g.debug);
	
	printf("Jml Smbr   : %d\r\n", sumber.jmlSumber);
	for(i=0; i<sumber.jmlSumber; i++)	{
		printf("%3d. IP: %s\r\n", ipsumber[i].no, ipsumber[i].ip);
	}
	for(i=0; i<sumber.jmlSumber; i++)	{
		for (j=0; j<PER_SUMBER; j++)	{
			printf("id[%2d]: %4d --- ", i*PER_SUMBER+j, idData[i*PER_SUMBER+j]);
			if (!((j+1)%5))	printf("\r\n");
		}
	}
	printf("\033[0m");
}

void hitung_wkt(unsigned int w, int *wx)	{
	int aW[] = {1, 60, 60, 24, 365, 1};
	char i=0;
	
	for (i=0; i<5; i++)	{
		wx[i] = w /= (int) aW[i];
		//printf("wx[%d]: %d, w:%d, aW[%d]\r\n",i,  wx[i], w, i, aW[i]);
		if (wx[i]>=aW[i+1] && i<4)
			wx[i] %= aW[i+1];
		//printf("wx[%d]: %d\r\n",i,  wx[i]);
	}
}

int akses_file_konfig()		{
	int i=0, max=100;
	char line [100];
	pFile = fopen (FILENYA,"r");
	
	printd(5, "===> %s masuk !!\r\n", __FUNCTION__);
	if (pFile!=NULL)	{
		while ( fgets ( line, sizeof line, pFile ) != NULL )	{
			if (!parsing_konfig(line)) return 0;
			if (i>max)	break;
			i++;
		}
		fclose(pFile);
	}
	printd(5, "tutup FILE\r\n");
	return i;
}

void init_var()		{
	char kata[20];
	iI = 0;
	aa = 02;
	bb = 1;
	
	g.debug = 100000;
	g.st_thread = 0;

	sumber.tSedot  = 1;		// tiap 1 detik
	sumber.tFile   = 60;	// tiap 1 menit
	com_mod.tKirim = 3600;	// tiap 1 jam

	signal(SIGINT, sig_int);
	//signal(SIGQUIT, sig_keluar);
	signal(SIGPIPE, sig_pipe);

	pFile = fopen (PID_FILE,"a+");
	if (pFile!=NULL)	{
		sprintf(kata, "%d", getpid());
		fputs (kata, pFile);
		fclose (pFile);
	}

	time ( &xtime );
	wfile = localtime ( &xtime );
	sprintf(kata, "%02d:%02d:%02d ", wfile->tm_hour, wfile->tm_min, wfile->tm_sec);
	printf("waktu : %s\r\n", kata);
}

#if 1
int buka_soket_satuan(int i)	{
	struct sockaddr_in server_m;
	printd(100, "~~~~~ %s masuk: soket: %d/%d\r\n", __FUNCTION__, ipsumber[i].socket, ipsumber[i].socket_desc);
	switch (ipsumber[i].socket)	{
		case -1: // buka lokal socket
			printd(5, " +++++ buat socket dulu !!\r\n");
			ipsumber[i].socket_desc = socket(AF_INET , SOCK_STREAM , 0);
			printd(5, "socket desc: %d\r\n", ipsumber[i].socket_desc);
			ipsumber[i].socket = 1;
		case 2:	 // konek socket ke tujuan
			printd(5, " +++++ coba buka soket %d/%d\r\n", i, ipsumber[i].socket_desc);
			server_m.sin_addr.s_addr = inet_addr(ipsumber[i].ip);
			server_m.sin_family = AF_INET;
			server_m.sin_port = htons( sumber.socket );
			ipsumber[i].socket = connect(ipsumber[i].socket_desc, (struct sockaddr *)&server_m , sizeof(server_m));
			if (ipsumber[i].socket >= 0)		{
				printd(5, "Connected buka mandiri: %d\r\n", ipsumber[i].socket_desc);
				ipsumber[i].stat_konek = 1;
				ipsumber[i].socket = 2;
			} else {
				printd(5, "Tidak nyambung !!! TUTUP SOKET %d !!!\r\n", ipsumber[i].socket_desc);
				ipsumber[i].stat_konek = 0;
				ipsumber[i].jeda = JEDA;
				close(ipsumber[i].socket_desc);
			}
	}
	return 1;
}
#endif

#if 0
int buka_soket()	{
	int i;
	//int socket_desc;
	struct sockaddr_in server_m[sumber.jmlSumber];
	
	printd(5, "===> %s masuk...\r\n", __FUNCTION__);
	
	for (i=0; i<sumber.jmlSumber; i++)	{
		ipsumber[i].socket_desc = socket(AF_INET , SOCK_STREAM , 0);
		printd(5, "socket desc: %d\r\n", ipsumber[i].socket_desc);
		ipsumber[i].socket = 1;
		if (ipsumber[i].socket_desc == -1)	{
			printd(5, "Could not create socket");
		} else {
			server_m[i].sin_addr.s_addr = inet_addr(ipsumber[i].ip);
			server_m[i].sin_family = AF_INET;
			server_m[i].sin_port = htons( sumber.socket );
			
			printd(5, "Connect to remote server\r\n");
			ipsumber[i].socket = connect(ipsumber[i].socket_desc, (struct sockaddr *)&server_m[i] , sizeof(server_m[i]));
			
			if (ipsumber[i].socket >= 0)		{
				printd(5, "Connected %d\r\n", ipsumber[i].socket);
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
#endif

int ambil_data_satuan(int no)	{
	struct t_xdata st_data;
	char message[20] , s_reply[300];

	iI = no;		strcpy(s_reply, "");
	printd(5, " %s masuk\r\n", __FUNCTION__);
	int jmlNSock, i;

	sprintf(message, "sampurasun%d", 1);
	printd(5, "------------%s ... sock desc[%d]: %d, pesan: %s\r\n", __FUNCTION__, no, ipsumber[no].socket_desc, message);

	jmlNSock = send(ipsumber[no].socket_desc, message, strlen(message), 0);
	if (jmlNSock < 0)    {
        printf("Send failed %s", ipsumber[no].ip);
        return 1;
    }
    
    if (jmlNSock>0)	{
		jmlNSock = recv(ipsumber[no].socket_desc, s_reply, 300 , 0);
		if( jmlNSock <= 0)    {
			printf("recv failed %s", ipsumber[no].ip);
			//sleep(2);
			return 2;
		}
		printd(5, "Reply received: %d/%d/%d ==== %d\n", jmlNSock, sizeof(st_data), sizeof(data_f), no*PER_SUMBER);
		printd(5, s_reply);
		
		memcpy ( &st_data, &s_reply, jmlNSock );
		memcpy( (char *) &data_f[no*PER_SUMBER], st_data.buf, PER_SUMBER*sizeof(float) );
		
		printd(5, "==> no: %d === %s >> urut:%d >> flag:%d >> alamat:%d\r\n", no, st_data.mon, st_data.nomer, st_data.flag, st_data.alamat);
		for(i=0; i<PER_SUMBER; i++)		{
			printd(5, "[%d]: %.1f ", no*PER_SUMBER+i, data_f[no*PER_SUMBER+i]);
		}
		printd(5, "\r\n");
		return 0;
	}
	printf("keluar JALUR %s tutup soket !!!\r\n", __FUNCTION__);
	return 100;
}

int ambil_mandiri(int no)	{
	struct sockaddr_in server_m;
	
	printd(5, "===== no: %d,  %s masuk !!\r\n", no, __FUNCTION__);
	ipsumber[no].socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (ipsumber[no].socket_desc == -1)	{
		printd(5, "Could not create socket");
		return 1;
	} else {
		server_m.sin_addr.s_addr = inet_addr(ipsumber[no].ip);
		server_m.sin_family = AF_INET;
		server_m.sin_port = htons( sumber.socket );
		
		printd(5, "Connect to server socket : %d\r\n", ipsumber[no].socket_desc);
		ipsumber[no].socket = connect(ipsumber[no].socket_desc, (struct sockaddr *)&server_m , sizeof(server_m));
		
		if (ipsumber[no].socket >= 0)		{
			printd(5, "Connected %d\r\n", ipsumber[no].socket);
			ipsumber[no].stat_konek = 1;
			ambil_data_satuan(no);
		} else {
			printd(5, "########## GAGAL Connected %d\r\n", ipsumber[no].socket);
		}
	}
	close(ipsumber[no].socket_desc);
	return 0;
}

void ambil_data()	{
	int i=0;
	printd(5, "  ---%3d/%d : Ambil DATA: %d\r\n", ++iI, sumber.jmlSumber, aa++);
	for (i=0; i<sumber.jmlSumber; i++)	{
		printd(5, " %d/%d --> \"%s\" ", ipsumber[i].no, sumber.jmlSumber, ipsumber[i].ip);
		ambil_mandiri(i);
	}
}

int cek_file()	{
	//int w[6], i, baru=0;
	double beda;
	
	char wx[30];
	
	time_t now;
	time ( &now );
	
	sprintf(wx, "%d-%02d-%02d %02d:%02d:%02d", wfile->tm_year+1900, wfile->tm_mon+1, wfile->tm_mday, \
		wfile->tm_hour, wfile->tm_min, wfile->tm_sec);
	printd(5, "wx: %s  ------", wx);
	
	beda = difftime (now, xtime);
	printd(1000, "----------------------beda: %.2lf / %d\r\n", beda, sumber.tFile);
	
	if ((int) beda >= sumber.tFile)	{
		time ( &xtime );
		return 1;
	}
	
	return 0;
}

int nama_file_simpan(char *namafile)	{
	int baru=0;
	
	printd(5, "++++++++++= masuk %s\r\n", __FUNCTION__);
	char nfbaru[30], *pchak;
	time_t rawtime;
	struct tm * ti;

	time ( &rawtime );

	pchak = strrchr(sumber.file, '.');
	baru = pchak-sumber.file;
	strncpy (nfbaru, sumber.file, baru);
	nfbaru[baru] = 0;
	
	if ( (nfbaru==NULL) || (pchak==NULL) )		return 0;

	baru = cek_file();
	printd(5, "cek file: %d : %d\r\n", baru, pchak-sumber.file);
	if (baru)	{
		ti = localtime ( &rawtime );
		sprintf(nfbaru, "%s.%d%02d%02d.%02d%02d%s", nfbaru, ti->tm_year+1900, ti->tm_mon+1, ti->tm_mday, ti->tm_hour, \
			ti->tm_min, pchak);
		printd(5, "=========== BUAT NAMA BARU !!!!!\r\n");
		baru = 1;
	} else	{
		wfile = localtime ( &xtime );
		sprintf(nfbaru, "%s.%d%02d%02d.%02d%02d%s", nfbaru, wfile->tm_year+1900, wfile->tm_mon+1, wfile->tm_mday, \
			wfile->tm_hour, wfile->tm_min, pchak);
	}
	//printd(1000, "namafile: %s\r\n", nfbaru);
	strcpy(namafile, nfbaru);

	return baru;
}

int simpan_ke_file()	{
	char isifile[512], perk[30], sl[128];
	int i, j=0, baru;
	
	time_t now;
	struct tm * tix;
	struct stat sts;
	
	printd(5, "_______%s: %d === %d\r\n", __FUNCTION__, PER_SUMBER*sumber.jmlSumber, bb++);
	
	//getcwd(isifile, 512);
	//printf("pwd: %s/%s\r\n", isifile, sumber.folder);
	sprintf(sl, "%s%s", sumber.folder, sumber.folderdata);
	if (chdir(sl))	{
		//printf("&*()*&*()_ GAGAL chdir \r\n");
		printf("____ FOLDER TIDAK DITEMUKAN ____\r\n");
		return 1;
	}

	baru = nama_file_simpan(perk);
	printd(5, "file baru: %s\r\n", perk);
	sprintf(sl, "%s%s/%s", sumber.folder, sumber.folderdata, perk);
	
	i = stat (sl, &sts);
	printd(5, "i: %d, sl %s\r\n", i, sl);
	if (i == -1)	{
		printd (5, "File %s belum ada...\n", perk);
		baru = 1;
	}
	
	time ( &now );
	tix = localtime ( &now );
	
	pFile = fopen (perk,"a+");
	if (pFile!=NULL)	{
		if (baru)	{
			printd(5, "========== file baru +++++++++++++++\r\n");
			sprintf(perk, "waktu ");
			strcpy(isifile, perk);
			for (i=0; i<(PER_SUMBER*sumber.jmlSumber); i++)	{
				if (idData[i]>0)	{
					sprintf(perk, "%d ", idData[i]);
					strcat(isifile, perk);
				}
			}
			strcat(isifile, "\r\n");
		} else {
			strcpy(isifile, "");
		}

		sprintf(perk, "%02d:%02d:%02d ", tix->tm_hour, tix->tm_min, tix->tm_sec);
		strcat(isifile, perk);
		printd(1000, "waktu: %s\r\n", isifile);
		for (i=0; i<(PER_SUMBER*sumber.jmlSumber); i++)	{
			if (idData[i]>0)	{
				sprintf(perk, "%.1f ", data_f[i]);
				//printf("[%d]: %.2f ", idData[i], data_f[i]);
				strcat(isifile, perk);
				j++;
			}
		}
		strcat(isifile, "\r\n");
		printd(5, "\r\n Data disimpan: %d\r\n", j);
		fputs (isifile, pFile);
		fclose (pFile);
	}
	return 0;
}

int kirim_httpport(char *ip, char *sfile)	{
	int sockfd, n, lSize=0, awal=0, akhir=0;
	struct sockaddr_in servaddr;
	struct stat sts;
	
	char * buffer, header[MAX_ISI], *paket;
	
	printf("ip: %s, file: %s\r\n", ip, sfile);
	
	if( (sockfd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP)) == -1 ){
        printf("socket: error");
        exit(0);
    }
    
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(penerima.porthttppost);  //connect to http server
    
    if(inet_pton(AF_INET, ip, &servaddr.sin_addr) < 0){
        printf("port: assigned invalid");
        //exit(0);
        return 1;
    }
    if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) == -1)	{
        printf("connect: error");
        //exit(0);
        return 2;
    }
    
    sprintf(header, "%s%s", sumber.folder, sumber.folderdata);
    if (chdir(header))	{
		//printf("&*()*&*()_ GAGAL chdir \r\n");
		printf("____ FOLDER TIDAK DITEMUKAN ____\r\n");
		return 3;
	}

	sprintf(header, "%s%s/%s", sumber.folder, sumber.folderdata, sfile);
	n = stat (header, &sts);
	printd(1000, "n: %d, sl %s\r\n", n, header);
	if (n == -1)	{
		printd (5, "File %s Tidak ada...\n", sfile);
		return 4;
	}
	//printf("header: %s\r\n", header);

    pFile = fopen (sfile, "rb");
    //strcpy(isifile, "");
    if (pFile!=NULL)	{
		//printf("ada file !!\r\n");
		
		fseek(pFile, 0, SEEK_END);
		lSize = ftell (pFile);
		rewind(pFile);
		//printf("ukuran: %d\r\n", lSize);
		
		buffer = (char*) malloc (sizeof(char)*lSize);
		n = fread (buffer, 1, lSize, pFile);
		if (n != lSize) {
			fputs ("Reading error",stderr);
			fclose (pFile);
			return 5;
		}
		fclose (pFile);
	}
	
	//printf("%s---------------stlh close .....\r\n", buffer);

    bzero(&header,sizeof(header));
    n += strlen(sfile) + 287;
    sprintf(header,"POST %s HTTP1.1\r\nAccept: */*\r\nUser-Agent: BalungKirik/1.0\r\n", penerima.file);
	sprintf(header,"%sContent-Type: multipart/form-data; boundary=B4LunK1r1K\r\n", header);
	//sprintf(header,"%sAccept-Encoding: gzip, deflate\r\n", header);
	sprintf(header,"%sContent-Length: %d\r\n\r\n", header, n);
	sprintf(header,"%s\r\n%c", header, 0);
	
	sprintf(header,"%s--B4LunK1r1K\r\n", header);
	sprintf(header,"%sContent-Disposition: form-data; name=\"nilai1\"\r\n", header);
	sprintf(header,"%s\r\n%c", header, 0);
	
	sprintf(header,"%sInidanItu\r\n", header);
	sprintf(header,"%s--B4LunK1r1K\r\n", header);
	sprintf(header,"%sContent-Disposition: form-data; name=\"file\"; filename=\"%s\"\r\n", header, sfile);
	sprintf(header,"%sContent-Type: text/plain\r\n", header);
	sprintf(header,"%s\r\n%c", header, 0);
	
	printf("ISIFILE: \r\n%s\r\n", header);
	//printf("%s\r\n", buffer);
	
	awal = strlen(header);
	//memcpy(paket, header, awal+1);
	memmove(paket+awal, buffer, lSize+1);
	//memcpy(buffer+awal, buffer, lSize);
	//memmove(paket, header, awal);
	
	bzero(&header,sizeof(header));
	sprintf(header,"%s--B4LunK1r1K\r\n%c", header, 0);
	akhir = strlen(header);
	memmove(buffer+awal+lSize, header, akhir+1);
	//memcpy(buffer+awal+lSize, header, akhir+1);
	buffer[awal+lSize+akhir] = 0;

    if (write(sockfd, buffer, strlen(buffer)+1) == -1)
        printf("write");
    
    printf("--------------------------------data kirim\r\n%s\r\n", buffer);

    bzero(&header, sizeof(header));	//strcpy(header, "");
    while ((n = read(sockfd, header, sizeof(header))) > 0){
		header[n] = 0;
		//printf("recv: %s\r\n", header);
		if(fputs(header,stdout) == EOF)
			printf("read error");            
    }
    
    close(sockfd);
    free (buffer);
    return 0;
}

int keluar_th(pthread_mutex_t *mtx)	{
	switch(pthread_mutex_trylock(mtx)) {
		case 0: /* if we got the lock, unlock and return 1 (true) */
			pthread_mutex_unlock(mtx);
			return 1;
		case EBUSY: /* return 0 (false) if the mutex was locked */
			return 0;
	}
	return 1;
}

void bagi_waktu(struct tm *tx, char *w, int pilih)	{
	char tmp[6];
	switch (pilih)	{
		case TGL:
			//printf("====tgl: %s\r\n", w);
			strncpy(tmp, &w[6], 2); tmp[2] = 0;		tx->tm_mday = atoi(tmp);
			strncpy(tmp, &w[4], 2); tmp[2] = 0;		tx->tm_mon = atoi(tmp) - 1;
			strncpy(tmp, &w[0], 4); tmp[4] = 0;		tx->tm_year = atoi(tmp) - 1900;
			//printf("******* tgl: %d-%d-%d\r\n", tx->tm_mday, tx->tm_mon, tx->tm_year);
			break;
		case JAM:
			//printf("====wkt: %s\r\n", w);
			strncpy(tmp, &w[0], 2); tmp[2] = 0;		tx->tm_hour = atoi(tmp);
			strncpy(tmp, &w[2], 2); tmp[2] = 0;		tx->tm_min = atoi(tmp);
			//printf("******* jam: %d:%d\r\n", tx->tm_hour, tx->tm_min);
			break;
		default:
			break;
	}
}

void *kirim_paket(void *argv)	{
	pthread_mutex_t *mx = argv;
	int l=0, i=0, hkirim=0, nbeda=0;
	char ipnya[50], stmp[100], nf[30];
	char tgl[10], ww[10];
	char *pch;
	double beda;
	DIR *dir;
	struct dirent *ent;
	
	time_t xraw, filetime;
	struct tm * tn, tf;
	time ( &xraw );
	//tn = localtime ( &xraw );

	sprintf(stmp, "%s%s", sumber.folder, sumber.folderdata);
	printd(1000, "---------- MASUK THREAD : %d KIRIM PAKET\r\n", counter++);

	while( !keluar_th(mx) ) {
		sleep(1);
		l++;
		if (l >= com_mod.tKirim)	{
			dir = opendir(stmp);
			if (dir != NULL) {
				while ((ent = readdir (dir)) != NULL) {
					strcpy(nf, ent->d_name);
					//printf("===========================================%s\r\n", nf);
					
					if (strlen(nf)<5) continue;
					i = 0;
					
					pch = strtok(nf, ".");
					//printf ("ISINYA: %s -- %s -- %s\n", ent->d_name, nf, pch);
					while (pch != NULL)	{
						//printf("pch: %s\r\n", pch);
						if (i==1)	{	strcpy(tgl, pch);	bagi_waktu(&tf, pch, TGL);	}
						if (i==2)	{	strcpy(ww, pch);	bagi_waktu(&tf, pch, JAM);	}
						pch = strtok(NULL, ".");
						i++;
					}
					
					filetime = mktime(&tf);
					beda = difftime(xraw, filetime);
					nbeda = (int) beda;
					//printf("beda: %.2d  == %.2f \r\n", nbeda, beda);

					if (beda > JEDA_NOW)	{
						//printf("KIRIM PKET FILE INI !!!\r\n");
						
						hkirim = kirim_httpport(penerima.serverip, ent->d_name);
						if (hkirim>0)	{
							strcpy(ipnya, penerima.server);
						//	ip_valid(ipnya);
						}
					}
				}
				closedir (dir);
			}
			l = 0;
		}
	}

	printd(100, "----===> %s KELUAR LOOP THREAD !!!\r\n", __FUNCTION__);
	return NULL;	
}

int main(int argc , char *argv[])	{
	int i, kk=0;
	
	init_var();
	
	i = akses_file_konfig();	printd(5, " Akses konfig selesai %d\r\n", i);
	cek_konfig();	printd(5, " Cek konfig selesai %d/%d\r\n", iI, sumber.jmlSumber);
	//buka_soket();
	iI = 0;
	counter = 0;

	time_t rawtime;
	struct tm * ti;

	time ( &rawtime );
	ti = localtime ( &rawtime );
	printd(1000, "waktu : %d-%d-%d %d:%d:%d\r\n", ti->tm_mday, ti->tm_mon, ti->tm_year, ti->tm_hour, ti->tm_min, ti->tm_sec);
	
	#if 0
	buka_sendiri(0);
	#endif
	
	pthread_mutex_init(&mxq,NULL);
	pthread_mutex_lock(&mxq);
	
	printf("Parent PID(%d): Mulai sedot !!...\n", getpid());
	
	if ( !pthread_create(&kirim_thread, NULL, kirim_paket, &mxq) ) {		// sukses = 0
		printf("======= Buat Thread\r\n");
		g.st_thread = 1;
	}

	#if 1
	while(1)	{
		ambil_data();
		simpan_ke_file();
		sleep(sumber.tSedot);
		kk++;
	}
	#endif
	printf("KELUAR LOOP UTAMA conter: %d\r\n", counter);

	sig_int(1);
	//siginthandler(1);
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
    printd(100, "Data Send\n");
	
	jmlNSock = recv(ipsumber[i].socket_desc, s_reply , 300 , 0);
    if( jmlNSock < 0)    {
        puts("recv failed");
    }
	
	printd(100, "Reply received: %d/%d/%d\n", jmlNSock, sizeof(st_data), sizeof(data_f));
    printd(100, "%s\r\n", s_reply);
	
	memcpy ( &st_data, &s_reply, jmlNSock );
	memcpy( (char *) &data_f[0], st_data.buf, PER_SUMBER*sizeof(float) );
    
    printd(100, "==> %s >> urut:%d >> flag:%d >> alamat:%d\r\n", st_data.mon, st_data.nomer, st_data.flag, st_data.alamat);
	
	for(i=0; i<PER_SUMBER; i++)		{
		printd(100, "%.2f ", data_f[i]);
	}
	printd(100, "\r\n");
	
	printf("Rehat dulu !!!\r\n");   
	close(ipsumber[i].socket_desc);
	sleep(1);
	return 0;

}
