/*
 * AES_Queue_Testbench.c
 *
 *  Created on: Oct 5, 2018
 *      Author: Peter S Procek
 */


/***********************************************************************************************************************************************************
 *                                                                    Includes
 *
 *
 *
 *
 ************************************************************************************************************************************************************/
#include "stdlib.h"   /* used for malloc - bare metal */
#include "stdbool.h"
#include "mem.h"

#include "../../../AES_Memory/AES_Heap/Include/mem.h"
#include "../../Include/AES_Queue.h"

XPRINTF_CONTAINER container;

/***********************************************************************************************************************************************************
 *                                                                    Definitions
 *
 *
 *
 *
 ************************************************************************************************************************************************************/

#define TEST_LENGTH									20
/***********************************************************************************************************************************************************
 *                                                                    Private Variables
 *
 *
 *
 *
 ************************************************************************************************************************************************************/

AES_Queue DUT;
int number_enqueued_items;
/***********************************************************************************************************************************************************
 *                                                                    Private Functions
 *
 *
 *
 *
 ************************************************************************************************************************************************************/

int tb_aes_queue_test_isEmpty_0()
{
	assert( AES_Queue_isEmpty(&DUT));

	return 1;
}

int tb_aes_queue_test_enQueue_1()
{
	char test_string[] = "test_enQueue_1";

	assert( AES_Queue_enQueue(&DUT, test_string, strlen(test_string)) == (void *)&DUT);

	number_enqueued_items++;

	assert( AES_Queue_Node_Count( &DUT ) == number_enqueued_items);

	return 1;
}

int tb_aes_queue_test_enQueue_2()
{

	char test_string[] = "test_enQueue_2";
	int test_int = 93;
	int x;

	for(x = 0; x < 20; x++)
	{
		if( (x % 2) == 0 ) //even
		{
			assert( AES_Queue_enQueue(&DUT, test_string, strlen(test_string)) == (void *)&DUT);
			number_enqueued_items++;

		}
		else //odd
		{
			assert( AES_Queue_enQueue(&DUT, &test_int, sizeof(int)) == (void *)&DUT);
			number_enqueued_items++;

		}
	}

	assert( AES_Queue_Node_Count( &DUT ) ==  number_enqueued_items);

	return 1;
}

int tb_aes_queue_test_isEmpty_3()
{
	char test_string[] = "test_isEmpty_3";

	assert( AES_Queue_enQueue(&DUT, test_string, strlen(test_string)) == (void *)&DUT);
	number_enqueued_items++;

	assert( !(AES_Queue_isEmpty(&DUT)));

	return 1;
}

int tb_aes_queue_test_enQueue_deQueue_4()
{
	AES_Queue_Node_Data_Packet my_data;

	char test_string[] = "test_enQueue_deQueue_4";
	int test_int = 93;
	int x;

	assert( AES_Queue_enQueue(&DUT, test_string, strlen(test_string)) == (void *)&DUT);
	number_enqueued_items++;

	for(x = 0; x < 20; x++)
	{
		if( (x % 2) == 0 ) //even
		{
			assert( AES_Queue_enQueue(&DUT, test_string, strlen(test_string)) == (void *)&DUT);
			number_enqueued_items++;
		}
		else //odd
		{
			assert( AES_Queue_enQueue(&DUT, &test_int, sizeof(int)) == (void *)&DUT);
			number_enqueued_items++;
		}
	}

	assert( AES_Queue_Node_Count( &DUT ) ==  (/*number_enqueued_items*/ + x + 1));

	if(AES_Queue_deQueue(&DUT, &my_data) != 0)
		number_enqueued_items--;

	assert( AES_Queue_Node_Count( &DUT ) ==  (/*number_enqueued_items*/ + x));

	return 1;
}


#if 0
int tb_aes_queue_test_enQueue_deQueue_5()
{

	int iterator;

	for(iterator = 0; iterator < TEST_LENGTH; iterator++)
	{

	}



	return 1;
}

int tb_aes_queue_test_enQueue_deQueue_6()
{



	return 1;
}

int tb_aes_queue_test_peak_7()
{



	return 1;
}

int tb_aes_queue_test_peak_8()
{



	return 1;
}
#endif
/***********************************************************************************************************************************************************
 *                                                                    Public Functions
 *
 *
 *
 *
 ************************************************************************************************************************************************************/
/* Initializes test -- really it just sets a pointer for the print function */
int tb_queue_init( void * ptr)
{
    container = (XPRINTF_CONTAINER)ptr;

    return 1;
}

int tb_aes_queue_top_level_testbench( )
{
	//int num_nodes = 0;

	assert( Mem_Uninit() == 0 );
	assert( Mem_Init() == 0);

	Mem_Dump(container);

	tb_aes_queue_test_isEmpty_0(); /*MUST BE CALLED FIRST*/

	//tb_aes_queue_test_enQueue_1();

	//tb_aes_queue_test_enQueue_2();

	//tb_aes_queue_test_isEmpty_3();

	tb_aes_queue_test_enQueue_deQueue_4();

	Mem_Dump(container);

	//num_nodes = DUT.Queue_number_of_nodes;

	AES_Queue_Delete(&DUT);
	//assert( AES_Queue_Delete(&DUT) == num_nodes);

	Mem_Dump(container);

	assert( Mem_Uninit() == 0 );



    /* proceed through each unit test */

    return 1;
}

