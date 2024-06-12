#if defined(DEBUG)
	#define DBUG(x,...) IExec->DebugPrintF("[%s:%-4ld] "x ,__FILE__,__LINE__, ##__VA_ARGS__)
	#define DBUGN(x,...) IExec->DebugPrintF(""x ,##__VA_ARGS__)
#else
	#define DBUG(x,...)
	#define DBUGN(x,...)
#endif
