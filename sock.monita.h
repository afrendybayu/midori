
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
};

struct t_ipsumber		{
	int no;
	char ip[16];
	int socket;
	int jeda;				// delay ketika socket mati
};

struct t_tuj	{
	int jmlSumber;
	int socket;
};

struct t_modem com_mod;
struct t_tuj sumber;
struct t_ipsumber *ipsumber;
float data_f[PER_SUMBER];
FILE * pFile;


int iI;
int printd(const char *format, ...);
int akses_file_konfig(void);
