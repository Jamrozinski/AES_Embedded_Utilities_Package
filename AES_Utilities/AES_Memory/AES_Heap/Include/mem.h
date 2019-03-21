#ifndef __mem_h__
#define __mem_h__

#include "stdint.h"
#include "AES_Common_Definitions.h"
#include "fsl_shell.h" /* not a fan of having external dependence -- perhaps eventually we create our own internal shell */

/******************************************************************************************/
/* port specific print function headers*/
/* port: K64 using KDS fsl_shell.h */
#define XPRINTF_CONTAINER   p_shell_context_t /* from fsl_shell.h */
/******************************************************************************************/

#define CONFIG_AES_HEAP_SIZE        ((size_t)(8 * AES_KILOBYTE)) /* USER MODIFIABLE */

int Mem_Init( void );
void * Mem_Alloc(int size);
int Mem_Free(void *ptr);
void Mem_Dump();
int Mem_Uninit( void );

#endif // __mem_h__


