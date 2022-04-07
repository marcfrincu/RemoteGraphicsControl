
/*
API prototypes 
*/
void rgcManageOperations(void);
int rgcTakeScreenshotJPG(unsigned int, unsigned int, char*, int);
void rgcTakeScreenshotBMP(int, int, char*);
void rgcReadTaskFile(char*);
long rgcGetTime(void);
long rgcGetTimeLastTask(void);
void rgcManageIdle(void);
void rgcGetAppStartTime(void);
void rgcPrintScene(void);
