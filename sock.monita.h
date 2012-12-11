
#define PER_SUMBER		20
#define FILENYA			"mon_konfig.cfg"



struct t_xdata {
	char mon[8];				// id bahwa data monita
    unsigned int nomer;		// urutan pengiriman data
    unsigned char flag;		// belum di definisikan
    unsigned char alamat;		// alamat stacking board (address power meter)
	char buf[256];				// digenapi sekalian
	//float datax[60];
};

struct t_modem	{
	int baud;
	char comSer[10];
	int tKirim;
};

struct t_ipsumber		{
	int no;
	char ip[16];
	int socket_desc;		// koneksi socket tiap modul
	int socket;				// 
	int jeda;				// delay ketika socket mati --> tidak jadi karena soket langsung ditutup
	int stat_konek;
};

struct t_tuj	{
	int jmlSumber;
	int socket;				// socket modul monita
	int tSedot;
	int tSimpan;
	char folder[20];
	char file[20];
};

struct t_modem com_mod;
struct t_tuj sumber;
//volatile static 
struct t_ipsumber *ipsumber;
float *data_f;
int *idData;
//float data_f[PER_SUMBER];
FILE * pFile;

int aa, bb;
int iI, debug;
struct tm * wfile;

void siginthandler(int param);
void sig_pipe(int signum);

void cek_konfig();
void ambil_data();
int buka_sendiri();
int waktu_atoi(char *waktu);

int printd(int prio, const char *format, ...);
int akses_file_konfig(void);
int parsing_konfig(char *s);
int buka_soket_satuan(int i);
int buka_soket();
int ambil_data_satuan(int no);
int parsing_data_satuan(int no);

