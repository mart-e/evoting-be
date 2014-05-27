int prockint(/*  void  */);
int proscopn(/*  void  */);
int proclear(/*  void  */);
int prosccls(/*  int   */);
int promsgd(/*  char *  */);
int prordl(/*  char *,int ,int *,int *  */);
int prordc(/*  char *,int ,char *,int *,int ,int *  */);
int prordd(/*  char *,int ,int *,int *,int *,int *  */);
int prordi(/*  char *,int ,long *,int *  */);
int prordn(/*  char *,int ,char *,int *,int ,int *  */);
int prordr(/*  char *,int ,long *,int *  */);
int prowtl(/*  char *,int ,int ,int   */);
int prowtc(/*  char *,int ,char *,int   */);
int prowtd(/*  char *,int ,int ,int ,int ,int   */);
int prowti(/*  char *,int ,long ,int   */);
int prowtn(/*  char *,int ,char *,int   */);
int prowtr(/*  char *,int ,long ,int   */);
int prowtbl(/*  char *,int ,int ,int ,int   */);
int prowtbc(/*  char *,int ,int ,char *,int   */);
int prowtbd(/*  char *,int ,int ,int ,int ,int ,int   */);
int prowtbi(/*  char *,int ,int ,long ,int   */);
int prowtbn(/*  char *,int ,int ,char *,int   */);
int prowtbr(/*  char *,int ,int ,long ,int   */);
int prordbl(/*  char *,int ,int ,int *,int *  */);
int prordbc(/*  char *,int ,int ,char *,int *,int ,int *  */);
int prordbd(/*  char *,int ,int ,int *,int *,int *,int *  */);
int prordbi(/*  char *,int ,int ,long *,int *  */);
int prordbn(/*  char *,int ,int ,char *,int *,int ,int *  */);
int prordbr(/*  char *,int ,int ,long *,int *  */);
int profldix(/*  char *,char *  */);

#define prosleep(secs)		(utsleep(secs))
#define proevt(secs, pflag)	(utevt((secs), (pflag)))
#define prowait(pflag)		(utwait(pflag))
#define procncel(pflag)		(utcncel(pflag))
