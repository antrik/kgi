void *xf86MapVidMem(int ScreenNum,int Flags, unsigned long Base, unsigned long Size);
void xf86UnMapVidMem(int ScreenNum,void *Base,unsigned long Size);
int xf86EnableIO(void);
void xf86DisableIO(void);
