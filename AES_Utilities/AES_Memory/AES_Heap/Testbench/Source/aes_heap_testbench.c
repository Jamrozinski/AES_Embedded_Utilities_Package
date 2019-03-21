/*
 * aes_mem_manager_testbench.c
 *
 *  Created on: Jan 14, 2019
 *      Author: AES_Local
 */
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include "../../Include/mem.h"

/*************************************************************/
/* port specific print function headers*/
/* port: K64 using KDS fsl_shell.h */

#include "fsl_shell.h"
#include "fsl_shell_usr.h"

XPRINTF_CONTAINER container;
/*************************************************************/

/* series of ported tests to validate memory */
/* TODO: consolidate these tests and make them more 'dynamic' - less statically defined variables; let rand() do the work */
/* TODO: override with AES_Assert */

int tb_mem_manager_test_alloc_0() {

   assert(Mem_Init() == 0);

   void* ptr = Mem_Alloc(8);
   assert(ptr != NULL);

   assert(Mem_Uninit() == 0);

   return 1;
}

int tb_mem_manager_test_alloc_1() {

   assert(Mem_Init() == 0);

   assert(Mem_Alloc(4) != NULL);
   assert(Mem_Alloc(8) != NULL);
   assert(Mem_Alloc(16) != NULL);
   assert(Mem_Alloc(4) != NULL);

   assert(Mem_Uninit() == 0);

   return 1;
}

int tb_mem_manager_test_alloc_2() {

   assert(Mem_Init() == 0);

   assert(Mem_Alloc(1) != NULL);
   assert(Mem_Alloc(5) != NULL);
   assert(Mem_Alloc(14) != NULL);
   assert(Mem_Alloc(8) != NULL);
   assert(Mem_Alloc(1) != NULL);
   assert(Mem_Alloc(4) != NULL);
   assert(Mem_Alloc(9) != NULL);
   assert(Mem_Alloc(33) != NULL);
   assert(Mem_Alloc(55) != NULL);

   Mem_Dump(container);

   assert(Mem_Uninit() == 0);

   return 1;
}

int tb_mem_manager_test_alloc_no_space_0() {

	   assert(Mem_Init() == 0);
	   assert(Mem_Alloc(CONFIG_AES_HEAP_SIZE - 1) == NULL);

	   assert(Mem_Uninit() == 0);

	   return 1;
}

int tb_mem_manager_test_alloc_no_space_1() {

   assert(Mem_Init() == 0);
   assert(Mem_Alloc((CONFIG_AES_HEAP_SIZE / 2) - 1) != NULL);
   assert(Mem_Alloc((CONFIG_AES_HEAP_SIZE / 2)) == NULL);

   assert(Mem_Uninit() == 0);

   return 1;
}

int tb_mem_manager_test_align_0() {

   assert(Mem_Init() == 0);

   int* ptr = (int*) Mem_Alloc(sizeof(int));
   assert(ptr != NULL);
   assert((int)ptr % 4 == 0);
   Mem_Dump(container);
   assert(Mem_Uninit() == 0);

   return 1;
}

int tb_mem_manager_test_align_1() {

   assert(Mem_Init() == 0);

   int* ptr[4];

   ptr[0] = (int*) Mem_Alloc(4);
   ptr[1] = (int*) Mem_Alloc(8);
   ptr[2] = (int*) Mem_Alloc(16);
   ptr[3] = (int*) Mem_Alloc(4);

   assert((int)(ptr[0]) % 4 == 0);
   assert((int)(ptr[1]) % 4 == 0);
   assert((int)(ptr[2]) % 4 == 0);
   assert((int)(ptr[3]) % 4 == 0);

   assert(Mem_Uninit() == 0);

   return 1;

}

int tb_mem_manager_test_align_2() {

   assert(Mem_Init() == 0);

   int* ptr[9];
   ptr[0] = Mem_Alloc(1);
   ptr[1] = (Mem_Alloc(5));
   ptr[2] = (Mem_Alloc(14));
   ptr[3] = (Mem_Alloc(8));
   ptr[4] = (Mem_Alloc(1));
   ptr[5] = (Mem_Alloc(4));
   ptr[6] = (Mem_Alloc(9));
   ptr[7] = (Mem_Alloc(33));
   ptr[8] = (Mem_Alloc(55));

   assert((int)(ptr[0]) % 4 == 0);
   assert((int)(ptr[1]) % 4 == 0);
   assert((int)(ptr[2]) % 4 == 0);
   assert((int)(ptr[3]) % 4 == 0);
   assert((int)(ptr[4]) % 4 == 0);
   assert((int)(ptr[5]) % 4 == 0);
   assert((int)(ptr[6]) % 4 == 0);
   assert((int)(ptr[7]) % 4 == 0);
   assert((int)(ptr[8]) % 4 == 0);

   assert(Mem_Uninit() == 0);

   return 1;
}

int tb_mem_manager_test_coalesce_0() {

   assert(Mem_Init() == 0);
   void * ptr[4];

   ptr[0] = Mem_Alloc(800);
   assert(ptr[0] != NULL);

   ptr[1] = Mem_Alloc(800);
   assert(ptr[1] != NULL);

   ptr[2] = Mem_Alloc(800);
   assert(ptr[2] != NULL);

   ptr[3] = Mem_Alloc(800);
   assert(ptr[3] != NULL);

   while (Mem_Alloc(800) != NULL)
     ;

   assert(Mem_Free(ptr[1]) == 0);
   assert(Mem_Free(ptr[2]) == 0);

   ptr[2] = Mem_Alloc(1600);
   assert(ptr[2] != NULL);

   assert(Mem_Uninit() == 0);

   return 1;
}

int tb_mem_manager_test_coalesce_1() {

   assert(Mem_Init() == 0);
   void * ptr[4];

   ptr[0] = Mem_Alloc(800);
   assert(ptr[0] != NULL);

   ptr[1] = Mem_Alloc(800);
   assert(ptr[1] != NULL);

   ptr[2] = Mem_Alloc(800);
   assert(ptr[2] != NULL);

   ptr[3] = Mem_Alloc(800);
   assert(ptr[3] != NULL);

   while (Mem_Alloc(800) != NULL)
     ;

   assert(Mem_Free(ptr[2]) == 0);
   assert(Mem_Free(ptr[1]) == 0);

   ptr[2] = Mem_Alloc(1600);
   assert(ptr[2] != NULL);

   Mem_Dump(container);
   assert(Mem_Uninit() == 0);

   return 1;
}

int tb_mem_manager_test_coalesce_2() {

   assert(Mem_Init() == 0);
   void * ptr[5];

   ptr[0] = Mem_Alloc(600);
   assert(ptr[0] != NULL);

   ptr[1] = Mem_Alloc(600);
   assert(ptr[1] != NULL);

   ptr[2] = Mem_Alloc(600);
   assert(ptr[2] != NULL);

   ptr[3] = Mem_Alloc(600);
   assert(ptr[3] != NULL);

   ptr[4] = Mem_Alloc(600);
   assert(ptr[4] != NULL);

   while (Mem_Alloc(600) != NULL)
     ;

   assert(Mem_Free(ptr[1]) == 0);
   assert(Mem_Free(ptr[3]) == 0);
   assert(Mem_Free(ptr[2]) == 0);

   ptr[2] = Mem_Alloc(1800);
   assert(ptr[2] != NULL);

   assert(Mem_Uninit() == 0);

   return 1;
}

int tb_mem_manager_test_coalesce_3() {

   assert(Mem_Init() == 0);
   void * ptr[7];

   ptr[0] = Mem_Alloc(500);
   assert(ptr[0] != NULL);

   ptr[1] = Mem_Alloc(500);
   assert(ptr[1] != NULL);

   ptr[2] = Mem_Alloc(500);
   assert(ptr[2] != NULL);

   ptr[3] = Mem_Alloc(500);
   assert(ptr[3] != NULL);

   ptr[4] = Mem_Alloc(500);
   assert(ptr[4] != NULL);

   ptr[5] = Mem_Alloc(500);
   assert(ptr[5] != NULL);

   ptr[6] = Mem_Alloc(500);
   assert(ptr[6] != NULL);

   //while (Mem_Alloc(500) != NULL)
    // ;

   assert(Mem_Free(ptr[1]) == 0);
   assert(Mem_Free(ptr[5]) == 0);
   assert(Mem_Free(ptr[2]) == 0);
   assert(Mem_Free(ptr[4]) == 0);
   assert(Mem_Free(ptr[3]) == 0);

   Mem_Dump(container);
   ptr[2] = Mem_Alloc(2500);
   assert(ptr[2] != NULL);

   Mem_Dump(container);
   assert(Mem_Uninit() == 0);

   return 1;
}

int tb_mem_manager_test_coalesce_4() {

   assert(Mem_Init() == 0);
   void * ptr[4];

   ptr[0] = Mem_Alloc(800);
   assert(ptr[0] != NULL);

   ptr[1] = Mem_Alloc(800);
   assert(ptr[1] != NULL);

   ptr[2] = Mem_Alloc(800);
   assert(ptr[2] != NULL);

   ptr[3] = Mem_Alloc(800);
   assert(ptr[3] != NULL);

   while (Mem_Alloc(800) != NULL)
     ;

   assert(Mem_Free(ptr[0]) == 0);
   assert(Mem_Free(ptr[1]) == 0);

   ptr[2] = Mem_Alloc(1600);
   assert(ptr[2] != NULL);

   assert(Mem_Uninit() == 0);

   return 1;
}


int tb_mem_manager_test_coalesce_5() {

   assert(Mem_Init() == 0);
   void * ptr[400];
   Mem_Dump(container);
   int x = 0;
   for(x = 0; x<102; x++){
    ptr[x] = Mem_Alloc(32);
    assert(ptr[x] != NULL);
   }
   Mem_Dump(container);
   for(x = 0; x<102; x++){
    assert(Mem_Free(ptr[x]) == 0);
   }
   Mem_Dump(container);

   assert(Mem_Uninit() == 0);

   return 1;
}

int tb_mem_manager_test_first_fit_0() {

   assert(Mem_Init() == 0);
   void* ptr[9];
   void* test;

   ptr[0] = Mem_Alloc(300);
   assert(ptr[0] != NULL);

   ptr[1] = Mem_Alloc(200);
   assert(ptr[1] != NULL);

   ptr[2] = Mem_Alloc(200);
   assert(ptr[2] != NULL);

   ptr[3] = Mem_Alloc(100);
   assert(ptr[3] != NULL);

   ptr[4] = Mem_Alloc(200);
   assert(ptr[4] != NULL);

   ptr[5] = Mem_Alloc(800);
   assert(ptr[5] != NULL);

   ptr[6] = Mem_Alloc(500);
   assert(ptr[6] != NULL);

   ptr[7] = Mem_Alloc(700);
   assert(ptr[7] != NULL);

   ptr[8] = Mem_Alloc(300);
   assert(ptr[8] != NULL);

   assert(Mem_Free(ptr[1]) == 0);
   assert(Mem_Free(ptr[3]) == 0);
   assert(Mem_Free(ptr[5]) == 0);
   assert(Mem_Free(ptr[7]) == 0);

   test = Mem_Alloc(50);

   assert(
           (!(
             (
               ((unsigned long int)test >= (unsigned long int)ptr[3])
               &&
               ((unsigned long int)test < (unsigned long int)ptr[4])
             )
             ||
             (
               ((unsigned long int)test >= (unsigned long int)ptr[3])
               &&
               ((unsigned long int)test < (unsigned long int)ptr[2])
             )
           ))
           &&
           (!(
             (
               ((unsigned long int)test >= (unsigned long int)ptr[5])
               &&
               ((unsigned long int)test < (unsigned long int)ptr[6])
             )
             ||
             (
               ((unsigned long int)test >= (unsigned long int)ptr[5])
               &&
               ((unsigned long int)test < (unsigned long int)ptr[4])
             )
           ))
         );

   assert(Mem_Uninit() == 0);

   return 1;
}

/* test null condition */
int tb_mem_manager_test_free_0() {

   assert(Mem_Init() == 0);

   void* ptr = Mem_Alloc(8);
   assert(ptr != NULL);
   ptr = NULL;
   assert(Mem_Free(ptr) == -1);

   assert(Mem_Uninit() == 0);

   return 1;
}

/* test simple condition */
int tb_mem_manager_test_free_1() {

   assert(Mem_Init() == 0);

   void* ptr = Mem_Alloc(8);
   assert(ptr != NULL);
   assert(Mem_Free(ptr) == 0);

   assert(Mem_Uninit() == 0);

   return 1;
}

/* test unallocated */
int tb_mem_manager_test_free_2() {

   assert(Mem_Init() == 0);

   int* ptr = (int*) Mem_Alloc(2 * sizeof(int));
   assert(ptr != NULL);
   ptr = ptr + 1;
   assert(Mem_Free(ptr) == -1);

   assert(Mem_Uninit() == 0);

   return 1;
}

int tb_mem_manager_test_free_3() {

   assert(Mem_Init() == 0);
   void* ptr[4];

   ptr[0] = Mem_Alloc(4);
   ptr[1] = Mem_Alloc(8);
   assert(Mem_Free(ptr[0]) == 0);
   assert(Mem_Free(ptr[1]) == 0);

   ptr[2] = Mem_Alloc(16);
   ptr[3] = Mem_Alloc(4);
   assert(Mem_Free(ptr[2]) == 0);
   assert(Mem_Free(ptr[3]) == 0);

   assert(Mem_Uninit() == 0);

   return 1;
}

int tb_mem_manager_test_free_4() {

   assert(Mem_Init() == 0);
   void * ptr[9];
   ptr[0] = Mem_Alloc(1);
   ptr[1] = (Mem_Alloc(5));
   ptr[2] = (Mem_Alloc(14));
   ptr[3] = (Mem_Alloc(8));
   Mem_Dump(container);
   assert(ptr[0] != NULL);
   assert(ptr[1] != NULL);
   assert(ptr[2] != NULL);
   assert(ptr[3] != NULL);

   assert(Mem_Free(ptr[1]) == 0);
   assert(Mem_Free(ptr[0]) == 0);
   assert(Mem_Free(ptr[3]) == 0);
   Mem_Dump(container);
   ptr[4] = (Mem_Alloc(1));
   ptr[5] = (Mem_Alloc(4));
   assert(ptr[4] != NULL);
   assert(ptr[5] != NULL);
   Mem_Dump(container);
   assert(Mem_Free(ptr[5]) == 0);
   Mem_Dump(container);
   ptr[6] = (Mem_Alloc(9));
   ptr[7] = (Mem_Alloc(33));
   assert(ptr[6] != NULL);
   assert(ptr[7] != NULL);

   assert(Mem_Free(ptr[4]) == 0);
   Mem_Dump(container);
   ptr[8] = (Mem_Alloc(55));
   assert(ptr[8] != NULL);
   Mem_Dump(container);
   assert(Mem_Free(ptr[2]) == 0);
   assert(Mem_Free(ptr[7]) == 0);
   assert(Mem_Free(ptr[8]) == 0);
   assert(Mem_Free(ptr[6]) == 0);
   Mem_Dump(container);

   assert(Mem_Uninit() == 0);

   return 1;
}

/* Initializes test -- really it just sets a pointer for the print function */
int tb_mem_manager_init( void * ptr)
{
    container = (XPRINTF_CONTAINER)ptr;

    return 1;
}

/* runs through all tests. If all tests return, then everythings kosher -- otherwise, test is designed to trap at the assertion that failed */
int tb_mem_manager_complete()
{
    tb_mem_manager_test_alloc_0();

    tb_mem_manager_test_alloc_1();

    tb_mem_manager_test_alloc_2();

    tb_mem_manager_test_alloc_no_space_0();

    tb_mem_manager_test_alloc_no_space_1();

    tb_mem_manager_test_align_0();

    tb_mem_manager_test_align_1();

    tb_mem_manager_test_align_2();

    tb_mem_manager_test_coalesce_0();

    tb_mem_manager_test_coalesce_1();

    tb_mem_manager_test_coalesce_2();

    tb_mem_manager_test_coalesce_3();

    tb_mem_manager_test_coalesce_4();

    tb_mem_manager_test_coalesce_5();

    tb_mem_manager_test_first_fit_0();

    /* test null condition */
    tb_mem_manager_test_free_0();

    /* test simple condition */
    tb_mem_manager_test_free_1();

    /* test unallocated */
    tb_mem_manager_test_free_2();

    tb_mem_manager_test_free_3();

    tb_mem_manager_test_free_4();

    /* SUCCESS! */
    return 1;
}
