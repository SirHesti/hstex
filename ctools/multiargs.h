/**	
 @file multiargs.h
 @brief Header fuer multiargs.c
*//** 
 @addtogroup c_multi_args
 @{ */
#ifndef MULTIARGS_H
#define MULTIARGS_H
#ifdef  __cplusplus
extern "C" {
#endif
int GetCMDLineParameter(int argc, char *argv[], char *fmt,...);
char *ReadConfigLineFromFile(char *cfg_name,int cfg_max_args);
int ReadVarsConfigFile(char *cfg_name, char *fmt, ...);
#ifdef  __cplusplus
}
#endif
#endif
///@}
/*EOF*/
