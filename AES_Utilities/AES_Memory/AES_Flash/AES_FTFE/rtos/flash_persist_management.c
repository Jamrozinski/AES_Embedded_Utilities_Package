
/********************************************************************************
 * Description:
 * Persistence Manager used to add persistence storage capability
 *	to various flash and memory peripherals
 *
 * Remarks:
 *
 * Example:
 *
 *
 * Copyright Notice:
 *
 * Copyright (c) 2017 by Apex Embedded Systems. All
 * rights reserved. This document is the confidential
 * property of Apex Embedded Systems Any reproduction
 * or dissemination is prohibited unless specifically
 * authorized in writing.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * *****************************************************************************/


#include "COMPILER_ASSERTION.h"
#include "flash_persist_management.h"
#include "flash_abstraction.h"
#include "buffer_shared.h"
#include "APEX_timestamp.h"
#include "string.h"
#include "fsl_flash.h"
#include "math.h"
#include "AES_Queue.h"
#include "Persist_Config.h"

#include "error_info.h"

/***********************************************************************************************************************************************************
 *                                                                Private Forward Declarations
 *
 *
 *
 *
 ************************************************************************************************************************************************************/
#if (defined(FLASH_DEBUG_MODE) && !FLASH_DEBUG_MODE)
static uint32_t Persist_flash_record_valid(persistRecord_t * recordPtr);
static uint32_t Persist_flash_record_exists(persistRecord_t * recordPtr);
static uint64_t Persist_record_checksum(persistRecord_t * record);
static void Persist_get_timestamp_config(apexTimestamp_t * t_config);
static enum buffer_shared_type Persist_get_record_type(persistRecord_t * record, uint8_t * isString);
static int32_t Persist_get_record_tag(persistRecord_t * record);
static uint32_t Persist_Flash_Record_Copy_To_RAM(persistRecord_t * record_in_ram, persistRecord_t * record_in_flash);
static uint32_t Persist_flash_get_total_record_count(void * base, , uint32_t bound_length_bytes);
static int32_t Persist_flash_compare_record(persistRecord_t * record_a, persistRecord_t * record_b);
static int32_t Persist_flash_write_record(persistRecord_t * record_in_ram, void * base)
static int32_t Persist_flash_record_bank_clean_request(persistRecord_t ** last_valid_record);
static uint64_t fletcher_checksum( uint32_t number_of_words, void * data_ptr);
static int32_t Persist_flash_region_initialize( void * primary_bank_ptr );
static Persist_flash_garbage_collect( void )
#endif

/***********************************************************************************************************************************************************
 *                                                                Private Variables
 *
 *
 *
 *
 ************************************************************************************************************************************************************/
static flash_config_t s_flashDriver;
static apexTimestamp_t persist_flash_timestamp_config;
static void * Primary_Bank_Pointer;
static uint32_t Primary_Bank_Size;
static flash_config_t s_flashDriver;
static flash_security_state_t securityStatus;
static uint32_t  failAddr, failDat;

/***********************************************************************************************************************************************************
 *                                                             Public Function Declarations
 *
 *
 *
 *
 ************************************************************************************************************************************************************/

/*FUNCTION**********************************************************************
 * Function Name : persist_flash_get_primary_bank
 * Description   : returns address of the primary bank pointer
 *
 * parameters:
 * -void
 *
 * Return value:
 * -void * flash region base
 *END**************************************************************************/
void * persist_flash_get_primary_bank( void )
{
    return (void *)Primary_Bank_Pointer;
}

/*FUNCTION**********************************************************************
 * Function Name : Flash_PersistInitialize
 * Description   : Initialization/boot load sequence function. It will begin
 * by scanning the 'Primary' persist bank. It will tally the amount of registered
 * records (validity of records doesn't matter) to find where the next write address
 * should be. If its found that the 'Primary' bank is full or near full, it will
 * execute a clean and rewrite (write current valid records up to a history high
 * water mark into 'Secondary' bank, erase 'Primary' bank, and write back 'secondary'
 * to 'Primary.'
 *
 * Then a recount of records must be done again to find the next write
 * location. Validate that the next write location is 8Byte aligned otherwise error.
 * Also look at the last valid record (here valid means both structure exist and
 * checksum passes) and see what kind of time stamp it used and what the last
 * time stamp value was and continue operation from there. Set global accordingly.
 *
 * (TODO:: make it so write back is not necessary and we simply erase
 * primary and can continue writing in the secondary -- make it so we can change
 * primary banks during operation to save on erase times. Then we only need to
 * erase during idle. Initialization will have to differentiate the banks however
 * to determine which one is 'Primary' on startup)
 *
 * TODO: clean up error handling.
 *
 * parameters:
 * -void
 *
 * Return value:
 * -void
 *END**************************************************************************/
int32_t Persist_flash_initialize( void )
{
	int32_t trouble;
	uint32_t number_of_records;
	void * last_valid_record_opaque;
	persistRecord_t * last_valid_record;
	/* initialize flash */
	uint32_t result;

    /********************************************************************/
    /************************INITIALIZE FLASH****************************/
	securityStatus = kFLASH_SecurityStateNotSecure; /* Return protection status */
	memset(&s_flashDriver, 0, sizeof(flash_config_t));

    result = FLASH_Init(&s_flashDriver);
	if (kStatus_FLASH_Success != result)
	{
		/* todo:: return flash initialize error */
		trouble = -1;
		goto FLASH_PERSISTENCE_INITIALIZE_ERROR;
	}

	/* Check security status. */
	result = FLASH_GetSecurityState(&s_flashDriver, &securityStatus);
	if (kStatus_FLASH_Success != result)
	{
	    /* security state retrieval error */
	    trouble = -1;
	    goto FLASH_PERSISTENCE_INITIALIZE_ERROR;
	}
    /********************************************************************/
    /********************************************************************/



#if 0
    /************************ERASE ALL OF FLASH****************************/
    uint32_t pflashSectorSize = 0;
	flash_security_state_t securityStatus = kFLASH_SecurityStateNotSecure; /* Return protection status */
//	uint32_t result = 0;

    FLASH_GetProperty(&s_flashDriver, kFLASH_PropertyPflashSectorSize, &pflashSectorSize);

	/* Check security status. */
	result = FLASH_GetSecurityState(&s_flashDriver, &securityStatus);
	if (kStatus_FLASH_Success != result)
	{
		return -1;
	}

	/* we don't care about any arguments here */

	/* Erase the entire persistent section */
    if (kFLASH_SecurityStateNotSecure == securityStatus)
    {
    	/* Erase Persistent section. */
        result = FLASH_Erase(&s_flashDriver, (uint32_t)PERSIST_FLASH_BASE, (uint32_t)(PERSIST_TOTAL_BANK_SIZE), kFLASH_ApiEraseKey);
        if (kStatus_FLASH_Success != result)
        {
        	return -1;
        }

        /* Verify sector if it's been erased. */
        result = FLASH_VerifyErase(&s_flashDriver, (uint32_t)PERSIST_FLASH_BASE, (uint32_t)(PERSIST_TOTAL_BANK_SIZE), kFLASH_MarginValueUser);
        if (kStatus_FLASH_Success != result)
        {
        	return -1;
        }
    }
    else
    {
    	return -1;
    }


    /**********************************************************************/
#else

	/* initialize the delete bank */
	//trouble = Persist_flash_initialize_delete_sector();
	//if(trouble < 0)
	//{
		//goto FLASH_PERSISTENCE_INITIALIZE_ERROR;
	//}
#if 0
    /* determine which bank is primary and do a recovery */
	trouble = Persist_flash_determine_primary_bank_and_recover();
	if(trouble < 0)
	{
		goto FLASH_PERSISTENCE_INITIALIZE_ERROR;
	}

	/* do a pre-emptive cleanup */

	//todo: Need to check the boundary conditions of 'committing' a copy record to the new
	//      primary sector

	trouble = Persist_flash_primary_sector_clean_and_shift(Primary_Bank_Pointer, Primary_Bank_Size, 1, PERSIST_FLASH_FILL_THRESHOLD, PERSIST_FLASH_MAX_HISTORY );
	if(trouble == -1 || trouble == -4)
	{
		goto FLASH_PERSISTENCE_INITIALIZE_ERROR;
	}
#else

	/* initialize flash region */
	trouble = Persist_flash_region_initialize( &Primary_Bank_Pointer );

	if(trouble < 0)
	{
		goto FLASH_PERSISTENCE_INITIALIZE_ERROR;
	}

#endif
#endif
    /**************************************************************************/
    /************************INITIALIZE TIME CONFIG****************************/

	number_of_records = Persist_flash_get_valid_record_count(&last_valid_record_opaque);

	last_valid_record = (persistRecord_t *)last_valid_record_opaque;

	if(number_of_records > 0)
	{
		/* we have a record, extract the time configuration */
		persist_flash_timestamp_config.timestamp_configuration = PERSIST_RECORD_TIMECONFIG_GET(last_valid_record->persist_record_header.record_id);
		persist_flash_timestamp_config.timestamp_value = last_valid_record->persist_record_header.timestamp;
	}

	trouble = 0;
    /**************************************************************************/
    /**************************************************************************/

    /**************************************************************************/
    /***********************INITIALIZE RECORD QUEUES***************************/

	CircularQueue_Init( (p_CircularQueue_t)&Persist_Write_Queue,(void *)&Persist_Write_Queue_Buffer, PERSIST_WRITE_QUEUE_BUFFER_SIZE);
    CircularQueue_Init( (p_CircularQueue_t)&Persist_Commit_Queue,(void *)&Persist_Commit_Queue_Buffer, PERSIST_COMMIT_QUEUE_BUFFER_SIZE);


    /**************************************************************************/
    /**************************************************************************/

FLASH_PERSISTENCE_INITIALIZE_ERROR:
    return trouble;
}

/*FUNCTION**********************************************************************
 * Function Name : Flash_PersistInitializeRegion
 * Description   : Initializes the region by determining which bank is the primary
 * persistence bank. Function will check the region header which will specify
 * what the last primary bank pointer should have been. If the last clean
 * operation succeeded, the valid bit should be set, in which case the last
 * primary pointer is indeed the new current primary pointer. Otherwise, default
 * to the opposite bank. If no header is found, that means this is the first time
 * this region is initialized entirely, in which case delete all persistent bank
 * regions and write the region header.
 *
 *
 *
 * parameters:
 * -void * primary_bank_ptr
 *
 * Return value:
 * -void
 *END**************************************************************************/
#if (defined(FLASH_DEBUG_MODE) && !FLASH_DEBUG_MODE)
static int32_t Persist_flash_region_initialize( void * primary_bank_ptr )
#else
int32_t Persist_flash_region_initialize( void ** primary_bank_ptr )
#endif
{
	(void)primary_bank_ptr;

	persistInitHeader_t my_region_header;
	persistInitHeader_t * region_header_ptr;

	uint32_t expected_header_size = 0;
	uint64_t checksum = 0;
	uint64_t checksum_internal = 0;

	int32_t trouble = SUCCESS;

	//persist region base starts with the Initialization Header
	region_header_ptr = (persistInitHeader_t *)PERSIST_REGION_BASE;

	expected_header_size = sizeof(my_region_header.last_operation_valid) + sizeof(my_region_header.primary_bank_ptr);

	//get number of words - should always be aligned (guaranteed by asserts/compiler)
	expected_header_size = expected_header_size >> 2;

	//check that the initialization header isn't corrupt -- do a checksum
	checksum = fletcher_checksum(expected_header_size, (void *)PERSIST_REGION_BASE);
	checksum_internal = region_header_ptr->checksum;

	if( checksum == checksum_internal)
	{
		//we have a valid region header, the flash header has not been corrupted
		(*primary_bank_ptr) = (void *)region_header_ptr->primary_bank_ptr;


#if 0 //SIMPLIFIED ABOVE
		//check if valid, otherwise swap
		if( (uint32_t)(*primary_bank_ptr) == (uint32_t)PERSIST_FLASH_FIRST_BANK_BASE )
		{
			if(region_header_ptr->last_operation_valid == 1)
			{
				//its valid, erase other bank
				trouble = uFlash_erase_and_verify( &s_flashDriver, (uint32_t)PERSIST_FLASH_SECOND_BANK_BASE, (uint32_t)PERSIST_SECOND_BANK_SIZE);

				if(trouble < 0)
				{
					trouble = -1;
		        	goto FLASH_PERSISTENCE_REGION_INITIALIZE_ERROR;
				}

			}
			else
			{
				//not valid, default to opposite bank
				(*primary_bank_ptr) = (void *)PERSIST_FLASH_SECOND_BANK_BASE;

				//erase opposite bank
				trouble = uFlash_erase_and_verify( &s_flashDriver, (uint32_t)PERSIST_FLASH_FIRST_BANK_BASE, (uint32_t)PERSIST_FLASH_FIRST_BANK_BASE);

				if(trouble < 0)
				{
					trouble = -1;
		        	goto FLASH_PERSISTENCE_REGION_INITIALIZE_ERROR;
				}
			}
		}
		else
		if( (uint32_t)(*primary_bank_ptr) == (uint32_t)PERSIST_FLASH_SECOND_BANK_BASE )
		{
			if(region_header_ptr->last_operation_valid == 1)
			{
				//its valid, erase other bank
				trouble = uFlash_erase_and_verify( &s_flashDriver, (uint32_t)PERSIST_FLASH_FIRST_BANK_BASE, (uint32_t)PERSIST_FLASH_FIRST_BANK_BASE);

				if(trouble < 0)
				{
					trouble = -1;
		        	goto FLASH_PERSISTENCE_REGION_INITIALIZE_ERROR;
				}

			}
			else
			{
				//not valid, default to opposite bank
				(*primary_bank_ptr) = (void *)PERSIST_FLASH_FIRST_BANK_BASE;

				//erase opposite bank
				trouble = uFlash_erase_and_verify( &s_flashDriver, (uint32_t)PERSIST_FLASH_SECOND_BANK_BASE, (uint32_t)PERSIST_FLASH_SECOND_BANK_BASE);

				if(trouble < 0)
				{
					trouble = -1;
		        	goto FLASH_PERSISTENCE_REGION_INITIALIZE_ERROR;
				}
			}
		}
		else
		{
			trouble = -1;
        	goto FLASH_PERSISTENCE_REGION_INITIALIZE_ERROR;
		}
#endif

	}
	else
	{
		trouble = uFlash_erase_and_verify( &s_flashDriver, (uint32_t)PERSIST_REGION_BASE, (uint32_t)PERSIST_REGION_SIZE);

		if(!(trouble < 0))
		{
			/* default to Bank One */
			my_region_header.primary_bank_ptr = PERSIST_FLASH_FIRST_BANK_BASE;
			my_region_header.last_operation_valid = 1;

			/* compute checksum */
			checksum = fletcher_checksum(expected_header_size, (void *)&my_region_header);

			my_region_header.checksum = checksum;

			/* set the primary bank pointer */
			(*primary_bank_ptr) = (void *)PERSIST_FLASH_FIRST_BANK_BASE;

			/* write the region header to the top of the flash persist region */
	    	/***********PROGRAM HEADER INTO FLASH***********/
	        trouble = FLASH_Program(&s_flashDriver, (uint32_t)PERSIST_REGION_BASE, (uint32_t *)(&(my_region_header)), sizeof(persistInitHeader_t));
	        if (kStatus_FLASH_Success != trouble)
	        {
	        	/* return persist flash record program failure */
	        	trouble = -EC_PERSIST_WRITE_RECORD_FAIL;
	        	goto FLASH_PERSISTENCE_REGION_INITIALIZE_ERROR;
	        }

	        /* Verify programming by Program Check command with user margin levels */
	        trouble = FLASH_VerifyProgram(	&s_flashDriver,
	        								(uint32_t)PERSIST_REGION_BASE,
	        								sizeof(persistInitHeader_t),
											(const uint32_t *)(&(my_region_header)), //MI: was (uint8_t *)&s_persistRecord,
											kFLASH_MarginValueUser,
											&failAddr, &failDat
										);
	        if (kStatus_FLASH_Success != trouble)
	        {
	        	/* program verify failure */
	        	trouble = -EC_PERSIST_WRITE_VERIFY;
	        	goto FLASH_PERSISTENCE_REGION_INITIALIZE_ERROR;
	        }
		}
		else
		{
			trouble = -1;
        	goto FLASH_PERSISTENCE_REGION_INITIALIZE_ERROR;		}
	}

FLASH_PERSISTENCE_REGION_INITIALIZE_ERROR:
	   return trouble;
}

/*FUNCTION**********************************************************************
 * Function Name : Flash_getNumberValidRecords
 * Description   : walk through FTFE flash and scan to find first record (each
 * record has a static ID associated). Then walk through the flash at
 * sizeof(persistRecord_t) bytes at a time to walk and find each subsequent record.
 * For each record found, only increment if the record is valid (checksum passed).
 *
 * If there is a mismatch between valid and total records on startup, initialization
 * is assumed to fix it. So all subsequent calls to getNumberValidRecords will
 * in fact equal the total records and return a valid pointer.
 *
 * Parameters:
 * -(pass by reference) void * pointer
 *
 * Return value:
 * -number of valid records that exist
 * -(pass by reference) pointer to the last counted record
 *END**************************************************************************/
int32_t Persist_flash_get_valid_record_count(void ** recordPointer)
{
//	uint64_t calculated_checksum;
	int32_t valid_record_count;

	uint32_t record_offset;
	void * base;
	void * record_index;
	void * end_of_bank;

	if(recordPointer != NULL)
	{
		*recordPointer = NULL;
	}

	/* look at the starting location of the primary bank */
//	calculated_checksum = 0;
	valid_record_count = 0;
	record_offset = 0;
	base = Primary_Bank_Pointer;

	/* determine where the end of bank is */
	if(((uint32_t)Primary_Bank_Pointer >= PERSIST_FLASH_FIRST_BANK_BASE) && ((uint32_t)Primary_Bank_Pointer < PERSIST_FLASH_SECOND_BANK_BASE))
	{
		end_of_bank = (void *)((char *)base + PERSIST_FIRST_BANK_SIZE);
	}
	else
	if(((uint32_t)Primary_Bank_Pointer >= PERSIST_FLASH_SECOND_BANK_BASE) && ((uint32_t)Primary_Bank_Pointer < PERSIST_FLASH_END_OF_PERSISTENCE))
	{
		end_of_bank = (void *)((char *)base + PERSIST_SECOND_BANK_SIZE);
	}


	record_index = (void *)((char*)base + record_offset);

	/* scroll through the flash primary sector to find all existing records */
	while( (uint32_t)record_index < (uint32_t)(end_of_bank) )
	{
		/* check if record is valid*/
		if(Persist_flash_record_exists((persistRecord_t *)record_index) && Persist_flash_record_valid(record_index))
		{
			/* record is valid, increment valid count and update return pointer */
			valid_record_count++;

			if(recordPointer != NULL)
			{
				*recordPointer = record_index;
			}

		}

		record_index = (void *)((char *)record_index + 1);
#if 0
		/* we won't stop if we hit an invalid record, we simply look at the next record */
		if((uint32_t)((char *)record_index + 1) >= PERSIST_FLASH_SECOND_BANK_BASE)
		{
			/* we've hit the end of our primary bank */
			goto END_OF_BANK;
		}
		else
		{
			record_index = (void *)((char *)record_index + 1);
		}
#endif
	}

	return valid_record_count;
}

/*FUNCTION**********************************************************************
 * Function Name : Flash_getRecord
 * Description   : scans through flash to find the most recent record of specified
 * record type and grabs timestamp. It is up to the user to copy it to ram.
 *
 * Parameters:
 * -enum buffer_shared_type type
 * -apexTimestamp_t * timestamp structure, if null, no timestamp returned
 *
 * Return value:
 * -NULL if no valid record of type is found
 * -pointer to record
 *END**************************************************************************/
persistRecord_t * Persist_Flash_Get_Recent_Record(int32_t tag, apexTimestamp_t * timestamp)
{
	persistRecord_t * my_type_record;
	apexTimestamp_t timestamp_config = { 0, 0};
	uint32_t last_timestamp;

	void * end_of_bank = NULL;

	void * record_iterator;

	record_iterator = Primary_Bank_Pointer;
	last_timestamp = 0;
	my_type_record = NULL;

	/* determine where the end of bank is */
	if(((uint32_t)Primary_Bank_Pointer >= PERSIST_FLASH_FIRST_BANK_BASE) && ((uint32_t)Primary_Bank_Pointer < PERSIST_FLASH_SECOND_BANK_BASE))
	{
		end_of_bank = (void *)((char *)Primary_Bank_Pointer + PERSIST_FIRST_BANK_SIZE);
	}
	else
	if(((uint32_t)Primary_Bank_Pointer >= PERSIST_FLASH_SECOND_BANK_BASE) && ((uint32_t)Primary_Bank_Pointer < PERSIST_FLASH_END_OF_PERSISTENCE))
	{
		end_of_bank = (void *)((char *)Primary_Bank_Pointer + PERSIST_SECOND_BANK_SIZE);
	}

	/* iterate through each record*/
	while((uint32_t)((char *)record_iterator + 1) < (uint32_t)end_of_bank)
	{
		if(Persist_flash_record_exists((persistRecord_t *)record_iterator))
		{
			/* see if record is valid */
			if(Persist_flash_record_valid(record_iterator))
			{
				/* if the record is valid, see if its of the type we want */
				if ( Persist_get_record_tag( (persistRecord_t *) record_iterator ) == tag )
				{
					/* see if current timestamp is greater then last found timestamp*/
					Persist_get_timestamp_config(&timestamp_config);

					if(timestamp_config.timestamp_value >= last_timestamp)
					{
						last_timestamp = timestamp_config.timestamp_value;
						my_type_record = (persistRecord_t *)record_iterator;
					}
				}
			}
		}

		/* iterate to the next record as long as your not cross boundaries */
		record_iterator = (void *)((char *)record_iterator + 1);
	}

	/* return the time stamp if the user requests it */
	if(timestamp != NULL)
	{
		memcpy((void *)timestamp, (void *)&timestamp_config, sizeof(apexTimestamp_t));
	}

	/* return the record of specific type */
	return my_type_record;

}

/*FUNCTION**********************************************************************
 * Function Name : Persist_flash_get_historical_record
 * Description   : Scans through record list and builds a list of specified
 * record type then returns the requested historical record. If user wants
 * the most recent, pass 1. For second most recent pass 2 and so on.
 *
 * Parameters:
 * -enum buffer_shared_type type
 * -uint32_t recent, 0 (most recent) to MAX_RECORD_ALLOWED (least recent)
 * -apexTimestamp_t * timestamp structure, if null, no timestamp returned
 * -uint32_t number_of_type_records, a pass by reference value to return amount of records found of that type
 *
 * Return value:
 * -NULL if no valid record of type is found
 * -pointer to record
 * -timestamp configuration if requested
 * -number of records of the requested type if requested
 *END**************************************************************************/
persistRecord_t * Persist_flash_get_historical_record(int32_t tag, uint32_t recent, apexTimestamp_t * timestamp, uint32_t * number_of_type_records)
{
	/* make an empty list for max possible records of a single type */
	persistRecord_t * type_record_list[MAX_PERSIST_RECORDS_ALLOWED];
	void * end_of_bank = NULL;
	void * base;

	persistRecord_t * my_type_record;
	apexTimestamp_t timestamp_config = { 0, 0};
	uint32_t number_of_records_found;

	void * record_iterator;

	/* initialize things */
	base = (void *)Primary_Bank_Pointer;
	record_iterator = (void *)base;
	number_of_records_found = 0;
	my_type_record = NULL;

	/* determine where the end of bank is */
	if(((uint32_t)Primary_Bank_Pointer >= PERSIST_FLASH_FIRST_BANK_BASE) && ((uint32_t)Primary_Bank_Pointer < PERSIST_FLASH_SECOND_BANK_BASE))
	{
		end_of_bank = (void *)((char *)base + PERSIST_FIRST_BANK_SIZE);
	}
	else
	if(((uint32_t)Primary_Bank_Pointer >= PERSIST_FLASH_SECOND_BANK_BASE) && ((uint32_t)Primary_Bank_Pointer < PERSIST_FLASH_END_OF_PERSISTENCE))
	{
		end_of_bank = (void *)((char *)base + PERSIST_SECOND_BANK_SIZE);
	}

	/* iterate through each record*/
	while((uint32_t)((char *)record_iterator + 1) < (uint32_t)end_of_bank)
	{
		if(Persist_flash_record_exists((persistRecord_t *)record_iterator))
		{
			/* see if record is valid */
			if(Persist_flash_record_valid(record_iterator))
			{
				/* if the record is valid, see if its of the type we want */
				if ( Persist_get_record_tag((persistRecord_t *)record_iterator) == tag )
				{
					/* we found a record of a certain type, add it to the list */
					type_record_list[number_of_records_found] = (persistRecord_t *)record_iterator;
					number_of_records_found++;
				}
			}

		}

		/* iterate to the next record as long as your not cross boundaries */
		record_iterator = (void *)((char *)record_iterator + 1);
	}

	/* our list is not guaranteed to be ordered depending on how clean up operations are implemented */
	/* order our list if we know we've actually found more than one record */

	if(number_of_records_found > 1)
	{
			uint32_t record_index, record_next_index;
			uint32_t timestamp_first_holder, timestamp_second_holder;
			persistRecord_t * record_swap_pointer;
			for(record_index = 0; record_index < (number_of_records_found - 1); record_index++)
			{
				timestamp_first_holder = type_record_list[record_index]->persist_record_header.timestamp;
				for(record_next_index = record_index + 1; record_next_index < number_of_records_found; record_next_index++)
				{
					timestamp_second_holder = type_record_list[record_next_index]->persist_record_header.timestamp;

					if(timestamp_second_holder > timestamp_first_holder)
					{
						/* swap */
						record_swap_pointer = type_record_list[record_index];
						type_record_list[record_index] = type_record_list[record_next_index];
						type_record_list[record_next_index] = record_swap_pointer;
					}

				}
			}
	}

	/* we now have an ordered list of newest (largest) to oldest (smallest) */


	/* return the time stamp if the user requests it */
	if(timestamp != NULL)
	{
		memcpy((void *)timestamp, (void *)&timestamp_config, sizeof(apexTimestamp_t));
	}

	if(number_of_type_records != NULL)
	{
		(*number_of_type_records) = number_of_records_found;
	}

	/* return proper index based on request */

	if(number_of_records_found == 0 || recent <= 0)
	{
		my_type_record = NULL;
	}
	else
	if(recent > number_of_records_found)
	{
		/* return the oldest record */
		my_type_record = type_record_list[number_of_records_found - 1];
	}
	else
	{
		/* return the exact however recent requested record */
		my_type_record = type_record_list[recent - 1];
	}

	/* return the record of specific type */
	return my_type_record;

}

/*FUNCTION**********************************************************************
 * Function Name : Persist_flash_get_record_history
 * Description   : scans through flash and builds a list of specified size into
 * a passed in buffer of valid records of the same type in incrementing order of
 * timestamp values. List will exist in ram.
 *
 * Parameters:
 * -enum buffer_shared_type type
 * -size_t bufferSize, the amount of records of the same time you want to retreive
 * -persistRecord_t * buffer, the actual list of records
 *
 * Return value:
 * -<0 on error
 * -number of records actually put into the list (can be = or < bufferSize)
 *END**************************************************************************/
int32_t Persist_flash_get_record_history(int32_t tag, size_t bufferSize, persistRecord_t ** buffer)
{
	/* ensure that the buffer actually lives outside of flash */
	/*Persist_flash_record_copy_to_ram*/
	/* make an empty list for max possible records of a single type */
	persistRecord_t * type_record_list[MAX_PERSIST_RECORDS_ALLOWED];

//	persistRecord_t * my_type_record;
//	apexTimestamp_t timestamp_config;
	uint32_t number_of_records_found;

	void * base;
	void * end_of_bank = NULL;
	void * record_iterator;

	/* initialize things */
	base = (void *)Primary_Bank_Pointer;
	record_iterator = (void *)base;
	number_of_records_found = 0;
//set but not used:	my_type_record = NULL;

	/* determine where the end of bank is */
	if(((uint32_t)Primary_Bank_Pointer >= PERSIST_FLASH_FIRST_BANK_BASE) && ((uint32_t)Primary_Bank_Pointer < PERSIST_FLASH_SECOND_BANK_BASE))
	{
		end_of_bank = (void *)((char *)base + PERSIST_FIRST_BANK_SIZE);
	}
	else
	if(((uint32_t)Primary_Bank_Pointer >= PERSIST_FLASH_SECOND_BANK_BASE) && ((uint32_t)Primary_Bank_Pointer < PERSIST_FLASH_END_OF_PERSISTENCE))
	{
		end_of_bank = (void *)((char *)base + PERSIST_SECOND_BANK_SIZE);
	}

	/* iterate through each record*/
	while((uint32_t)((char *)record_iterator + 1) < (uint32_t)end_of_bank)
	{
		if(Persist_flash_record_exists((persistRecord_t *)record_iterator))
		{
			/* see if record is valid */
			if(Persist_flash_record_valid(record_iterator))
			{
				/* if the record is valid, see if its of the type we want */
				if(Persist_get_record_tag((persistRecord_t *)record_iterator) == tag)
				{
					/* we found a record of a certain type, add it to the list */
					type_record_list[number_of_records_found] = (persistRecord_t *)record_iterator;
					number_of_records_found++;
				}
			}
		}

		/* iterate to the next record as long as your not crossing boundaries */
		record_iterator = record_iterator + 1;
	}

//END_OF_BANK:

	/* our list is not guaranteed to be ordered depending on how clean up operations are implemented */
	/* order our list if we know we've actually found more than one record */

	if(number_of_records_found > 1)
	{
			uint32_t record_index, record_next_index;
			uint32_t timestamp_first_holder, timestamp_second_holder;
			persistRecord_t * record_swap_pointer;
			for(record_index = 0; record_index < number_of_records_found; record_index++)
			{
				timestamp_first_holder = type_record_list[record_index]->persist_record_header.timestamp;
				for(record_next_index = record_index + 1; record_next_index < number_of_records_found; record_next_index++)
				{
					timestamp_second_holder = type_record_list[record_next_index]->persist_record_header.timestamp;

					if(timestamp_second_holder > timestamp_first_holder)
					{
						/* swap */
						record_swap_pointer = type_record_list[record_index];
						type_record_list[record_index] = type_record_list[record_next_index];
						type_record_list[record_next_index] = record_swap_pointer;
					}

				}
			}
	}

	/* we now have an ordered list of newest (largest) to oldest (smallest) */

	/* check the requested size vs actual size */

	if(buffer == NULL || bufferSize == 0)
	{
		return -1;
	}
	else
	if(number_of_records_found == 0)
	{
		return number_of_records_found;
	}
	else
	if( bufferSize > number_of_records_found)
	{
		/* requested a history of more records than available, return what's available */
		uint32_t buffer_index;
		for(buffer_index = 0; buffer_index < number_of_records_found; buffer_index++ )
		{
			buffer[buffer_index] = type_record_list[buffer_index];
			/*memcpy((void *)(buffer[buffer_index]), (void *)(type_record_list[buffer_index]), sizeof(persistRecord_t));*/
		}

		return number_of_records_found;

	}
	else
	{
		/* buffer size is less than what's available, so simply return request size */
		uint32_t buffer_index;
		for(buffer_index = 0; buffer_index < bufferSize; buffer_index++ )
		{
			buffer[buffer_index] = type_record_list[buffer_index];
			/*memcpy((void *)(buffer[buffer_index]), (void *)(type_record_list[buffer_index]), sizeof(persistRecord_t));*/
		}

		return bufferSize;
	}

	/* error, shouldn't get here */
	return -1;
}

/*FUNCTION**********************************************************************
 * Function Name :  Persist_flash_create_record
 * Description   : Takes a pointer to a record in ram and fills it with appropriate
 * data. It will fill in the struct ID as well as the record ID specified by
 * the type(+any other configuration needed). It will then fill in the internal
 * record buffer with the data which is passed by reference. If the type is a char,
 * the pointer will point to a char. If the type is a char but the data has a '\0'
 * terminator, then we know it is implicitly a string, so string type could then be
 * concatenated in the record ID as well.
 *
 * **NOTE::
 * Can't create time stamp here because the record won't be committed here. Multiple
 * records can be made with varying timestamps in ram, but it might not be indicative
 * of the real time stamp if it hasn't been committed yet.
 *
 * Ex.) Create 5 records, only commit the 5th one to flash. Now if I try to commit
 * the 1st or 2nd, they will have older timestamps, meaning i'd have to check now
 * if what I'm trying to commit is logically older than the last committed record
 * in flash, which is a hastle.
 *
 * Parameters:
 * -persistRecord_t * record_in_ram, the existing record to be filled and committed
 * -enum buffer_shared_type type, the type of data the user is committing to RAM
 * -uint32_t data_length, length of data passed in by user in BYTES
 * -char * data, a pointer to the data the user wants to write to flash
 *
 * Return value:
 * -<0 if record passed in is null, type is invalid, or data is null or if full data own't be committed
 * ->0 for successful creation
 *END**************************************************************************/
int32_t Persist_Flash_Create_Record( persistRecord_t * 			record_in_ram,  /*!< */
									 int					 	type, 			/*!< deprecated */
									 int32_t 					tag, 			/*!< */
									 uint32_t 					data_length, 	/*!< */
									 char * 					data			/*!< */
								   )
{
//	uint32_t record_id_buffer;
	char * record_data_buffer;
	/* make sure valid data is passed in */
	if(record_in_ram == NULL || data == NULL)
	{
		return -EC_PARAMETER;
	}

	/* check that the user attempted data (string or otherwise) will fit in buffer */
	/* we reserve the last spot in the buffer for the '\0' character */
	if(data_length > (CONTAINER_BYTE_LENGTH))
	{
		/* data length overflow error */
		return -EC_BUFFER_LENGTH;
	}

	/* clear out the record the user wishes to create */
	memset((void*)record_in_ram, 0, sizeof(persistRecord_t));


	/* tag with a struct ID */
	record_in_ram->persist_record_header.struct_id = RECORD_STRUCT_ID;

//TODO: record_id is set twice.
//TODO: also the 'type' really is defined at upper layers so long as same

#if 0



	/* see if we have a real type -- returns if its not a real type */
	switch((uint32_t)PERSIST_RECORD_TYPE_GET(record_in_ram->persist_record_header.record_id))
	{
		case (uint32_t)BUFFER_SHARED_TYPE__UINT8:
			break;

		case (uint32_t)BUFFER_SHARED_TYPE__UINT16:
			break;

		case (uint32_t)BUFFER_SHARED_TYPE__UINT32:
			break;

		case (uint32_t)BUFFER_SHARED_TYPE__INT8:
			break;

		case (uint32_t)BUFFER_SHARED_TYPE__CHAR:
			break;

		case (uint32_t)BUFFER_SHARED_TYPE__INT16:
			break;

		case (uint32_t)BUFFER_SHARED_TYPE__INT32:
			break;

		case (uint32_t)BUFFER_SHARED_TYPE__INTEGER:
			break;

		case (uint32_t)BUFFER_SHARED_TYPE__FLOAT:
			break;

		case (uint32_t)BUFFER_SHARED_TYPE__DOUBLE:
			break;

		case (uint32_t)BUFFER_SHARED_TYPE__VPOINT:
			break;
		default:
			return -EC_DATA_TYPE;
			break;
	}
#endif

	/* place in the type then see if its a real type */
//TODO: to go away??? Mike		record_in_ram->persistRecord_header.record_id |= (type & PERSIST_RECORD_TYPE_MASK )	<<	PERSIST_RECORD_TYPE_SHIFT  ;

	/* pack in the type, data_length, and tag - note, we don't need to pack the timeconfig because this will be done at when flash is committed*/
	/* during commit, timeconfig is garnered from existing flash records, then timeconfig is rebuilt and updated*/

//TODO: in this case, from what I can find, the tag is limited to 8-bits.  Mike.
//      note if we use tag it will cause an overflow situation and non-uniqueness.
//	record_in_ram->persistRecord_header.record_id = PERSIST_RECORD_ID_BUILD(type, 0, data_length, tag);
	record_in_ram->persist_record_header.record_id = PERSIST_RECORD_ID_BUILD(type, 0, data_length, 0);

//TODO: commented out since does nothing, maybe something to be here?? Mike.	record_in_ram->persistRecord_header.struct_id;

	/* pack the actual tag into the extended field */
	record_in_ram->persist_record_header.record_tag = tag;

	record_data_buffer = record_in_ram->container;

	/* clear the buffer in RAM */
	memset((void*)record_data_buffer, 0, CONTAINER_BYTE_LENGTH);

	/* copy the buffer to the record in ram */
	memcpy((void*)record_data_buffer, (void *)data, data_length);

	/* we've created our record */
	return 1; /* returns the number of records successfully written */
}

/*TODO:: complete this function in its entirety */
/*FUNCTION**********************************************************************
 * Function Name : Persist_flash_record_commit
 * Description   : Takes in a valid record in ram and writes it to flash. To do
 * this, use getNumberValid records to get the pointer to the last written valid
 * record. We will write to the next address. Only write if there is enough space
 * in our current bank. If there is not enough space in the current bank, do a
 * clean record bank request. Try writing again, if still not enough space,
 * do a clean record bank request with a smaller history count (this will likely
 * not happen but its a guard never the less). If we fail to program at the
 * specified address, then we do a clean up and try again. If still fails, then
 * the user is SOL and flash is corrupt and we have to erase and start over.
 *
 * Parameters:
 * -persistRecord_t * record_in_ram, record in ram to be programmed to flash memory
 *
 * Return value:
 * -<0 if programming to flash failed
 * ->0 if record is programmed to flash successfully
 *END**************************************************************************/
int32_t Persist_Flash_Record_Commit(persistRecord_t * record_in_ram)
{
	return Persist_flash_write_record(record_in_ram, (void *)Primary_Bank_Pointer);
}

/*FUNCTION**********************************************************************
 * Function Name : Persist_Flash_Record_Commit_Arbiter
 * Description   :
 *
 * Parameters:
 *
 * Return value:
 *END**************************************************************************/
int32_t Persist_Flash_Record_Commit_Arbiter(persistRecord_t * record_in_ram)
{

}

#if 0
/*FUNCTION**********************************************************************
 * Function Name : Persist_flash_extract_record_data
 * Description   : Takes a record, finds its type, unpacks its type length then
 * unpacks and copies the data/payload to the destination. This is a dangerous
 * function so user MUST be careful. It takes a void pointer as a parameter
 * thus its up to the user to make sure that the void pointer points to
 * a buffer of the type specified by buffer_shared_type.
 *
 * Parameters:
 * -persistRecord_t * input_record
 * -void * return_data, buffer for data to be returned
 *
 * Return value:
 * -<0 if error
 * -(pass by reference)pointer to record
 *END**************************************************************************/
int32_t Persist_flash_extract_record_data(persistRecord_t * input_record, void * return_data)
{
	enum buffer_shared_type type;
	uint32_t type_length;

	if(input_record == NULL || return_data == NULL)
	{
		/* todo:: more verbose error message */
		return -1;
	}

	type = Persist_get_record_type(input_record, NULL);

	if(type < 0)
	{
		/* todo:: more verbose error message */
		return -1;
	}

	/* unpack the length of the type */
	type_length = BUFFER_SHARED_SIZE_GET(type);

	/* now simply copy the record data to the return_data location */

	memcpy(return_data, (void *)input_record->uString, type_length);

}
#endif

/***********************************************************************************************************************************************************
 *                                                             Private Function Declarations
 *
 *
 *
 *
 ************************************************************************************************************************************************************/

#if FUNCTION_DEPRICATED
/*FUNCTION**********************************************************************
 * Function Name : Flash_getNumberTotalRecords
 * Description   : walk through FTFE flash and scan to find first record (each
 * record has a static ID associated). Then walk through the flash at
 * sizeof(persistRecord_t) bytes at a time to walk and find each subsequent record.
 * Increment a count for each record found until the first nonrecord is hit.
 *
 ***NOTE**
 * This only checks for the struct ID which is statically set. Probability of hitting
 * a 4 byte word that is not in a record is probably relatively high, making this
 * function depricated. Use get total valid count.
 ***NOTE**
 *
 * (TODO:: is it necessary to perhaps scan the addresses after the first nonrecord
 * to validate that no more records exist after. If records to exist that means
 * some error in flash writes occurred and we became misaligned --flag for a
 * clean and rewrite/write back?)
 *
 * Parameters:
 * -void
 *
 * Return value:
 * -number of records that exist
 * -<0 if looking across improper boundaries
 *END**************************************************************************/
#if (defined(FLASH_DEBUG_MODE) && !FLASH_DEBUG_MODE)
static int32_t Persist_flash_get_total_record_count(void * base, uint32_t bound_length_bytes)
#else
int32_t Persist_flash_get_total_record_count(void * base, uint32_t bound_length_bytes)
#endif
{
#if 0
	int32_t record_count = 0;

	void * recordPtr;

	void * bounds = (void *)(((char *)base + bound_length_bytes) - 1 );

	if(((uint32_t)bounds) >= PERSIST_FLASH_END_OF_PERSISTENCE || (uint32_t)base < PERSIST_FLASH_BASE)
	{
		record_count = -1;
		goto END_OF_RECORDS;
	}

	/* point to the first assumed place records should be kept */
	recordPtr = base;

	/* scroll through flash until you found all persist records OR until end of bank reached */
	while(Persist_flash_record_exists(recordPtr))
	{
		/* increment record count */
		record_count++;

		if((uint32_t)((char *)recordPtr + 1) >= (uint32_t)bounds)
		{
			goto END_OF_RECORDS;
		}
		else
		{
			/* go to the next struct if it exists */
			recordPtr = (void *)((char *)recordPtr + 1);
		}
	}
#endif

	void * end_of_bank;
	void * scanner_index;
	persistRecord_t * record_pointer;

	uint32_t data_length_found;
	uint32_t data_length_padding_found;
	uint32_t current_record_size;
	uint32_t flash_memory_offset;

	if( Persist_flash_range_check( base, bound_length_bytes ) < 0 )
	{
		record_count = -1;
		goto END_OF_RECORDS;
	}

	/* determine where the end of bank is */
	if(((uint32_t)base >= PERSIST_FLASH_FIRST_BANK_BASE) && ((uint32_t)base < PERSIST_FLASH_SECOND_BANK_BASE))
	{
		end_of_bank = (void *)((char *)base + PERSIST_FIRST_BANK_SIZE);
	}
	else
	if(((uint32_t)base >= PERSIST_FLASH_SECOND_BANK_BASE) && ((uint32_t)base < PERSIST_FLASH_END_OF_PERSISTENCE))
	{
		end_of_bank = (void *)((char *)base + PERSIST_SECOND_BANK_SIZE);
	}

//NOTICE: possible uninitialized 'end_of_bank'

	record_count = 0;
	flash_memory_offset = 0;
	/* start address for scanning */
	scanner_index = (void *)((char *)base + flash_memory_offset);

	//NOTICE: possible uninitialized 'end_of_bank'
	while(((uint32_t)scanner_index) < ((uint32_t)end_of_bank))
	{
		/* check if we have a record */
		record_pointer = (persistRecord_t *)(scanner_index);

		if(record_pointer->persist_record_header.struct_id == RECORD_STRUCT_ID)
		{
			if(Persist_flash_record_valid(scanner_index))
			{
				data_length_found = PERSIST_RECORD_DATA_LENGTH_GET(record_pointer->persist_record_header.record_id);
				data_length_padding_found = PERSIST_RECORD_DATA_LENGTH_PADDING(data_length);

				/* get all of the size information and index to a point after the record */
				current_record_size = sizeof(record_pointer->persist_record_header) + \
									  data_length_found +\
									  data_length_padding_found +\
									  sizeof(record_pointer->checksum);

				/* offset the offset to point to after the found record*/
				flash_memory_offset += current_record_size;
			}
			else
			{
				/* look at the next byte */
				flash_memory_offset++;
			}

		}

		/* look at the next byte */
		flash_memory_offset++;
	}

END_OF_RECORDS:
	return record_count;

}
#endif

/*FUNCTION**********************************************************************
 * Function Name : Persist_record_checksum
 * Description   : Perform a fletcher-32 checksum on the specified record
 *
 * Parameters:
 * -persistRecord_t * record
 *
 * Return value:
 * -<0 if NULL
 * -64 bit checksum
 *END**************************************************************************/
#if (defined(FLASH_DEBUG_MODE) && !FLASH_DEBUG_MODE)
static uint64_t Persist_record_checksum(persistRecord_t * record)
#else
uint64_t Persist_record_checksum(void * record)
#endif
{
	uint64_t checksum_value = 0;
	uint32_t checksum_0 = 0;
	uint32_t checksum_1 = 0;
	uint32_t temp_value = 0;

	persistRecord_t * record_ptr;

	if(record == NULL)
	{
		return 0;
	}
	else
	{
#if defined(CHECKSUM_FLETCHER_32) && CHECKSUM_FLETCHER_32

		record_ptr = (persistRecord_t *)record;

		uint32_t data_length = PERSIST_RECORD_DATA_LENGTH_GET(record_ptr->persist_record_header.record_id);
		uint32_t data_length_padding = PERSIST_RECORD_DATA_LENGTH_PADDING(data_length);
		uint32_t data_length_total = data_length + data_length_padding;



		uint32_t record_word_index = 0;
		uint32_t * record_word_pointer;

		/* compute the checksum over the valid data */
		/* we don't know if we are looking at a flash record or ram record
		 * but it doesn't matter since we're looking at the first n data bytes (header + data)
		 */
		uint32_t number_of_words = 0;
		uint32_t record_size = sizeof(persistRecordHeader_t) + data_length_total;

		/* convert to amount of 4 Byte words (uint32_t's) -- 4 byte record data creation must be enforced*/
		/* note: since our data is padded, 4 byte alignment is guaranteed */
		number_of_words = record_size >> 2; /* floor */
		if((record_size & 0x03) > 0) //TODO: Self to Peter - should this be an OR instead?
		{
			number_of_words++;
		}

		/* walk through the record a uint32_t amount at a time */
		record_word_pointer = (uint32_t *)record;

		for(record_word_index = 0; record_word_index < number_of_words; record_word_index++)
		{
			/* get a 4 byte word */
			temp_value = *(record_word_pointer + record_word_index);

			/* do the checksum */
			checksum_0 = (checksum_0 + temp_value) % (0xFFFFFFFF);
			checksum_1 = (checksum_1 + checksum_0) % (0xFFFFFFFF);
		}

		/* final checksum is the combination of the two in a single 64bit word */
		checksum_value = ((uint64_t)checksum_0 << 32) | checksum_1;

#endif

	}

	return checksum_value;
}

/*FUNCTION**********************************************************************
 * Function Name : fletcher_checksum
 * Description   : Perform a fletcher-32 checksum on a piece of data. Must
 * be provided with the number of 32-bit words that will be iterated over
 * as well as the pointer to the data in question.
 *
 * Parameters:
 * -uint32_t number_of_words
 * -void * data_ptr
 *
 * Return value:
 * -<0 if NULL
 * -64 bit checksum
 *END**************************************************************************/
#if (defined(FLASH_DEBUG_MODE) && !FLASH_DEBUG_MODE)
static uint64_t fletcher_checksum( uint32_t number_of_words, void * data_ptr)
#else
uint64_t fletcher_checksum( uint32_t number_of_words, void * data_ptr)
#endif
{
	int checksum_value = 0;

	if(data_ptr != NULL)
	{
		uint32_t checksum_0 = 0;
		uint32_t checksum_1 = 0;
		uint32_t temp_value = 0;

		uint32_t record_word_index = 0;
		uint32_t * record_word_pointer;

		/* walk through the record a uint32_t amount at a time */
		record_word_pointer = (uint32_t *)data_ptr;

		for(record_word_index = 0; record_word_index < number_of_words; record_word_index++)
		{
			/* get a 4 byte word */
			temp_value = *(record_word_pointer + record_word_index);

			/* do the checksum */
			checksum_0 = (checksum_0 + temp_value) % (0xFFFFFFFF);
			checksum_1 = (checksum_1 + checksum_0) % (0xFFFFFFFF);
		}

		/* final checksum is the combination of the two in a single 64bit word */
		checksum_value = ((uint64_t)checksum_0 << 32) | checksum_1;
	}

	return checksum_value;
}

/*FUNCTION**********************************************************************
 * Function Name : Flash_getTimestampConfig
 * Description   : get the number of valid records and index into the last valid
 * record to find both A.) the type of timestamp last used and B.) the timestamp
 * value last used. This will be used to continue timestamping correctly. If no
 * timestamps found or no records written, default values are used.
 *
 * (TODO:: basic mode entails each record simply holding an incrimented value
 * for each record writen. Count continues from the last valid record count value.
 * This will cap off at ~4.5billion records so a realtime clock conversion will be
 * necessary at some point or julian time timestamp.)
 *
 * Parameters:
 * -void
 *
 * Return value:
 * **following are encapsulated in an internally accessed structure**
 * -value of last record timestamp
 * -type of timestamp mechanism
 *END**************************************************************************/
#if (defined(FLASH_DEBUG_MODE) && !FLASH_DEBUG_MODE)
static void Persist_get_timestamp_config(apexTimestamp_t * t_config)
#else
void Persist_get_timestamp_config(apexTimestamp_t * t_config)
#endif
{
	uint32_t timestamp_type;
	uint32_t timestamp_value;

//	uint32_t number_valid_records;
	void * opaque_last_valid_record;
	persistRecord_t * last_valid_record_in_flash;

	/* returns the pointer to a valid record, if none exist then its null */
	Persist_flash_get_valid_record_count(&opaque_last_valid_record);

	last_valid_record_in_flash = (persistRecord_t *)opaque_last_valid_record;

	if(last_valid_record_in_flash != NULL)
	{
		/* we have access to the last written record */
		timestamp_value = last_valid_record_in_flash->persist_record_header.timestamp;

		/* extract the timestamp configuration type */
		/*timestamp_type = last_record_in_flash->record_id & 0x0000000F;*/
		timestamp_type = PERSIST_RECORD_TIMECONFIG_GET(last_valid_record_in_flash->persist_record_header.record_id);
	}
	else
	{
		timestamp_value = 0;
		timestamp_type  = PERSIST_FLASH_TIMESTAMP_MODE;
	}

	t_config->timestamp_configuration = timestamp_type;
	t_config->timestamp_value = timestamp_value;

}

/*FUNCTION**********************************************************************
 * Function Name : Flash_recordExists
 * Description   : Looks at a specified pointer and checks if a record exists
 * at that point in flash memory by checking the record struct ID. Simple
 * function to keep initialization code cleaner and verbose.
 *
 * Parameters:
 * -pointer to record location in flash
 *
 * Return value:
 * +1 if exists
 * - ~1 if doesn't exist
 *END**************************************************************************/
#if (defined(FLASH_DEBUG_MODE) && !FLASH_DEBUG_MODE)
static int32_t Persist_flash_record_exists(persistRecord_t * recordPtr)
#else
int32_t Persist_flash_record_exists(persistRecord_t * recordPtr)
#endif
{
	/* check if the flash record exists */

	if(recordPtr->persist_record_header.struct_id == RECORD_STRUCT_ID )
	{
		return 1;
	}

	return 0;

}

/*FUNCTION**********************************************************************
 * Function Name : Flash_recordValid
 * Description   : Looks at a specified record IN FLASH and calculates a checksum
 * to see if the record is actually valid. DOES NOT CHECK IF RECORD EXISTS.
 *
 * Return value:
 * +1 if valid
 * -~1 if invalid
 *END**************************************************************************/
#if (defined(FLASH_DEBUG_MODE) && !FLASH_DEBUG_MODE)
static int32_t Persist_flash_record_valid(persistRecord_t * recordPtr)
#else
int32_t Persist_flash_record_valid(void * recordPtr)
#endif
{
	uint32_t data_length;
	uint32_t data_length_padding;

	persistRecord_t * persist_record_ptr;

	uint64_t checksum;
	uint32_t checksum_index;
	uint64_t * checksum_flash_ptr;

	persist_record_ptr = (persistRecord_t *)recordPtr;

	if(Persist_flash_record_exists(persist_record_ptr) && !((uint32_t)persist_record_ptr > END_OF_FLASH_MEMORY))
	{
		data_length = PERSIST_RECORD_DATA_LENGTH_GET(persist_record_ptr->persist_record_header.record_id);
		data_length_padding = PERSIST_RECORD_DATA_LENGTH_PADDING(data_length);

		/* re-calculate checksum */
		checksum = Persist_record_checksum(recordPtr);

		/* point to the checksum in flash */
		checksum_index = sizeof(persistRecordHeader_t) + data_length + data_length_padding;

		checksum_flash_ptr = (uint64_t *)((char *)recordPtr + checksum_index);

		if(*(checksum_flash_ptr) == checksum)
		{
			return 1;
		}
	}

	return 0;
}

/*FUNCTION**********************************************************************
 * Function Name : ram_recordValid
 * Description   : Looks at a specified record and calculates a checksum to see
 * if the record is actually valid. DOES NOT CHECK IF RECORD EXISTS.
 *
 * Return value:
 * +1 if valid
 * -~1 if invalid
 *END**************************************************************************/
#if (defined(FLASH_DEBUG_MODE) && !FLASH_DEBUG_MODE)
static int32_t Persist_flash_record_valid(persistRecord_t * recordPtr)
#else
int32_t Persist_ram_record_valid(persistRecord_t * recordPtr)
#endif
{
	uint64_t checksum;

	/* enforce RAM locations */
	if(Persist_flash_record_exists(recordPtr) &&(uint32_t)recordPtr > END_OF_FLASH_MEMORY)
	{
		/* re-calculate checksum */
		checksum = Persist_record_checksum(recordPtr);

		if(recordPtr->checksum == checksum)
		{
			return 1;
		}
	}



	return 0;
}

#if 0
/*FUNCTION**********************************************************************
 * Function Name : Persist_get_record_type
 * Description   : Simple helper method to grab the record type of a record.
 *
 *
 * Parameters:
 * -persistRecord_t * record pointer
 * -uint8_t * isString, <not required>
 *
 * Return value:
 * -<0 if record is invalid
 * -enum buffer_shared_type of the record
 * -(pass by reference) isString = 1 if string else 0
 *END**************************************************************************/
#if (defined(FLASH_DEBUG_MODE) && !FLASH_DEBUG_MODE)
static enum buffer_shared_type Persist_get_record_type(persistRecord_t * record, uint8_t ** isString)
#else
enum buffer_shared_type Persist_get_record_type(persistRecord_t * record, uint8_t * isString)
#endif
{
	uint32_t record_type;
	uint32_t buffer_index;

	if(isString != NULL)
	{
		/* default to no string */
		*isString = 0;
	}

	if( (Persist_flash_record_valid(record) || Persist_ram_record_valid(record)))
	{
		/* we have a real record */

		/* get the record type */
		record_type = PERSIST_RECORD_TYPE_GET(record->persist_record_header.record_id); /*record->record_id >> 4;*/

		/* cast to uin32_t because compiler behavior can change underlying type of enums */
		switch(record_type)
		{
			case (uint32_t)BUFFER_SHARED_TYPE__UINT8:
				return BUFFER_SHARED_TYPE__UINT8;
				break;

			case (uint32_t)BUFFER_SHARED_TYPE__UINT16:
				return BUFFER_SHARED_TYPE__UINT16;
				break;

			case (uint32_t)BUFFER_SHARED_TYPE__UINT32:
				return BUFFER_SHARED_TYPE__UINT32;
				break;

			case (uint32_t)BUFFER_SHARED_TYPE__INT8:
				return BUFFER_SHARED_TYPE__INT8;
				break;

			case (uint32_t)BUFFER_SHARED_TYPE__CHAR:

				/* cull through the buffer to find a null character */
				for(buffer_index = 0; buffer_index < CONTAINER_BYTE_LENGTH; buffer_index++)
				{
					if(record->container[buffer_index] == '\0' && isString != NULL)
					{
						*isString = 1;
					}
				}

				return BUFFER_SHARED_TYPE__CHAR;
				break;

			case (uint32_t)BUFFER_SHARED_TYPE__INT16:
				return BUFFER_SHARED_TYPE__INT16;
				break;

			case (uint32_t)BUFFER_SHARED_TYPE__INT32:
				return BUFFER_SHARED_TYPE__INT32;
				break;

			case (uint32_t)BUFFER_SHARED_TYPE__INTEGER:
				return BUFFER_SHARED_TYPE__INTEGER;
				break;

			case (uint32_t)BUFFER_SHARED_TYPE__FLOAT:
				return BUFFER_SHARED_TYPE__FLOAT;
				break;

			case (uint32_t)BUFFER_SHARED_TYPE__DOUBLE:
				return BUFFER_SHARED_TYPE__DOUBLE;
				break;

			case (uint32_t)BUFFER_SHARED_TYPE__VPOINT:
				return BUFFER_SHARED_TYPE__VPOINT;
				break;

			default:
				return -1;
				break;
		}

	}

	return -1;
}
#endif

/*FUNCTION**********************************************************************
 * Function Name : Persist_get_record_tag
 * Description   : Simple helper method to grab the tag of a record.
 *
 *
 * Parameters:
 * -persistRecord_t * record pointer
 *
 * Return value:
 * -<0 if record is invalid
 * -enum buffer_shared_type of the record
 * -(pass by reference) isString = 1 if string else 0
 *END**************************************************************************/
#if (defined(FLASH_DEBUG_MODE) && !FLASH_DEBUG_MODE)
static int32_t Persist_get_record_tag(persistRecord_t * record)
#else
int32_t Persist_get_record_tag(persistRecord_t * record)
#endif
{
	int32_t tag;

	if(record == NULL)
	{
		return -1;
	}

	/* extract the record tag if record exists*/
	if ( Persist_flash_record_exists(record) && ( Persist_flash_record_valid(record) || Persist_ram_record_valid(record) ) )
	{
		tag = (int32_t) record->persist_record_header.record_tag;
	}
	else
	{
		return -1;
	}
	return tag;
}

/*FUNCTION**********************************************************************
 * Function Name : Persist_get_record_size_interanl
 * Description   : Calculates the size of a record when its committed to flash.
 *
 *
 * Parameters:
 * -persistRecord_t * record pointer
 *
 * Return value:
 * -<0 if record is invalid
 * -size of the record
 *END**************************************************************************/
#if (defined(FLASH_DEBUG_MODE) && !FLASH_DEBUG_MODE)
static uint32_t Persist_get_record_size_internal(persistRecord_t * record)
#else
int32_t Persist_get_record_size_internal(persistRecord_t * record)
#endif
{
	int32_t trouble;
	uint32_t record_size;

	uint32_t data_length;
	uint32_t data_length_padding;

	if(record == NULL)
	{
		trouble = -1;
		goto GET_RECORD_SIZE_INTERNAL_ERROR_HANDLER;
	}

	/* extract the record tag if record exists*/
	if ( Persist_flash_record_exists(record) && ( Persist_flash_record_valid(record) || Persist_ram_record_valid(record) ) )
	{
		data_length = PERSIST_RECORD_DATA_LENGTH_GET(record->persist_record_header.record_id);
		data_length_padding = PERSIST_RECORD_DATA_LENGTH_PADDING(data_length);

		record_size = sizeof(record->persist_record_header) + \
				data_length +\
					  data_length_padding +\
				      sizeof(record->checksum);

		trouble = record_size;
	}
	else
	{
		return trouble = -1;;
	}

GET_RECORD_SIZE_INTERNAL_ERROR_HANDLER:
	return trouble;
}

/*FUNCTION**********************************************************************
 * Function Name : Flash_recordCopyToRam
 * Description   : Takes in a record from ram and copies to it a valid record in
 * flash specified by a pointer.
 *
 * Parameters:
 * -persistRecord_t * record_in_ram is the record to be copied to
 * -persistRecord_t * record_in_flash is the record in flash to be copied from
 *
 * Return value:
 * -<0 if record in flash is non valid
 * ->0 for successful copy
 *END**************************************************************************/
#if (defined(FLASH_DEBUG_MODE) && !FLASH_DEBUG_MODE)
static int32_t Persist_Flash_Record_Copy_To_RAM(persistRecord_t * record_in_ram, persistRecord_t * record_in_flash)
#else
int32_t Persist_Flash_Record_Copy_To_RAM(persistRecord_t * record_in_ram, void * record_in_flash)
#endif
{
	uint32_t data_length;
	uint32_t data_length_padding;
	void * flash_record_index;

	if(record_in_ram == NULL || record_in_flash == NULL)
	{
		return -1;
	}

	flash_record_index = (void *)record_in_flash;



	/* make sure user passes in an actual record in RAM */
	/* we don't want to accidently try writing to flash */
	if((uint32_t)record_in_ram > END_OF_FLASH_MEMORY)
	{
		/* check if the record in flash is valid */
		if(Persist_flash_record_exists((persistRecord_t *)record_in_flash) && Persist_flash_record_valid(record_in_flash))
		{
			/* clear out the record to be copied into */
			memset((void *)(record_in_ram), 0, sizeof(persistRecord_t));

			/* copy the persist header */
			memcpy((void *)record_in_ram, (void *)flash_record_index, sizeof(persistRecordHeader_t));

			/* copy the payload */
			data_length = PERSIST_RECORD_DATA_LENGTH_GET(record_in_ram->persist_record_header.record_id);
			data_length_padding = PERSIST_RECORD_DATA_LENGTH_PADDING(data_length);
			flash_record_index += sizeof(persistRecordHeader_t); /* index into the payload in flash */

			memcpy((void *)(&(record_in_ram->container)), (void *)flash_record_index, (data_length + data_length_padding));

			/* copy the checksum */
			flash_record_index += (data_length + data_length_padding); /* index into flash checksum */

			memcpy((void *)(&(record_in_ram->checksum)), (void *)flash_record_index, sizeof(uint64_t));

			/* validate that it was copied correctly */
			if(Persist_ram_record_valid(record_in_ram))
			{
				return 1;
			}
		}
	}


	return -1;
}

/*FUNCTION**********************************************************************
 * Function Name : Flash_getRecord
 * Description   : Sends a request out by setting a flag then tries to take a
 * semaphore (TODO: what kind of semaphore, we need to daisy chain all high
 * priority hardware/software tasks that need to retain states with the semaphore
 * so all of them know their last known state before the flash clean up routine
 * takes over. All of these critical tasks will also have to take a semaphore and
 * hang until the flash clean is complete). Once the semaphore is given, it will
 * shut off all interrupts and go into a critical mode. During critical mode, it
 * will copy all current valid record histories into ram, then it will write them
 * to the secondary bank. Then it will erase the primary bank, then it will write
 * back all of the secondary bank valid histories.
 *
 * (TODO:: functionality will have to change later where the secondary bank can
 * become the primary bank and record histories are copied (something like
 * flash_transferPrimaryBank) and we will ignore erasing the original primary bank.
 * We just keep writing to the secondary bank. Upon both banks becoming full,
 * cleanRecordBankRequest will clean up the original bank and transfer the primary
 * bank again -- or the cleanRecordBankRequest can erase the original primary
 * during idle modes even while the new primary is still not full). For sake of
 * time we will simply use the secondary bank as a copy-write-back buffer for now
 * and the primary bank is always the primary bank.
 *
 *
 * Parameters:
 * -void
 *
 * Return value:
 * -NULL if cleanup failed
 * -pointer to new last valid location
 *END**************************************************************************/
#if (defined(FLASH_DEBUG_MODE) && !FLASH_DEBUG_MODE)
static int32_t Persist_flash_record_bank_clean_request(persistRecord_t ** last_valid_record)
#else
int32_t Persist_flash_record_bank_clean_request(persistRecord_t ** last_valid_record)
#endif
{
	flash_security_state_t securityStatus = kFLASH_SecurityStateNotSecure; /* Return protection status */
	flash_config_t s_flashDriver;
//	uint32_t i, failAddr, failDat;
	status_t result;

	/*TODO:: add semaphores and a request flag for cleaning */
	persistRecord_t * second_bank_pointer;
	persistRecord_t * first_bank_pointer;
	persistRecord_t * primary_bank_iterator;
	persistRecord_t * secondary_bank_iterator;

	uint8_t record_saved;
	uint32_t number_of_history_records_returned;

	uint32_t primary_record_tag;
	uint32_t secondary_record_tag;

	persistRecord_t * type_record_history[PERSIST_FLASH_MAX_HISTORY];
	persistRecord_t	  type_record_history_dummy;

	uint32_t record_iterator;
	int32_t copy_result;
	int32_t program_result;

	/* we want to return at least null if a condition never sets this pointer */
	if(last_valid_record != NULL)
	{
		last_valid_record = NULL;
	}

	/* get our pointers to our respective banks */
	first_bank_pointer = (persistRecord_t *)PERSIST_FLASH_FIRST_BANK_BASE;
	second_bank_pointer = (persistRecord_t *)PERSIST_FLASH_SECOND_BANK_BASE;

	primary_bank_iterator = first_bank_pointer;
	secondary_bank_iterator = second_bank_pointer;

	/* Setup flash driver structure for device and initialize variables. */
	/* Clean up Flash driver Structure*/
	memset(&s_flashDriver, 0, sizeof(flash_config_t));

	result = FLASH_Init(&s_flashDriver);
	if (kStatus_FLASH_Success != result)
	{
	    return -1;
	}

	/* Check security status. */
	result = FLASH_GetSecurityState(&s_flashDriver, &securityStatus);
	if (kStatus_FLASH_Success != result)
	{
		return -1;
	}

	/* Erase the entire persistent section */
	if (kFLASH_SecurityStateNotSecure == securityStatus)
	{
	    /* Erase Persistent section. */
	    result = FLASH_Erase(&s_flashDriver, (uint32_t)PERSIST_SECOND_BANK_SECTOR_COUNT, (uint32_t)(PERSIST_SECOND_BANK_SIZE), kFLASH_ApiEraseKey);
	    if (kStatus_FLASH_Success != result)
	    {
	        return -1;
	    }

	    /* Verify sector if it's been erased. */
	    result = FLASH_VerifyErase(&s_flashDriver, (uint32_t)PERSIST_SECOND_BANK_SECTOR_COUNT, (uint32_t)(PERSIST_SECOND_BANK_SIZE), kFLASH_MarginValueUser);
	    if (kStatus_FLASH_Success != result)
	    {
	         return -1;
	    }

	}
	else
	{
	     return -1;
	}


	/* TODO:: this is basically a backup function -- make into its own function for code clarity */

	/* start saving histories to back up sector */
	while(Persist_flash_record_exists(primary_bank_iterator))
	{

		if(Persist_flash_record_valid(primary_bank_iterator))
		{
			/* get record type, see if the secondary bank has this type */
			primary_record_tag = PERSIST_RECORD_TAG_GET(primary_bank_iterator->persist_record_header.record_id);

			record_saved = 0;

			/* iterate through the second bank to see if any records of this type exist */
			while(Persist_flash_record_exists(secondary_bank_iterator))
			{
				if(Persist_flash_record_valid(secondary_bank_iterator))
				{
					secondary_record_tag = PERSIST_RECORD_TAG_GET(secondary_bank_iterator->persist_record_header.record_id);

					if(secondary_record_tag == primary_record_tag)
					{
						record_saved = 1;
					}
				}

				secondary_bank_iterator = secondary_bank_iterator + 1;

				/* check that we aren't crossing boundaries */
				if((uint32_t)secondary_bank_iterator >= PERSIST_FLASH_END_OF_PERSISTENCE)
				{
					break;
				}
			}

			/* check if the record was saved off */

			if(!record_saved)
			{
				/* record has not yet been saved so get a max history on it */
				number_of_history_records_returned = Persist_flash_get_record_history(primary_record_tag,
																					  PERSIST_FLASH_MAX_HISTORY,
																					  type_record_history);

				/* for each record returned, copy to ram then write to secondary bank */
				for(record_iterator = 0; record_iterator < number_of_history_records_returned; record_iterator++)
				{
					copy_result = Persist_Flash_Record_Copy_To_RAM(&type_record_history_dummy, type_record_history[record_iterator]);

					if(copy_result < 0)
					{
						return -1;
					}
					else
					{
						/* write to flash */
						program_result = Persist_flash_write_record(&type_record_history_dummy, (void *) PERSIST_FLASH_SECOND_BANK_BASE);

						if ( SUCCESS != program_result )
						{
							return -program_result;
						}
					}
				}
			}
		}

		primary_bank_iterator = primary_bank_iterator + 1;
		/* check that we aren't crossing boundaries */
		if((uint32_t)primary_bank_iterator >= PERSIST_FLASH_SECOND_BANK_BASE)
		{
			break;
		}
	}

	/* now erase the primary block */
	/* Erase the entire persistent section */
	if (kFLASH_SecurityStateNotSecure == securityStatus)
	{
	    /* Erase Persistent section. */
	    result = FLASH_Erase(&s_flashDriver, (uint32_t)PERSIST_FLASH_FIRST_BANK_BASE, (uint32_t)(PERSIST_FIRST_BANK_SIZE), kFLASH_ApiEraseKey);
	    if (kStatus_FLASH_Success != result)
	    {
	        return -1;
	    }

	    /* Verify sector if it's been erased. */
	    result = FLASH_VerifyErase(&s_flashDriver, (uint32_t)PERSIST_FLASH_FIRST_BANK_BASE, (uint32_t)(PERSIST_FIRST_BANK_SIZE), kFLASH_MarginValueUser);
	    if (kStatus_FLASH_Success != result)
	    {
	         return -1;
	    }

	}
	else
	{
	     return -1;
	}

	/* now copy over everything from the secondary bank back into the primary bank */

	secondary_bank_iterator = second_bank_pointer;

	while(Persist_flash_record_exists(secondary_bank_iterator))
	{
		if(Persist_flash_record_valid(secondary_bank_iterator))
		{
			if(last_valid_record != NULL)
			{
				*last_valid_record = secondary_bank_iterator;
			}
			copy_result = Persist_Flash_Record_Copy_To_RAM(&type_record_history_dummy, secondary_bank_iterator);

			if(copy_result < 0)
			{
				return -1;
			}
			else
			{
				/* write to flash */
				program_result = Persist_flash_write_record(&type_record_history_dummy, (void *) PERSIST_FLASH_FIRST_BANK_BASE);

				if ( SUCCESS != program_result )
				{
					return -program_result;
				}
			}
		}

		secondary_bank_iterator = secondary_bank_iterator + 1;

		/* check that we aren't crossing boundaries */
		if((uint32_t)secondary_bank_iterator >= PERSIST_FLASH_END_OF_PERSISTENCE)
		{
			break;
		}
	}

	return SUCCESS;
}

/*FUNCTION**********************************************************************
 * Function Name : Persist_flash_compare_record
 * Description   : Compares two valid records IN RAM to see if they are the same.
 * Copy record from flash to ram if you wish to compare.
 *
 * Parameters:
 * -persistRecord_t * record_a
 * -persistRecord_t * record_b
 *
 * Return value:
 * -0 if equal
 * -<0 if unequal or NULL passed in
 *END**************************************************************************/
#if (defined(FLASH_DEBUG_MODE) && !FLASH_DEBUG_MODE)
static int32_t Persist_flash_compare_record(persistRecord_t * record_a, persistRecord_t * record_b)
#else
int32_t Persist_ram_compare_record(persistRecord_t * record_a, persistRecord_t * record_b)
#endif
{
	/* records are 8 byte aligned meaning they're also 4 byte aligned */
//	uint32_t valid_a;
//	uint32_t valid_b;

	void * base_a;
	void * base_b;

	void * byte_itr_record_a;
	void * byte_itr_record_b;

	uint32_t data_length;
	uint32_t data_length_padding;
	uint32_t data_length_total;

	base_a = (void *)record_a;
	base_b = (void *)record_b;

	uint32_t header_bytes_offset;
	uint32_t data_bytes_offset;

	if(record_a == NULL || record_b == NULL)
	{
		return -1;
	}

	//* go through the header byte by byte *//

	for(header_bytes_offset = 0; header_bytes_offset < sizeof(persistRecordHeader_t); header_bytes_offset++)
	{
		byte_itr_record_a = (void *)((char *)base_a + header_bytes_offset);
		byte_itr_record_b = (void *)((char *)base_b + header_bytes_offset);

		if(*((char *)byte_itr_record_a) != *((char *)(byte_itr_record_b)))
		{
			return -1;
		}

	}

	/* headers are equal, data length and padding are equal */
	data_length = PERSIST_RECORD_DATA_LENGTH_GET(record_a->persist_record_header.record_id);
	data_length_padding = PERSIST_RECORD_DATA_LENGTH_PADDING(data_length);
	data_length_total = data_length + data_length_padding;

	base_a = (void *)((char *)byte_itr_record_a) + sizeof(persistRecordHeader_t);
	base_b = (void *)((char *)byte_itr_record_b) + sizeof(persistRecordHeader_t);

	for(data_bytes_offset = 0; data_bytes_offset < data_length_total; data_bytes_offset++)
	{
		byte_itr_record_a = (void *)((char *)base_a + data_bytes_offset);
		byte_itr_record_b = (void *)((char *)base_b + data_bytes_offset);

		if(*((char *)byte_itr_record_a) != *((char *)(byte_itr_record_b)))
		{
			return -1;
		}
	}

	/* data must be equal, so check the checksums */
	if(record_a->checksum != record_b->checksum)
	{
		return -1;
	}


	return 0;
}

/*FUNCTION**********************************************************************
 * Function Name : Persist_flash_write_record
 * Description   : Takes in a valid record in ram and writes it to flash. To do
 * this, use getNumberValid records to get the pointer to the last written valid
 * record. We will write to the next address. Only write if there is enough space
 * in our current bank. If there is not enough space in the current bank, do a
 * clean record bank request. Try writing again, if still not enough space,
 * do a clean record bank request with a smaller history count (this will likely
 * not happen but its a guard never the less). If we fail to program at the
 * specified address, then we do a clean up and try again. If still fails, then
 * the user is SOL and flash is corrupt and we have to erase and start over.
 *
 * Parameters:
 * -persistRecord_t * record_in_ram, record in ram to be programmed to flash memory
 *
 * TODO: error handling
 *
 * Return value:
 * -<0 if programming to flash failed
 * ->0 if record is programmed to flash successfully
 *END**************************************************************************/
#if (defined(FLASH_DEBUG_MODE) && !FLASH_DEBUG_MODE)
static int32_t Persist_flash_write_record(persistRecord_t * record_in_ram, void * base)
#else
int32_t Persist_flash_write_record( persistRecord_t * record_in_ram, void * base )
#endif
{
	int trouble = SUCCESS;
	status_t result;    /* Return code from each flash driver function */
	uint32_t failAddr, failDat;

//	apexTimestamp_t new_record_timestamp;
	int32_t number_records_in_flash;
	persistRecord_t * record_pointer;
	uint64_t checksum;

	uint32_t tag;
	uint32_t type;
	uint32_t data_length;
	uint32_t data_length_padding;

	uint32_t data_length_found;
	uint32_t data_length_padding_found;

	void * scanner_index = 0;
	void * subscanner_index = 0;
	void * potential_write_index;
	uint32_t current_record_size;
//	uint32_t free_bytes_found = 0;

	/* check that the record in ram exists */
	if(record_in_ram == NULL || base == NULL || record_in_ram->persist_record_header.struct_id != RECORD_STRUCT_ID)
	{
		/* invalid RAM record error */
		trouble = -EC_PERSIST_WRITE_RAM;
		goto Persist_flash_write_record_ERROR_HANDLER;
	}

	/* ram record has a tag and exists */

	/* get the time stamp configuration of the last written valid record */
	Persist_get_timestamp_config(&persist_flash_timestamp_config);

	/* compare this compiled code's timestamp type against the last timestamp used in a valid record */
	if(persist_flash_timestamp_config.timestamp_configuration != PERSIST_FLASH_TIMESTAMP_MODE)
	{
		/*todo:: conversion necessary once we introduced other modes */
		/* for now, error out:: timestamp configuration mismatch error */
		trouble = -EC_PERSIST_WRITE_TIMESTAMP;
		goto Persist_flash_write_record_ERROR_HANDLER;
	}

	/* timestamp mode is the same */
	/* we must increment the timestamp unless no previous records exist*/


	/* get the number of records that exist in flash at the moment even if invalid */
	/* TODO:: for now, both banks are of the same size, so this is okay, but what if we want
	 * variable size?
	 */
	number_records_in_flash = Persist_flash_get_valid_record_count(NULL);/* Persist_flash_get_total_record_count(base, PERSIST_FIRST_BANK_SIZE); */

	if ( 0 == number_records_in_flash )
	{
		/* rebuild the record ID */
		tag = 0; /* DEPRICATED -- NEEDS TO BE CLEANEDUP */ //PERSIST_RECORD_TAG_GET(record_in_ram->persist_record_header.record_id);
		type = PERSIST_RECORD_TYPE_GET(record_in_ram->persist_record_header.record_id);
		data_length = PERSIST_RECORD_DATA_LENGTH_GET(record_in_ram->persist_record_header.record_id);
		data_length_padding = PERSIST_RECORD_DATA_LENGTH_PADDING(data_length);

		record_in_ram->persist_record_header.record_id |= PERSIST_RECORD_ID_BUILD( type, persist_flash_timestamp_config.timestamp_configuration, data_length, tag);
		record_in_ram->persist_record_header.timestamp  = persist_flash_timestamp_config.timestamp_value;
		//record_in_ram->persist_record_header.record_tag = tag; /* depricated -- needs to be cleaned */
	}
	else
	{
		/* get timestamp gave us back a legitimate last timestamp */
		/* update said timestamp and write it to new record */
		APEX_timestamp_update(&persist_flash_timestamp_config);

		/* rebuild the record ID */
		tag = 0;/* DEPRICATED -- NEEDS TO BE CLEANEDUP */ //PERSIST_RECORD_TAG_GET(record_in_ram->persist_record_header.record_id);
		type = PERSIST_RECORD_TYPE_GET(record_in_ram->persist_record_header.record_id);
		data_length = PERSIST_RECORD_DATA_LENGTH_GET(record_in_ram->persist_record_header.record_id);
		data_length_padding = PERSIST_RECORD_DATA_LENGTH_PADDING(data_length);

		record_in_ram->persist_record_header.record_id |= PERSIST_RECORD_ID_BUILD( type, persist_flash_timestamp_config.timestamp_configuration, data_length, tag);
		record_in_ram->persist_record_header.timestamp  = persist_flash_timestamp_config.timestamp_value;
		//record_in_ram->persist_record_header.record_tag = tag; /* depricated -- needs to be cleaned */
	}


	/* calculate the checksum and store it*/
	checksum = Persist_record_checksum(record_in_ram);
	record_in_ram->checksum = checksum;

	/* *****NOW WE BEGIN WRITING TO FLASH USING FLASH API FROM FLS_FLASH.h***** */

	void * end_of_bank = NULL;

	uint32_t flash_memory_offset = 0;
	uint32_t free_byte_index = 0;
	uint32_t write_index_found = 0;

	/* determine where the end of bank is */
	if(((uint32_t)base >= PERSIST_FLASH_FIRST_BANK_BASE) && ((uint32_t)base < PERSIST_FLASH_SECOND_BANK_BASE))
	{
		end_of_bank = (void *)((char *)base + PERSIST_FIRST_BANK_SIZE);
	}
	else
	if(((uint32_t)base >= PERSIST_FLASH_SECOND_BANK_BASE) && ((uint32_t)base < PERSIST_FLASH_END_OF_PERSISTENCE))
	{
		end_of_bank = (void *)((char *)base + PERSIST_SECOND_BANK_SIZE);
	}

	/* start address for scanning */
	scanner_index = (void *)((char *)base + flash_memory_offset);

	while(((uint32_t)scanner_index) < ((uint32_t)end_of_bank))
	{
		/* check if we have a record */
		record_pointer = (persistRecord_t *)(scanner_index);

		if(record_pointer->persist_record_header.struct_id == RECORD_STRUCT_ID)
		{
			if(Persist_flash_record_valid(scanner_index))
			{
				data_length_found = PERSIST_RECORD_DATA_LENGTH_GET(record_pointer->persist_record_header.record_id);
				data_length_padding_found = PERSIST_RECORD_DATA_LENGTH_PADDING(data_length_found);

				/* get all of the size information and index to a point after the record */
				current_record_size = sizeof(record_pointer->persist_record_header) + \
									  data_length_found +\
									  data_length_padding_found +\
									  sizeof(record_pointer->checksum);

				/* offset the offset to point to after the found record*/
				flash_memory_offset += current_record_size;
			}
			else
			{
				/* look at the next byte */
				flash_memory_offset++;
			}

		}
		else
		{
			/*check and see if this byte was NOT written to -- only pay attention if the index is 8byte aligned */
			if( (((uint32_t)scanner_index & 0x07) == 0) && (*((uint8_t *)scanner_index) == 0xFF))
			{
				free_byte_index = 0;

				potential_write_index = scanner_index;

				subscanner_index = (void *)((char *)potential_write_index + free_byte_index);

				/* check ahead and see if we have enough free bytes */
				while(subscanner_index < (end_of_bank))
				{
					if(*((uint8_t *)subscanner_index) == 0xFF)
					{
						free_byte_index++;

						if(free_byte_index >= ( sizeof(persistRecordHeader_t) + data_length + data_length_padding + sizeof(uint64_t)))
						{
							/* write position was found, we have a big enough space for the record */
							/* flag that we found a big enough space */
							write_index_found = 1;
							goto FOUND_WRITE_POSITION;
						}

						/* update subscanner position */
						subscanner_index = (void *)((char *)potential_write_index + free_byte_index);
					}
					else
					{
						break;
					}
				}

				/* if we never went to 'found_write_position' then there were not enough free bytes */
				/* skip these bytes over -- this is likely due to corruption */
				flash_memory_offset += free_byte_index;
			}
			else
			{
				/* look at the next byte */
				flash_memory_offset++;
			}
		}

		/* increment scanner index */
		scanner_index = (void *)((char *)base + flash_memory_offset);
	}

FOUND_WRITE_POSITION:

	if(write_index_found)
	{
		/* we have an aligned address at which we can store the data with appropriate size */

	    /* program the record into flash only if security allows us */
	    if (kFLASH_SecurityStateNotSecure == securityStatus)
	    {
	        /* Program record into flash memory*/

	    	/* **********PROGRAM HEADER INTO FLASH********** */

	        result = FLASH_Program(&s_flashDriver, (uint32_t)potential_write_index, (uint32_t *)(&(record_in_ram->persist_record_header)), sizeof(persistRecordHeader_t));
	        if (kStatus_FLASH_Success != result)
	        {
	        	/* return persist flash record program failure */
	        	trouble = -EC_PERSIST_WRITE_RECORD_FAIL;
	        	goto Persist_flash_write_record_ERROR_HANDLER;
	        }

	        /* Verify programming by Program Check command with user margin levels */
	        result = FLASH_VerifyProgram(	&s_flashDriver,
	        								(uint32_t)potential_write_index,
	        								sizeof(persistRecordHeader_t),
											/* not sure if this is what you intend - but there was a warning */
											(const uint32_t *)(&(record_in_ram->persist_record_header)), //MI: was (uint8_t *)&s_persistRecord,
											kFLASH_MarginValueUser,
											&failAddr, &failDat
										);
	        if (kStatus_FLASH_Success != result)
	        {
	        	/* program verify failure */
	        	trouble = -EC_PERSIST_WRITE_VERIFY;
	        	goto Persist_flash_write_record_ERROR_HANDLER;
	        }

	    	/* **********PROGRAM PAYLOAD INTO FLASH********** */
	        potential_write_index += sizeof(persistRecordHeader_t);

	        result = FLASH_Program(&s_flashDriver, (uint32_t)potential_write_index, (uint32_t *)(&(record_in_ram->container)), (data_length + data_length_padding));
	        if (kStatus_FLASH_Success != result)
	        {
	        	/* return persist flash record program failure */
	        	trouble = -EC_PERSIST_WRITE_RECORD_FAIL;
	        	goto Persist_flash_write_record_ERROR_HANDLER;
	        }

	        /* Verify programming by Program Check command with user margin levels */
	        result = FLASH_VerifyProgram(	&s_flashDriver,
	        								(uint32_t)potential_write_index,
											(data_length + data_length_padding),
											/* not sure if this is what you intend - but there was a warning */
											(const uint32_t *)(&(record_in_ram->container)), //MI: was (uint8_t *)&s_persistRecord,
											kFLASH_MarginValueUser,
											&failAddr, &failDat
										);
	        if (kStatus_FLASH_Success != result)
	        {
	        	/* program verify failure */
	        	trouble = -EC_PERSIST_WRITE_VERIFY;
	        	goto Persist_flash_write_record_ERROR_HANDLER;
	        }

	    	/* **********PROGRAM CHECKSUM INTO FLASH********** */
	        potential_write_index += data_length + data_length_padding;

	        result = FLASH_Program(&s_flashDriver, (uint32_t)potential_write_index, (uint32_t *)(&(record_in_ram->checksum)), sizeof(uint64_t));
	        if (kStatus_FLASH_Success != result)
	        {
	        	/* return persist flash record program failure */
	        	trouble = -EC_PERSIST_WRITE_RECORD_FAIL;
	        	goto Persist_flash_write_record_ERROR_HANDLER;
	        }

	        /* Verify programming by Program Check command with user margin levels */
	        result = FLASH_VerifyProgram(	&s_flashDriver,
	        								(uint32_t)potential_write_index,
											sizeof(uint64_t),
											/* not sure if this is what you intend - but there was a warning */
											(const uint32_t *)(&(record_in_ram->checksum)), //MI: was (uint8_t *)&s_persistRecord,
											kFLASH_MarginValueUser,
											&failAddr, &failDat
										);
	        if (kStatus_FLASH_Success != result)
	        {
	        	/* program verify failure */
	        	trouble = -EC_PERSIST_WRITE_VERIFY;
	        	goto Persist_flash_write_record_ERROR_HANDLER;
	        }

	    }
	    else
	    {
	    	/* flash security error */
	    	trouble = -EC_PERSIST_WRITE_SECURITY;
	    	goto Persist_flash_write_record_ERROR_HANDLER;
	    }

		//return Persist_flash_primary_sector_clean_and_shift(Primary_Bank_Pointer, Primary_Bank_Size, 0, PERSIST_FLASH_FILL_THRESHOLD, PERSIST_FLASH_MAX_HISTORY );

	}
	else
	{
		/* no large enough chunk was found for this */
		/* issue a clean/repair */
		trouble = Persist_flash_garbage_collect();

		if(trouble < 0)
		{
	    	goto Persist_flash_write_record_ERROR_HANDLER;
		}

		/* try committing the record now -- recursive */ //todo: check the resolving of this recursive method
		trouble = Persist_Flash_Record_Commit( record_in_ram );

		if(trouble < 0)
		{
	    	goto Persist_flash_write_record_ERROR_HANDLER;
		}
	}

Persist_flash_write_record_ERROR_HANDLER:
	return trouble;
}

/*FUNCTION**********************************************************************
 * Function Name : Persist_flash_determine_primary_bank
 * Description   : Scans both available banks. If valid records are found in a
 * single bank, that bank is assumed to be the primary bank. If both banks have
 * valid records stored in them, that means there was as system crash during
 * a cleanup operation in which data from one bank was stored into the redundant
 * bank but a crash happened either during the copy operation before the
 * non-redundant bank was cleared. This gives potential to the following cases:
 *
 * 1.) Crash during copy operation to redundant bank. Complete transfer failed.
 *     Redundant bank will have LESS UNIQUE VALID records than the original primary.
 *     Original primary remains primary.
 *
 * 1.) Crash during copy operation to redundant bank. Complete transfer failed.
 *     Redundant bank will have EQUAL UNIQUE VALID records as the original primary
 *     HOWEVER the history amount of records for unique tags may not have finished
 *     copying. I.E. we redundantly save up to three levels of history for each
 *     unique record; during a copy, the last set of unique historical records were
 *     being copied, but the last of the history level amount of records failed to copy.
 *     Original primary bank remains primary.
 *
 * 3.) Crash before the erase of the original primary bank. Both banks have an equal
 *     number of UNIQUE VALID RECORD TAGS, but the new primary bank should have only
 *     the number of unique tags * number of history levels amount of records.
 *
 * 4.) Crash during the erase of the old primary bank. Resolved by checking for which
 *     we check for each unique tag, see if we only have a history level
 *     amount of each unique tag. Could still accidently choose the partially erased bank
 *     if by dumb luck, the original primary bank had a history level amount per unique tag
 *     even after a partial erase -- in this case, the new primary will be the bank with
 *     more unique tags. Otherwise default to the first bank.
 *
 * Based on these possibilities, the correct primary bank will be set globally.
 *
 * Parameters:
 * -void
 *
 * TODO: error handling
 *
 * Return value:
 * -<0 if primary bank is failed to be set correctly
 * ->0 if primary bank is set correctly
 *END**************************************************************************/


#if (defined(FLASH_DEBUG_MODE) && !FLASH_DEBUG_MODE)
static int32_t Persist_flash_determine_primary_bank( void )
#else
int32_t Persist_flash_determine_primary_bank_and_recover( void )
#endif
{
	static uint32_t my_unique_record_tags_first_bank[MAX_PERSIST_RECORDS_ALLOWED] = { 0 };
	static uint32_t my_unique_record_tags_second_bank[MAX_PERSIST_RECORDS_ALLOWED] = { 0 };

	int32_t trouble = 0;

	uint32_t num_records_first_bank;
	uint32_t num_records_second_bank;

	uint32_t unique_records_first_bank_iterator;
	uint32_t unique_records_second_bank_iterator;
	uint32_t number_of_matches;

	persistRecord_t * record_ptr_bank_1;
	persistRecord_t * record_ptr_bank_2;

	uint32_t most_recent_tag_count_bank1;
	uint32_t most_recent_tag_count_bank2;

	uint32_t ptr_1_timestamp;
	uint32_t ptr_2_timestamp;

	uint32_t total_valid_bank1;
	uint32_t total_valid_bank2;

	num_records_first_bank = Persist_flash_unique_tags_get((void *)PERSIST_FLASH_FIRST_BANK_BASE, PERSIST_FIRST_BANK_SIZE, my_unique_record_tags_first_bank, sizeof(my_unique_record_tags_first_bank));
	num_records_second_bank = Persist_flash_unique_tags_get((void *)PERSIST_FLASH_SECOND_BANK_BASE, PERSIST_SECOND_BANK_SIZE, my_unique_record_tags_second_bank, sizeof(my_unique_record_tags_second_bank));

    /* check how many unique tags we have in each case
     * the one with more unique tags becomes primary bank
     */

	if( num_records_first_bank > num_records_second_bank )
	{
		Primary_Bank_Pointer = (uint32_t *) PERSIST_FLASH_FIRST_BANK_BASE;
    	Primary_Bank_Size = PERSIST_FIRST_BANK_SIZE;


		/* erase the non primary bank preemptively */
		trouble = Persist_flash_erase_range( (void *)PERSIST_FLASH_SECOND_BANK_BASE, PERSIST_SECOND_BANK_SIZE );
	}
	else
    if( num_records_first_bank < num_records_second_bank )
    {
		Primary_Bank_Pointer = (uint32_t *) PERSIST_FLASH_SECOND_BANK_BASE;
    	Primary_Bank_Size = PERSIST_SECOND_BANK_SIZE;


		/* erase the non primary bank preemptively */
		trouble = Persist_flash_erase_range( (void *)PERSIST_FLASH_FIRST_BANK_BASE, PERSIST_FIRST_BANK_SIZE );

    }
    else /* equal amount of unique tags */
    {
    	/* both have an equal amount */
    	if( num_records_first_bank == 0 && num_records_second_bank == 0)
    	{
    		/* default */
    		Primary_Bank_Pointer = (uint32_t *) PERSIST_FLASH_FIRST_BANK_BASE;
        	Primary_Bank_Size = PERSIST_FIRST_BANK_SIZE;


    		/* erase all banks preemptively */
    		trouble = Persist_flash_erase_all_banks();
    	}
    	else
    	{
    		/* both have the same number of tags, look at the unique tags in each bank.
    		 * Given the unique tags, verify that both lists are indeed the same,
    		 * if lists aren't the same, somehow tags got corrupted -- erase both
    		 * sectors and set primary to first bank
    		 */
    		number_of_matches = 0;

    		for(unique_records_first_bank_iterator = 0; unique_records_first_bank_iterator < num_records_first_bank; unique_records_first_bank_iterator++)
    		{
        		for(unique_records_second_bank_iterator = 0; unique_records_second_bank_iterator < num_records_first_bank; unique_records_second_bank_iterator++)
        		{
                    if( my_unique_record_tags_second_bank[unique_records_second_bank_iterator] == my_unique_record_tags_first_bank[unique_records_first_bank_iterator])
                    {
                    	number_of_matches = number_of_matches + 1;
                    }
        		}
    		}

    		if(number_of_matches != num_records_first_bank)
    		{
        		/* default */
        		Primary_Bank_Pointer = (uint32_t *) PERSIST_FLASH_FIRST_BANK_BASE;
            	Primary_Bank_Size = PERSIST_FIRST_BANK_SIZE;

        		/* erase all banks preemptively */
        		trouble = Persist_flash_erase_all_banks();
    		}
    		else
    		{
        		/* go through a single list and get most recent record out of both banks,
        		 * for each unique tag, if any bank has a recent record that is more out of date
        		 * the other is assumed to be the primary
        		 */

    			most_recent_tag_count_bank1 = 0;
    			most_recent_tag_count_bank2 = 0;

        		for(unique_records_first_bank_iterator = 0; unique_records_first_bank_iterator < num_records_first_bank; unique_records_first_bank_iterator++)
        		{
            		/* note, we use the first bank list because they are proven to be equal */

        			/* look at the first bank */
            		Primary_Bank_Pointer = (uint32_t *) PERSIST_FLASH_FIRST_BANK_BASE;
                	Primary_Bank_Size = PERSIST_FIRST_BANK_SIZE;

        			record_ptr_bank_1 = Persist_Flash_Get_Recent_Record(my_unique_record_tags_first_bank[unique_records_first_bank_iterator], NULL);

        			/* look at the second bank */
        			Primary_Bank_Pointer = (uint32_t *) PERSIST_FLASH_SECOND_BANK_BASE;
                	Primary_Bank_Size = PERSIST_SECOND_BANK_SIZE;

        			record_ptr_bank_2 = Persist_Flash_Get_Recent_Record(my_unique_record_tags_first_bank[unique_records_first_bank_iterator], NULL);

        			/* if either pointer is null, then this is an error since we found the tag before */
            		if( record_ptr_bank_1 == NULL || record_ptr_bank_2 == NULL)
            		{
            			/* default */
                		Primary_Bank_Pointer = (uint32_t *) PERSIST_FLASH_FIRST_BANK_BASE;
                    	Primary_Bank_Size = PERSIST_FIRST_BANK_SIZE;

                		/* erase all banks preemptively */
                		trouble = Persist_flash_erase_all_banks();
            		}
            		else
            		{
            			/* for each 'most recent' pair, check which bank's record is more recent */
            			ptr_1_timestamp = record_ptr_bank_1->persist_record_header.timestamp;
            			ptr_2_timestamp = record_ptr_bank_2->persist_record_header.timestamp;

            			/*TODO: this should really be calling a compare timestamp function from
            			 * apextimestamp.c which still needs to be made
            			 */

            			if(ptr_1_timestamp > ptr_2_timestamp)
            			{
            				most_recent_tag_count_bank1++;
            			}
            			else
            			if(ptr_2_timestamp > ptr_1_timestamp)
            			{
            				most_recent_tag_count_bank2++;
            			}
            		}
        		}

        		/* compare which bank has the majority of recent tags */
            	if(most_recent_tag_count_bank1 > most_recent_tag_count_bank2)
            	{
            		/* we assume bank 1 is the primary then */
                	Primary_Bank_Pointer = (uint32_t *) PERSIST_FLASH_FIRST_BANK_BASE;
                	Primary_Bank_Size = PERSIST_FIRST_BANK_SIZE;


            		/* erase the non primary bank preemptively */
            		trouble = Persist_flash_erase_range( (void *)PERSIST_FLASH_SECOND_BANK_BASE, PERSIST_SECOND_BANK_SIZE );
            	}
            	else
            	if(most_recent_tag_count_bank2 > most_recent_tag_count_bank1)
            	{
            		/* we assume bank 2 is the primary then */
                	Primary_Bank_Pointer = (uint32_t *) PERSIST_FLASH_SECOND_BANK_BASE;
                	Primary_Bank_Size = PERSIST_SECOND_BANK_SIZE;


            		/* erase the non primary bank preemptively */
            		trouble = Persist_flash_erase_range( (void *)PERSIST_FLASH_FIRST_BANK_BASE, PERSIST_FIRST_BANK_SIZE );
            	}
            	else
            	{
            		/* both are equal in recent-ness */

                	/* if both have most up to date records, get total number of records in each bank
                	 * we'll assume that the new primary bank was intended to be the one with the leser
                	 * amount of records
                	*/

        			/* look at the first bank */
            		Primary_Bank_Pointer = (uint32_t *) PERSIST_FLASH_FIRST_BANK_BASE;
                	Primary_Bank_Size = PERSIST_FIRST_BANK_SIZE;


            		total_valid_bank1 = Persist_flash_get_valid_record_count(NULL);

        			/* look at the second bank */
        			Primary_Bank_Pointer = (uint32_t *) PERSIST_FLASH_SECOND_BANK_BASE;
                	Primary_Bank_Size = PERSIST_SECOND_BANK_SIZE;


        			total_valid_bank2 = Persist_flash_get_valid_record_count(NULL);

        			if( total_valid_bank1 > total_valid_bank2 )
        			{
                		Primary_Bank_Pointer = (uint32_t *) PERSIST_FLASH_FIRST_BANK_BASE;
                    	Primary_Bank_Size = PERSIST_FIRST_BANK_SIZE;


                		/* erase the non primary bank preemptively */
                		trouble = Persist_flash_erase_range( (void *)PERSIST_FLASH_SECOND_BANK_BASE, PERSIST_SECOND_BANK_SIZE );
        			}
        			else
        			if( total_valid_bank2 > total_valid_bank1 )
        			{
                		Primary_Bank_Pointer = (uint32_t *) PERSIST_FLASH_SECOND_BANK_BASE;
                    	Primary_Bank_Size = PERSIST_SECOND_BANK_SIZE;


                		/* erase the non primary bank preemptively */
                		trouble = Persist_flash_erase_range( (void *)PERSIST_FLASH_FIRST_BANK_BASE, PERSIST_FIRST_BANK_SIZE );

        			}
        			else
        			{
                		/* default */
                		Primary_Bank_Pointer = (uint32_t *) PERSIST_FLASH_FIRST_BANK_BASE;                    	Primary_Bank_Size = PERSIST_FIRST_BANK_SIZE;
                    	Primary_Bank_Size = PERSIST_FIRST_BANK_SIZE;

                		/* erase the non primary bank preemptively */
                		trouble = Persist_flash_erase_range( (void *)PERSIST_FLASH_SECOND_BANK_BASE, PERSIST_SECOND_BANK_SIZE );
        			}
            	}
    		}
    	}
    }

	return trouble;
}

/*FUNCTION**********************************************************************
 * Function Name : Persist_flash_unique_tags_get
 * Description   : Counts the number of unique records (no repeats), and returns
 * a list of the tags if user provides a large enough buffer.
 *
 * Parameters:
 * -void * base: the scan start point
 * -uint32_t bounds: the length we will scan through from base to (base+bounds)
 * -uint32_t * tag_buffer: user passed in buffer for found unique tags
 * -uint32_t tag_buffer_size: length of the user passed in buffer
 *
 * Return value:
 * -<0 if base or (base+bounds) violates flash memory range possible
 * -# of unique tags found
 * Return value (pass by reference):
 * -Buffer filled with up to 'tag_buffer_size' amount of unique tags
 *END**************************************************************************/
#if (defined(FLASH_DEBUG_MODE) && !FLASH_DEBUG_MODE)
static int32_t Persist_flash_unique_tags_get(void * base, uint32_t bounds, uint32_t * tag_buffer, uint32_t tag_buffer_size )
#else
int32_t Persist_flash_unique_tags_get(void * base, uint32_t bounds, uint32_t * tag_buffer, uint32_t tag_buffer_size )
#endif
{
	uint32_t unique_record_tags[MAX_PERSIST_RECORDS_ALLOWED];
	uint32_t num_unique_records_found;
	uint32_t unique_records_iterator;
	uint32_t unique_tag_already_found;

	void * flash_iterator;
	void * base_bounds;
	persistRecord_t * persist_ptr;

	uint32_t flash_iterator_offset;

	uint32_t data_length_found;
	uint32_t data_length_padding_found;
	uint32_t found_record_size;

	if( Persist_flash_range_check( base, bounds ) < 0 )
	{
		return -1;
	}

	num_unique_records_found = 0;
	flash_iterator_offset = 0;

	base_bounds = (void *)((uint32_t)base + bounds);
	flash_iterator = (void *)((char *)base + flash_iterator_offset);

	while( (uint32_t)flash_iterator < (uint32_t)base_bounds)
	{
		if(*((uint8_t *)flash_iterator) != 0xFF)
		{
			persist_ptr = (persistRecord_t *)flash_iterator;

			if(persist_ptr->persist_record_header.struct_id == RECORD_STRUCT_ID)
			{
				if(Persist_flash_record_valid((void *)persist_ptr))
				{
					data_length_found = PERSIST_RECORD_DATA_LENGTH_GET(persist_ptr->persist_record_header.record_id);
					data_length_padding_found = PERSIST_RECORD_DATA_LENGTH_PADDING(data_length_found);

					/* get all of the size information and index to a point after the record */
					found_record_size =   sizeof(persist_ptr->persist_record_header) + \
										  data_length_found +\
									      data_length_padding_found +\
										  sizeof(persist_ptr->checksum);

					/* offset the offset to point to after the found record*/
					flash_iterator_offset = found_record_size;

					unique_tag_already_found = 0;
					/* look at the tag and see if we have collected it yet */
					for(unique_records_iterator = 0; unique_records_iterator < num_unique_records_found; unique_records_iterator++)
					{
		                if(unique_record_tags[unique_records_iterator] == persist_ptr->persist_record_header.record_tag)
		                {
		                    unique_tag_already_found = 1;
		                }
					}

					/* if the tag wasn't found in our list, add it */
					if( !unique_tag_already_found)
					{
						unique_record_tags[num_unique_records_found] = persist_ptr->persist_record_header.record_tag;
						num_unique_records_found++;
					}
				}
				else
				{
					/* look at the next byte */
					flash_iterator_offset++;
				}

			}
			else
			{
				flash_iterator_offset++;
			}
		}
		else
		{
	        flash_iterator_offset++;
		}

		flash_iterator = (void *)((char *)flash_iterator + flash_iterator_offset);
	}

    /* if user passed in a buffer, return the unique tags */
    if(tag_buffer != NULL && tag_buffer_size > 0)
    {
        for(unique_records_iterator = 0; unique_records_iterator < (tag_buffer_size/4); unique_records_iterator++)
        {
        	*(tag_buffer + unique_records_iterator) = unique_record_tags[unique_records_iterator];
        }
    }

    return num_unique_records_found;
}

/*FUNCTION**********************************************************************
 * Function Name : Persist_flash_threshold_check
 * Description   : Checks how many bytes are currently written and flash in
 * the range provided. Based on written_bytes/(bounds), if that ratio is greater
 * than the provided fill_threshold_percent provided, function will return 1.
 * Otherwise, if we do not exceed the threshold, return 0.
 *
 * Parameters:
 * -void * base: the scan start point
 * -uint32_t bounds: the length we will scan through from base to (base+bounds)
 * -uint32_t fill_threshold_percent: range 0-100, threshold for used memory
 *
 * Return value:
 * -1 if threshold exceeded
 * -0 if threshold not exceeded
 * -<0 if base or (base+bounds) violates acceptable flash range
 *END**************************************************************************/
#if (defined(FLASH_DEBUG_MODE) && !FLASH_DEBUG_MODE)
static int32_t Persist_flash_threshold_check(void * base, uint32_t bounds, uint32_t fill_threshold_percent )
#else
int32_t Persist_flash_threshold_check(void * base, uint32_t bounds, uint32_t fill_threshold_percent )
#endif
{
    float calculated_threshold_fl;
    uint32_t calculated_threshold_uint;

    void * flash_iterator;
    uint32_t flash_offset;
    void * base_bounds;

    uint32_t bytes_allocated = 0;

    if(Persist_flash_range_check(base, bounds) < 0)
    {
    	return -1;
    }

    if(fill_threshold_percent > 100)
    {
    	return -1;
    }

    flash_offset = 0;

    base_bounds = (void *)((char *)base + bounds);
    flash_iterator = (void *)((char *)base + flash_offset);

    while((uint32_t)flash_iterator < (uint32_t)base_bounds)
    {
        if(*((uint8_t *)flash_iterator) == 0xFF)
        {
        	bytes_allocated = bytes_allocated + 1;
        }

        /* check next position */
        flash_iterator = (void *)((char *)flash_iterator + 1);
    }

    /* we now have the number of allocated bytes
     * so calculate how full we are in range
     */

    calculated_threshold_fl = bytes_allocated / bounds;
    calculated_threshold_uint = floor(calculated_threshold_fl * 100);

    /* check if threshold is exceeded */
    if( calculated_threshold_uint < fill_threshold_percent)
    {
    	return 0;
    }

    return 1;
}

/*FUNCTION**********************************************************************
 * Function Name : Persist_flash_range_check
 * Description   : Given a base and bounds, check if we are within flash memory.
 *
 * Parameters:
 * -void * base: the scan start point
 * -uint32_t bounds: the length we will scan through from base to (base+bounds)
 *
 * Return value:
 * -0 if threshold not exceeded
 * -<0 if base or (base+bounds) violates acceptable flash range
 *END**************************************************************************/
#if (defined(FLASH_DEBUG_MODE) && !FLASH_DEBUG_MODE)
int32_t Persist_flash_range_check(void * base, uint32_t bounds )
#else
int32_t Persist_flash_range_check(void * base, uint32_t bounds )
#endif
{
	/* if null, or base is not 8 byte aligned, or bounds isn't 8 byte aligned */
	if(base == NULL || ((uint32_t)base & 0x7) != 0 || (bounds & 0x7) != 0)
	{
		return -1;
	}

	/* remember, base is included in the range so its -1 */
	if(((uint32_t)base > K64_FLASH_END_ADDR) || ((((uint32_t)base) + (bounds - 1)) > K64_FLASH_END_ADDR))
	{
		return -1;
	}

	return 0;

}

#if 0
/*FUNCTION**********************************************************************
 * Function Name : Persist_flash_garbage_collect
 * Description   :
 *
 * Parameters:
 * -
 *
 * Return value:
 * -
 *END**************************************************************************/
#if (defined(FLASH_DEBUG_MODE) && !FLASH_DEBUG_MODE)
static Persist_flash_garbage_collect( void )
#else
int32_t Persist_flash_garbage_collect( void )
#endif
{

}

/*FUNCTION**********************************************************************
 * Function Name : Persist_flash_garbage_collect
 * Description   :
 *
 * Parameters:
 * -
 *
 * Return value:
 * -
 *END**************************************************************************/
#if (defined(FLASH_DEBUG_MODE) && !FLASH_DEBUG_MODE)
static Persist_flash_garbage_collect( void )
#else
int32_t Persist_flash_garbage_collect_rtos_idle( void )
#endif
{

}

/*FUNCTION**********************************************************************
 * Function Name : Persist_flash_garbage_collect
 * Description   :
 *
 * Parameters:
 * -
 *
 * Return value:
 * -
 *END**************************************************************************/
#if (defined(FLASH_DEBUG_MODE) && !FLASH_DEBUG_MODE)
static Persist_flash_garbage_collect( void )
#else
int32_t Persist_flash_garbage_collect_erase_size( void )
#endif
{

}
#endif
/*FUNCTION**********************************************************************
 * Function Name : Persist_flash_garbage_collect
 * Description   :
 *
 * Parameters:
 * -
 *
 * Return value:
 * -
 *END**************************************************************************/
#if (defined(FLASH_DEBUG_MODE) && !FLASH_DEBUG_MODE)
static Persist_flash_garbage_collect( void )
#else
int32_t Persist_flash_garbage_collect( void )
#endif
{
	persistInitHeader_t my_init_header;
	void * new_primary_bank_ptr = NULL;
	uint64_t checksum = 0;
	uint32_t expected_header_size = 0;

	static uint32_t my_unique_record_tags_primary_bank[MAX_PERSIST_RECORDS_ALLOWED] = { 0 };
	static persistRecord_t * my_unique_record_ptrs[MAX_PERSIST_RECORDS_ALLOWED] = { 0 };
	int32_t num_records_primary_bank = 0;
	int32_t record_ptr_iterator = 0;
	persistRecord_t * temp_record_ptr;
	persistRecord_t my_ram_copy_record;

	int32_t trouble = 0;

	//check which bank we are currently in, record which bank we will be changing to
	if(((uint32_t)Primary_Bank_Pointer >= PERSIST_FLASH_FIRST_BANK_BASE) && ((uint32_t)Primary_Bank_Pointer < PERSIST_FLASH_SECOND_BANK_BASE))
	{
		new_primary_bank_ptr = (void *)PERSIST_FLASH_SECOND_BANK_BASE;
	}
	else
	if(((uint32_t)Primary_Bank_Pointer >= PERSIST_FLASH_SECOND_BANK_BASE) && ((uint32_t)Primary_Bank_Pointer < PERSIST_FLASH_END_OF_PERSISTENCE))
	{
		new_primary_bank_ptr = (void *)PERSIST_FLASH_FIRST_BANK_BASE;
	}

	//write new bank and valid = 0 to the house-keeping header (involves erase and write procedure -- can be optimized for ware-leveling)
	my_init_header.primary_bank_ptr = (uint32_t)new_primary_bank_ptr;
	my_init_header.last_operation_valid = 0;

	expected_header_size = sizeof(my_init_header.last_operation_valid) + sizeof(my_init_header.last_operation_valid);
	checksum = fletcher_checksum(expected_header_size, (void *)&my_init_header);

	my_init_header.checksum = checksum;

	//erase header
	trouble = uFlash_erase_and_verify( &s_flashDriver, (uint32_t)PERSIST_REGION_BASE, (uint32_t)sizeof(persistInitHeader_t));

	if(trouble < 0)
	{
		trouble = -1;
    	goto GARBAGE_COLLECTION_ERROR;
	}

	/***********PROGRAM HEADER INTO FLASH***********/
    trouble = FLASH_Program(&s_flashDriver, (uint32_t)PERSIST_REGION_BASE, (uint32_t *)(&(my_init_header)), sizeof(persistInitHeader_t));
    if (kStatus_FLASH_Success != trouble)
    {
    	/* return persist flash record program failure */
    	trouble = -EC_PERSIST_WRITE_RECORD_FAIL;
    	goto GARBAGE_COLLECTION_ERROR;
    }

    /* Verify programming by Program Check command with user margin levels */
    trouble = FLASH_VerifyProgram(	&s_flashDriver,
    								(uint32_t)PERSIST_REGION_BASE,
    								sizeof(persistInitHeader_t),
									(const uint32_t *)(&(my_init_header)), //MI: was (uint8_t *)&s_persistRecord,
									kFLASH_MarginValueUser,
									&failAddr, &failDat
								);
    if (kStatus_FLASH_Success != trouble)
    {
    	/* program verify failure */
    	trouble = -EC_PERSIST_WRITE_VERIFY;
    	goto GARBAGE_COLLECTION_ERROR;
    }
	/***********************************************/

	//go through and get all possible unique pointers of most recent versions of the records (todo: what if theres no history and we are simply out of space)

    //todo: we shouldn't use macros for size, the size should be known dynamically -- add this to the header info -- they're both same right now so its fine
	num_records_primary_bank = Persist_flash_unique_tags_get((void *)Primary_Bank_Pointer, PERSIST_FIRST_BANK_SIZE, my_unique_record_tags_primary_bank, sizeof(my_unique_record_tags_primary_bank));

	if(num_records_primary_bank < 0)
	{
		trouble = -1;
    	goto GARBAGE_COLLECTION_ERROR;
	}

	//go through each tag and get the unique record and store the pointer
	for(record_ptr_iterator = 0; record_ptr_iterator < num_records_primary_bank; record_ptr_iterator++)
	{
		temp_record_ptr = Persist_Flash_Get_Recent_Record(my_unique_record_tags_primary_bank[record_ptr_iterator], NULL); //todo: do we need to check if the return is good if the tag was found?


		if(temp_record_ptr == NULL)
		{
			//we found a tag, but no record?
			trouble = -1;
			goto GARBAGE_COLLECTION_ERROR;
		}

		my_unique_record_ptrs[record_ptr_iterator] = temp_record_ptr; //store the address
	}

	//set new primary
	Primary_Bank_Pointer = new_primary_bank_ptr;

	//start procedure: copy record from flash -> to ram; write the record
	for(record_ptr_iterator = 0; record_ptr_iterator < num_records_primary_bank; record_ptr_iterator++)
	{
		trouble = Persist_Flash_Record_Copy_To_RAM(&my_ram_copy_record, my_unique_record_ptrs[record_ptr_iterator]);

		if(trouble < 0)
		{
			trouble = -1;
	    	goto GARBAGE_COLLECTION_ERROR;
		}

		trouble = Persist_Flash_Record_Commit(&my_ram_copy_record);

		if(trouble < 0)
		{
			trouble = -1;
	    	goto GARBAGE_COLLECTION_ERROR;
		}
	}

	//completed writing all records to new bank, now we must update the housekeeping/initialization header
	//on completion write new valid = 1 to house-keeping header (involves erase and write procedure -- can be optimized for ware-leveling)

	//write new bank and valid = 0 to the house-keeping header (involves erase and write procedure -- can be optimized for ware-leveling)
	my_init_header.last_operation_valid = 1;

	expected_header_size = sizeof(my_init_header.last_operation_valid) + sizeof(my_init_header.last_operation_valid);
	checksum = fletcher_checksum(expected_header_size, (void *)&my_init_header);

	my_init_header.checksum = checksum;

	//erase header
	trouble = uFlash_erase_and_verify( &s_flashDriver, (uint32_t)PERSIST_REGION_BASE, (uint32_t)sizeof(persistInitHeader_t));

	if(trouble < 0)
	{
		trouble = -1;
    	goto GARBAGE_COLLECTION_ERROR;
	}

	/***********PROGRAM HEADER INTO FLASH***********/
    trouble = FLASH_Program(&s_flashDriver, (uint32_t)PERSIST_REGION_BASE, (uint32_t *)(&(my_init_header)), sizeof(persistInitHeader_t));
    if (kStatus_FLASH_Success != trouble)
    {
    	/* return persist flash record program failure */
    	trouble = -EC_PERSIST_WRITE_RECORD_FAIL;
    	goto GARBAGE_COLLECTION_ERROR;
    }

    /* Verify programming by Program Check command with user margin levels */
    trouble = FLASH_VerifyProgram(	&s_flashDriver,
    								(uint32_t)PERSIST_REGION_BASE,
    								sizeof(persistInitHeader_t),
									(const uint32_t *)(&(my_init_header)), //MI: was (uint8_t *)&s_persistRecord,
									kFLASH_MarginValueUser,
									&failAddr, &failDat
								);
    if (kStatus_FLASH_Success != trouble)
    {
    	/* program verify failure */
    	trouble = -EC_PERSIST_WRITE_VERIFY;
    	goto GARBAGE_COLLECTION_ERROR;
    }
	/***********************************************/

    //todo: we really shouldn't do any erasing here, but we need to fix the recursiveness to provision what to do
    //when both banks are full. Need recursive function to be aware of both bank full trigger
    /******ERASE THE NON PRIMARY BANK NOW******/
    trouble = Persist_flash_region_initialize(&Primary_Bank_Pointer); //though its an initialization function, it basically does what we need here temporarily

GARBAGE_COLLECTION_ERROR:
	return trouble;
}

/*FUNCTION**********************************************************************
 * Function Name : Persist_flash_current_bank_records_get
 * Description   : Scrolls through the current persistent bank region and collects
 * all valid record addresses in the order that they are found (which is the order
 * they were initially stored in -- unless garbage collection occurred in which
 * the ordering is not guaranteed)
 *
 * Parameters:
 * -persistRecord_t ** buffer: pass by reference buffer to store record addresses
 * -uint32_t buffer_size: size of passed in buffer
 *
 * Return value:
 * -<0 if error
 * -else returns number of records in bank
 *END**************************************************************************/
int32_t Persist_flash_current_bank_records_get( persistRecord_t ** buffer, uint32_t buffer_size )
{
    /* make an empty list for max possible records of a single type */
    persistRecord_t * type_record_list[MAX_PERSIST_RECORDS_ALLOWED];
    void * end_of_bank = NULL;
    void * base;

    uint32_t number_of_records_found;
    uint32_t record_itr;

    void * record_iterator;

    /* initialize things */
    base = (void *)Primary_Bank_Pointer;
    record_iterator = (void *)base;
    number_of_records_found = 0;

    if( buffer == NULL)
    {
        return -1;
    }

    /* determine where the end of bank is */
    if(((uint32_t)Primary_Bank_Pointer >= PERSIST_FLASH_FIRST_BANK_BASE) && ((uint32_t)Primary_Bank_Pointer < PERSIST_FLASH_SECOND_BANK_BASE))
    {
        end_of_bank = (void *)((char *)base + PERSIST_FIRST_BANK_SIZE);
    }
    else
    if(((uint32_t)Primary_Bank_Pointer >= PERSIST_FLASH_SECOND_BANK_BASE) && ((uint32_t)Primary_Bank_Pointer < PERSIST_FLASH_END_OF_PERSISTENCE))
    {
        end_of_bank = (void *)((char *)base + PERSIST_SECOND_BANK_SIZE);
    }

    /* iterate through each record*/
    while((uint32_t)((char *)record_iterator + 1) < (uint32_t)end_of_bank)
    {
        if(Persist_flash_record_exists((persistRecord_t *)record_iterator))
        {
            /* see if record is valid */
            if(Persist_flash_record_valid(record_iterator))
            {
                    /* we found a record of a certain type, add it to the list */
                    type_record_list[number_of_records_found] = (persistRecord_t *)record_iterator;
                    number_of_records_found++;
            }

        }

        /* iterate to the next record as long as your not cross boundaries */
        record_iterator = (void *)((char *)record_iterator + 1);
    }

    /* we now have all the records stored - copy them back to the user */
    if( buffer_size < number_of_records_found)
    {
        for(record_itr = 0; record_itr < buffer_size; record_itr++)
        {
            buffer[record_itr] = type_record_list[record_itr];
        }
    }
    else
    {
        for(record_itr = 0; record_itr < number_of_records_found; record_itr++)
        {
            buffer[record_itr] = type_record_list[record_itr];
        }
    }

    /* return the record of specific type */
    return number_of_records_found;
}

/*FUNCTION**********************************************************************
 * Function Name : Persist_flash_primary_sector_clean_and_shift
 * Description   : Changes primary sector to the redundant sector. Copies 'history'
 * amount of each unique record to the new primary sector from the old primary
 * sector, then performs an erase on the old primary sector memory. If force_clean
 * is active, regardless of thresholds, the primary sector gets copied and cleaned.
 * Otherwise, copy and clean is only performed if the current primary bank is full
 * past the allowed threshold value.
 *
 * Parameters:
 * -void * base: the scan start point
 * -uint32_t bounds: the length we will scan through from base to (base+bounds)
 * -uint32_t force_clean: if 1, primary sector is copied and cleaned, otherwise
 *                        copy/clean happens only if threshold violation occurs
 * -uint32_t threshold: the threshold value determining how full a bank can get
 *                      before we must clean
 * -uint32_t history: the history level amount of each unique tag/record we copy
 *
 * Return value:
 * --4 if there are too many records and user must delete them manually
 * --3 if corruption caused primary bank to be erased: clean unsuccessful
 * --2 if corruption caused primary bank to be erased: clean unsuccessful
 * --1 if error
 * -0 no action needed
 * -1 if copy/clean performed successfully
 * -2 if no error, but no copy/clean functions performed (threshold not violated)
 *END**************************************************************************/
#if (defined(FLASH_DEBUG_MODE) && !FLASH_DEBUG_MODE)
static Persist_flash_range_check(void * base, uint32_t bounds )
#else
int32_t Persist_flash_primary_sector_clean_and_shift(void * base, uint32_t bounds, uint32_t force_clean, uint32_t threshold, uint32_t history )
#endif
{

    /* states simply added to make debug more clear */
	enum
	{
		__TABULATE_FORWARD_COPY_LENGTH,
		__REATTEMPT_TABULATE_FORWARD_COPY_LENGTH,
		__COPY_FORWARD_UNMARKED
	};

	uint32_t __CURRENT_OPERATION_STATE;

	int32_t trouble = 0;
	int32_t return_val = 0;



	int32_t threshold_check_exceeded;

	int32_t number_unique_records;
	int32_t number_delete_records;

	int32_t * delete_records;
	uint32_t unique_record_tags[MAX_PERSIST_RECORDS_ALLOWED];
	persistRecord_t * type_record_list[MAX_PERSIST_RECORDS_ALLOWED];

	uint32_t first_or_second_bank; /* flag that tells us which bank is currently primary */

	int32_t unique_record_iterator;
	int32_t delete_record_iterator;
	uint32_t dont_skip;

	uint32_t history_size_iterator;
	uint32_t total_size_to_be_copied;
	int32_t current_record_size;

	int32_t number_of_history_records_returned;
	uint32_t opposite_bank_threshold;



	if(base == NULL || bounds == 0)
	{
		trouble = -1;
		goto PRIMARY_SECTOR_CLEAN_AND_SHIFT_ERROR_HANDLER;
	}

    if(Persist_flash_range_check(base, bounds) < 0)
    {
		trouble = -1;
		goto PRIMARY_SECTOR_CLEAN_AND_SHIFT_ERROR_HANDLER;
    }

    if( history < 1 )
    {
		trouble = -1;
		goto PRIMARY_SECTOR_CLEAN_AND_SHIFT_ERROR_HANDLER;
    }

	/* determine what the current primary is */
	if(((uint32_t)Primary_Bank_Pointer >= PERSIST_FLASH_FIRST_BANK_BASE) && ((uint32_t)Primary_Bank_Pointer < PERSIST_FLASH_SECOND_BANK_BASE))
	{
		first_or_second_bank = 0;
	}
	else
	if(((uint32_t)Primary_Bank_Pointer >= PERSIST_FLASH_SECOND_BANK_BASE) && ((uint32_t)Primary_Bank_Pointer < PERSIST_FLASH_END_OF_PERSISTENCE))
	{
		first_or_second_bank = 1;
	}

	/* if force_clean is on, then regardless of threshold value, we do a clean and primary swap operation */
	if(force_clean)
	{
		goto FORCE_CLEAN_UNCONDITIONAL;
	}
	else
	{
		/* default operation */
        threshold_check_exceeded =  Persist_flash_threshold_check(base, bounds, PERSIST_FLASH_FILL_THRESHOLD);

        if( threshold_check_exceeded == 1)
        {
        	/* threshold exceeded, perform a clean */

FORCE_CLEAN_UNCONDITIONAL:
        	/* get list of all unique records in current primary bank */
        	number_unique_records = Persist_flash_unique_tags_get(base, bounds, unique_record_tags, sizeof(unique_record_tags));

        	if(number_unique_records <= 0)
        	{
        		/* threshold exceeded meaning bytes are used up in the bank
        		 * but here no unique records are found. Bank needs to be erased.
        		 */
                return_val =  Persist_flash_erase_range( base, bounds );
                if(return_val == 0)
                {
                	trouble = -2;
                }
                else
                {
                	trouble = -3;
                }
                goto PRIMARY_SECTOR_CLEAN_AND_SHIFT_ERROR_HANDLER;
        	}
        	else
        	{
        		/* go through each record of the unique tags we found
        		 * if any of the unique tags match a tag in the delete bank
        		 * then skip that tag and unmark it from the delete sector
        		 *
        		 * otherwise, get the history for that tag and figure out
        		 * what the total number of bytes is for history levels of
        		 * records. Do this for all valid non deleted records to get
        		 * total number of bytes.
        		 *
        		 * check if this amount of bytes will violate the threshold if
        		 * we were to copy it over to the non primary bank. If yet, try
        		 * procedure again with history levels - 1. Do this until history
        		 * level is 1. If still violating threshold, that means user has too
        		 * many large records and must delete one manually. Return in this case.
        		 *
        		 * otherwise, again go through each tag that wasn't deleted, and for each
        		 * history set, copy a 'history' amount of records to the new primary bank.
        		 *
        		 * once done, set primary to the opposite bank and end procedure
        		 *
        		 */
        		/* go through the delete bank to see which records to skip */

        		/* get number of delete records */
        		__CURRENT_OPERATION_STATE = __TABULATE_FORWARD_COPY_LENGTH;
RECORD_FORWARD_COPY_OPERATION:

        		number_delete_records = Persist_flash_get_records_marked_for_deletion(&(delete_records));


                total_size_to_be_copied = 0;

				if(__CURRENT_OPERATION_STATE == __COPY_FORWARD_UNMARKED)
				{
					/* erase the bank that is opposite of current primary */
					if(first_or_second_bank == 0)
					{
	                    if( Persist_flash_erase_range( (void *)PERSIST_FLASH_SECOND_BANK_BASE, PERSIST_SECOND_BANK_SIZE ) < 0 )
	                    {
    						trouble = -1;
    						goto PRIMARY_SECTOR_CLEAN_AND_SHIFT_ERROR_HANDLER;
	                    }

	                	//Primary_Bank_Pointer = (uint32_t *) PERSIST_FLASH_SECOND_BANK_BASE;
	                	//Primary_Bank_Size = PERSIST_SECOND_BANK_SIZE;
					}
					else
					if(first_or_second_bank == 1)
					{
	                    if( Persist_flash_erase_range( (void *)PERSIST_FLASH_FIRST_BANK_BASE, PERSIST_FIRST_BANK_SIZE ) < 0 )
	                    {
    						trouble = -1;
    						goto PRIMARY_SECTOR_CLEAN_AND_SHIFT_ERROR_HANDLER;
	                    }

	                	//Primary_Bank_Pointer = (uint32_t *) PERSIST_FLASH_FIRST_BANK_BASE;
	                	//Primary_Bank_Size = PERSIST_FIRST_BANK_SIZE;
					}
				}

        		for(unique_record_iterator = 0; unique_record_iterator < number_unique_records; unique_record_iterator++)
        		{
        			/* for this unique record, does it exist in the delete list? */

            		dont_skip = 1;
        			for(delete_record_iterator = 0; delete_record_iterator < number_delete_records; delete_record_iterator++)
        			{
        				if((int32_t)unique_record_tags[unique_record_iterator] == delete_records[delete_record_iterator])
        				{
        					dont_skip = 0;
        				}
        			}

        			if(dont_skip)
        			{
        				/* add the record length */
        				if(__CURRENT_OPERATION_STATE == __TABULATE_FORWARD_COPY_LENGTH)
        				{
        					number_of_history_records_returned = Persist_flash_get_record_history(unique_record_tags[unique_record_iterator], MAX_PERSIST_RECORDS_ALLOWED, type_record_list);

        					/* get the record history */
        					if( number_of_history_records_returned < 0)
        					{
        						trouble = -1;
        						goto PRIMARY_SECTOR_CLEAN_AND_SHIFT_ERROR_HANDLER;
        					}

        					for(history_size_iterator = 0; history_size_iterator < (uint32_t)number_of_history_records_returned; history_size_iterator++ )
        					{
        						if(history_size_iterator == history)
        						{
        							break;
        						}
        						else
        						{
                					/* loop the history amount to get (history * sizeof(record)) amount */
                					current_record_size = Persist_get_record_size_internal(type_record_list[history_size_iterator]);

                					if(current_record_size < 0)
                					{
                						trouble = -1;
                						goto PRIMARY_SECTOR_CLEAN_AND_SHIFT_ERROR_HANDLER;
                					}

                					total_size_to_be_copied += current_record_size;
        						}
        					}
        				}
        				else /* this state is really just here to make it clear in debug */
        				if(__CURRENT_OPERATION_STATE == __REATTEMPT_TABULATE_FORWARD_COPY_LENGTH)
        				{
        					number_of_history_records_returned = Persist_flash_get_record_history(unique_record_tags[unique_record_iterator], MAX_PERSIST_RECORDS_ALLOWED, type_record_list);

        					/* get the record history */
        					if( number_of_history_records_returned < 0)
        					{
        						trouble = -1;
        						goto PRIMARY_SECTOR_CLEAN_AND_SHIFT_ERROR_HANDLER;
        					}

        					for(history_size_iterator = 0; history_size_iterator < (uint32_t)number_of_history_records_returned; history_size_iterator++ )
        					{
        						if(history_size_iterator == history)
        						{
        							break;
        						}
        						else
        						{
                					/* loop the history amount to get (history * sizeof(record)) amount */
                					current_record_size = Persist_get_record_size_internal(type_record_list[history_size_iterator]);

                					if(current_record_size < 0)
                					{
                						trouble = -1;
                						goto PRIMARY_SECTOR_CLEAN_AND_SHIFT_ERROR_HANDLER;
                					}

                					total_size_to_be_copied += current_record_size;
        						}
        					}
        				}
        				else
        				if(__CURRENT_OPERATION_STATE == __COPY_FORWARD_UNMARKED)
        				{
        					/* new primary should be cleaned and set and ready to go */

        					/* get the history of records of prior primary */
        					number_of_history_records_returned = Persist_flash_get_record_history(unique_record_tags[unique_record_iterator], MAX_PERSIST_RECORDS_ALLOWED, type_record_list);

        					/* get the record history */
        					if( number_of_history_records_returned < 0)
        					{
        						trouble = -1;
        						goto PRIMARY_SECTOR_CLEAN_AND_SHIFT_ERROR_HANDLER;
        					}

        					for(history_size_iterator = 0; history_size_iterator < (uint32_t)number_of_history_records_returned; history_size_iterator++ )
        					{
        						if(history_size_iterator == history)
        						{
        							break;
        						}
        						else
        						{

        							if( history <= (uint32_t)number_of_history_records_returned)
        							{
        								/* temporarily set the new primary bank for commitment */
        			                    if(first_or_second_bank == 0)
        			                    {
        			                    	/* current bank is first bank */
            			                	Primary_Bank_Pointer = (uint32_t *) PERSIST_FLASH_SECOND_BANK_BASE;
            			                	Primary_Bank_Size = PERSIST_SECOND_BANK_SIZE;
        			                    }
        			                    else
        			                    if(first_or_second_bank == 1)
        			                    {
        			                    	/* current bank is second bank */
            			                	Primary_Bank_Pointer = (uint32_t *) PERSIST_FLASH_FIRST_BANK_BASE;
            			                	Primary_Bank_Size = PERSIST_FIRST_BANK_SIZE;
        			                    }

            							/* history is returned with the most recent to least recent */
            							/* we need to store least recent to most recent */
            							if( Persist_Flash_Record_Commit(type_record_list[(history - history_size_iterator - 1 )]) < 0)
            							{
                    						trouble = -1;
                    						goto PRIMARY_SECTOR_CLEAN_AND_SHIFT_ERROR_HANDLER;
            							}

            							/* set it back to the prior primary to keep getting records */
        			                    if(first_or_second_bank == 0)
        			                    {
        			                    	/* restore first bank */
            			                	Primary_Bank_Pointer = (uint32_t *) PERSIST_FLASH_FIRST_BANK_BASE;
            			                	Primary_Bank_Size = PERSIST_FIRST_BANK_SIZE;
        			                    }
        			                    else
        			                    if(first_or_second_bank == 1)
        			                    {
        			                    	/* restore second bank */
            			                	Primary_Bank_Pointer = (uint32_t *) PERSIST_FLASH_SECOND_BANK_BASE;
            			                	Primary_Bank_Size = PERSIST_SECOND_BANK_SIZE;
        			                    }
        							}
        							else
        							{
        								/* num records return is smaller than history level request */

        								/* temporarily set the new primary bank for commitment */
        			                    if(first_or_second_bank == 0)
        			                    {
        			                    	/* current bank is first bank */
            			                	Primary_Bank_Pointer = (uint32_t *) PERSIST_FLASH_SECOND_BANK_BASE;
            			                	Primary_Bank_Size = PERSIST_SECOND_BANK_SIZE;
        			                    }
        			                    else
        			                    if(first_or_second_bank == 1)
        			                    {
        			                    	/* current bank is second bank */
            			                	Primary_Bank_Pointer = (uint32_t *) PERSIST_FLASH_FIRST_BANK_BASE;
            			                	Primary_Bank_Size = PERSIST_FIRST_BANK_SIZE;
        			                    }

            							/* history is returned with the most recent to least recent */
            							/* we need to store least recent to most recent */
            							if( Persist_Flash_Record_Commit(type_record_list[(number_of_history_records_returned - history_size_iterator - 1 )]) < 0)
            							{
                    						trouble = -1;
                    						goto PRIMARY_SECTOR_CLEAN_AND_SHIFT_ERROR_HANDLER;
            							}

            							/* set it back to the prior primary to keep getting records */
        			                    if(first_or_second_bank == 0)
        			                    {
        			                    	/* restore first bank */
            			                	Primary_Bank_Pointer = (uint32_t *) PERSIST_FLASH_FIRST_BANK_BASE;
            			                	Primary_Bank_Size = PERSIST_FIRST_BANK_SIZE;
        			                    }
        			                    else
        			                    if(first_or_second_bank == 1)
        			                    {
        			                    	/* restore second bank */
            			                	Primary_Bank_Pointer = (uint32_t *) PERSIST_FLASH_SECOND_BANK_BASE;
            			                	Primary_Bank_Size = PERSIST_SECOND_BANK_SIZE;
        			                    }
        							}

        						}
        					}

							/* set the new primary bank for commitment and erase prior bank*/
		                    if(first_or_second_bank == 0)
		                    {
		                    	/* current bank is first bank */
			                	Primary_Bank_Pointer = (uint32_t *) PERSIST_FLASH_SECOND_BANK_BASE;
			                	Primary_Bank_Size = PERSIST_SECOND_BANK_SIZE;

			                    if( Persist_flash_erase_range( (void *)PERSIST_FLASH_FIRST_BANK_BASE, PERSIST_FIRST_BANK_SIZE ) < 0 )
			                    {
		    						trouble = -1;
		    						goto PRIMARY_SECTOR_CLEAN_AND_SHIFT_ERROR_HANDLER;
			                    }
		                    }
		                    else
		                    if(first_or_second_bank == 1)
		                    {
		                    	/* current bank is second bank */
			                	Primary_Bank_Pointer = (uint32_t *) PERSIST_FLASH_FIRST_BANK_BASE;
			                	Primary_Bank_Size = PERSIST_FIRST_BANK_SIZE;

			                    if( Persist_flash_erase_range( (void *)PERSIST_FLASH_SECOND_BANK_BASE, PERSIST_SECOND_BANK_SIZE ) < 0 )
			                    {
		    						trouble = -1;
		    						goto PRIMARY_SECTOR_CLEAN_AND_SHIFT_ERROR_HANDLER;
			                    }
		                    }

        					trouble = 1;
    						goto PRIMARY_SECTOR_CLEAN_AND_SHIFT_ERROR_HANDLER;
        				}
        			}
        		}

				if(__CURRENT_OPERATION_STATE == __TABULATE_FORWARD_COPY_LENGTH)
				{
					/* we have our total size to be copied, see if that breaks the threshold */
					if(first_or_second_bank == 0 )
					{
						opposite_bank_threshold = floor(((float)((float)threshold / (float)100)) * (float)PERSIST_SECOND_BANK_SIZE);
						/* primary is the first bank */
						if(total_size_to_be_copied >= opposite_bank_threshold )
						{
							/* try again with a smaller history value */
							if(history == 1)
							{
								/* ERROR, YOU MUST DELETE RECORDS */
								/* this means there are enough single recent records that fill up persistence too much */
								trouble = -4;
								goto PRIMARY_SECTOR_CLEAN_AND_SHIFT_ERROR_HANDLER;
							}
							else
							{
								history = history - 1;
								__CURRENT_OPERATION_STATE = __REATTEMPT_TABULATE_FORWARD_COPY_LENGTH;
								goto RECORD_FORWARD_COPY_OPERATION;
							}
						}
						else
						{
							/* no threshold violation, go ahead and begin copying */
							__CURRENT_OPERATION_STATE = __COPY_FORWARD_UNMARKED;
							goto RECORD_FORWARD_COPY_OPERATION;
						}
					}
					else
					if(first_or_second_bank == 1)
					{
						/* primary is the second bank */
						/* primary is the first bank */
						opposite_bank_threshold = floor(((float)(threshold / 100)) * (float)PERSIST_FIRST_BANK_SIZE);
						if(total_size_to_be_copied >= opposite_bank_threshold )
						{
							/* try again with a smaller history value */
							if(history == 1)
							{
								/* ERROR, YOU MUST DELETE RECORDS */
								/* this means there are enough single recent records that fill up persistence too much */
								trouble = -4;
								goto PRIMARY_SECTOR_CLEAN_AND_SHIFT_ERROR_HANDLER;
							}
							else
							{
								history = history - 1;
								__CURRENT_OPERATION_STATE = __REATTEMPT_TABULATE_FORWARD_COPY_LENGTH;
								goto RECORD_FORWARD_COPY_OPERATION;
							}
						}
						else
						{
							/* no threshold violation, go ahead and begin copying */
							__CURRENT_OPERATION_STATE = __COPY_FORWARD_UNMARKED;
							goto RECORD_FORWARD_COPY_OPERATION;
						}
					}
					else
					{
						trouble = -1;
						goto PRIMARY_SECTOR_CLEAN_AND_SHIFT_ERROR_HANDLER;
					}

				}
				else /* this state is really just here to make it clear in debug */
				if(__CURRENT_OPERATION_STATE == __REATTEMPT_TABULATE_FORWARD_COPY_LENGTH)
				{
					/* we have our total size to be copied, see if that breaks the threshold */
					if(first_or_second_bank == 0 )
					{
						/* primary is the first bank */
						opposite_bank_threshold = floor(((float)(threshold / 100)) * (float)PERSIST_SECOND_BANK_SIZE);
						if(total_size_to_be_copied >= opposite_bank_threshold)
						{
							/* try again with a smaller history value */
							if(history == 1)
							{
								/* ERROR, YOU MUST DELETE RECORDS */
								/* this means there are enough single recent records that fill up persistence too much */
								trouble = -4;
								goto PRIMARY_SECTOR_CLEAN_AND_SHIFT_ERROR_HANDLER;
							}
							else
							{
								history = history - 1;
								__CURRENT_OPERATION_STATE = __REATTEMPT_TABULATE_FORWARD_COPY_LENGTH;
								goto RECORD_FORWARD_COPY_OPERATION;
							}
						}
						else
						{
							/* no threshold violation, go ahead and begin copying */
							__CURRENT_OPERATION_STATE = __COPY_FORWARD_UNMARKED;
							goto RECORD_FORWARD_COPY_OPERATION;
						}
					}
					else
					if(first_or_second_bank == 1)
					{
						/* primary is the second bank */
						/* primary is the first bank */
						opposite_bank_threshold = floor(((float)(threshold / 100)) * PERSIST_FIRST_BANK_SIZE);
						if(total_size_to_be_copied > opposite_bank_threshold)
						{
							/* try again with a smaller history value */
							if(history == 1)
							{
								/* ERROR, YOU MUST DELETE RECORDS */
								/* this means there are enough single recent records that fill up persistence too much */
								trouble = -4;
								goto PRIMARY_SECTOR_CLEAN_AND_SHIFT_ERROR_HANDLER;
							}
							else
							{
								history = history - 1;
								__CURRENT_OPERATION_STATE = __REATTEMPT_TABULATE_FORWARD_COPY_LENGTH;
								goto RECORD_FORWARD_COPY_OPERATION;
							}
						}
						else
						{
							/* no threshold violation, go ahead and begin copying */
							__CURRENT_OPERATION_STATE = __COPY_FORWARD_UNMARKED;
							goto RECORD_FORWARD_COPY_OPERATION;
						}
					}
					else
					{
						trouble = -1;
						goto PRIMARY_SECTOR_CLEAN_AND_SHIFT_ERROR_HANDLER;
					}
				}
        	}
        }
        else
        if( threshold_check_exceeded == 0)
        {
        	/* no action needed */
        	trouble = 0;
        	goto PRIMARY_SECTOR_CLEAN_AND_SHIFT_ERROR_HANDLER;
        }
        else
        {
        	/* this case should technically never be caught here since the range check passed to get here */
    		trouble = -1;
    		goto PRIMARY_SECTOR_CLEAN_AND_SHIFT_ERROR_HANDLER;
        }
	}

PRIMARY_SECTOR_CLEAN_AND_SHIFT_ERROR_HANDLER:
	return trouble;

}

/*FUNCTION**********************************************************************
 * Function Name : Persist_flash_erase_range
 * Description   : Erase a given range of persistent memory
 *
 * Parameters:
 * -void * base
 * -uint32_t bounds
 *
 * Return value:
 * -0 deleted succesfully
 * -<0 erase procedure failed
 *END**************************************************************************/
#if (defined(FLASH_DEBUG_MODE) && !FLASH_DEBUG_MODE)
int32_t Persist_flash_erase_range( void * base, uint32_t bounds )
#else
int32_t Persist_flash_erase_range( void * base, uint32_t bounds )
#endif
{
	    /************************ERASE ALL OF FLASH****************************/
	    uint32_t pflashSectorSize = 0;
		flash_security_state_t securityStatus = kFLASH_SecurityStateNotSecure; /* Return protection status */
		uint32_t result = 0;

	    FLASH_GetProperty(&s_flashDriver, kFLASH_PropertyPflashSectorSize, &pflashSectorSize);

		/* Check security status. */
		result = FLASH_GetSecurityState(&s_flashDriver, &securityStatus);
		if (kStatus_FLASH_Success != result)
		{
			result = -1;
		}

		/* we don't care about any arguments here */

		/* Erase the entire persistent section */
	    if (kFLASH_SecurityStateNotSecure == securityStatus)
	    {
	    	/* Erase Persistent section. */
	        result = FLASH_Erase(&s_flashDriver, (uint32_t)base, (uint32_t)(bounds), kFLASH_ApiEraseKey);
	        if (kStatus_FLASH_Success != result)
	        {
	        	result = -1;
	        }

	        /* Verify sector if it's been erased. */
	        result = FLASH_VerifyErase(&s_flashDriver, (uint32_t)base, (uint32_t)(bounds), kFLASH_MarginValueUser);
	        if (kStatus_FLASH_Success != result)
	        {
	        	result = -1;
	        }
	    }
	    else
	    {
	    	result = -1;
	    }
	    /**********************************************************************/

	    return result;
}

/*FUNCTION**********************************************************************
 * Function Name : Persist_flash_erase
 * Description   : Erase all of persistent flash memory banks
 *
 * Parameters:
 * -void
 *
 * Return value:
 * -0 deleted succesfully
 * -<0 erase procedure failed
 *END**************************************************************************/
#if (defined(FLASH_DEBUG_MODE) && !FLASH_DEBUG_MODE)
static int32_t Persist_flash_erase( void )
#else
int32_t Persist_flash_erase_all_banks( void )
#endif
{
	uint32_t result = 0;

	result = Persist_flash_erase_range( (void *)PERSIST_FLASH_BASE, PERSIST_TOTAL_BANK_SIZE );

	return result;


#if 0
	    /************************ERASE ALL OF FLASH****************************/
	    uint32_t pflashSectorSize = 0;
		flash_security_state_t securityStatus = kFLASH_SecurityStateNotSecure; /* Return protection status */
		uint32_t result = 0;

	    FLASH_GetProperty(&s_flashDriver, kFLASH_PropertyPflashSectorSize, &pflashSectorSize);

		/* Check security status. */
		result = FLASH_GetSecurityState(&s_flashDriver, &securityStatus);
		if (kStatus_FLASH_Success != result)
		{
			result = -1;
		}

		/* we don't care about any arguments here */

		/* Erase the entire persistent section */
	    if (kFLASH_SecurityStateNotSecure == securityStatus)
	    {
	    	/* Erase Persistent section. */
	        result = FLASH_Erase(&s_flashDriver, (uint32_t)PERSIST_FLASH_BASE, (uint32_t)(PERSIST_TOTAL_BANK_SIZE), kFLASH_ApiEraseKey);
	        if (kStatus_FLASH_Success != result)
	        {
	        	result = -1;
	        }

	        /* Verify sector if it's been erased. */
	        result = FLASH_VerifyErase(&s_flashDriver, (uint32_t)PERSIST_FLASH_BASE, (uint32_t)(PERSIST_TOTAL_BANK_SIZE), kFLASH_MarginValueUser);
	        if (kStatus_FLASH_Success != result)
	        {
	        	result = -1;
	        }
	    }
	    else
	    {
	    	result = -1;
	    }
	    /**********************************************************************/

	    return result;
#endif
}

/*FUNCTION**********************************************************************
 * Function Name : Persist_flash_mark_record_for_deletion
 * Description   : Writes a tag for a record to be deleted in our delete
 *                 bank/sector. These records will not be rewritten when a bank
 *                 clean is in operation.
 *
 * Parameters:
 * -uint32_t tag
 *
 * Return value:
 * -1 written successfully
 * -0 tag already marked for deletion
 * -<0 delete procedure failed: write failure or too many marked for deletion
 *END**************************************************************************/
int32_t Persist_flash_mark_record_for_deletion( uint32_t tag )
{
	uint32_t trouble = 0;
	uint32_t result  = 0;

	int32_t * tags_marked_for_deletion; //[PERSIST_DELETE_SECTOR_SIZE/4] = {0};

	int32_t ram_copy_buffer[PERSIST_DELETE_SECTOR_SIZE/4];

	uint32_t tag_iterator;
	int32_t new_tag_location;

    /* read in the current list */
	tags_marked_for_deletion = (int32_t *)PERSIST_DELETE_SECTOR_BASE;

	for(tag_iterator = 0; tag_iterator < PERSIST_DELETE_SECTOR_SIZE/4; tag_iterator++)
	{
		if(tags_marked_for_deletion[tag_iterator] == (int32_t)tag)
		{
			trouble = 0;
			goto MARK_RECORD_FOR_DELETION_ERROR_HANDLER;
		}
	}

	/* no duplicate, copy and write back */
	memcpy(ram_copy_buffer, tags_marked_for_deletion, PERSIST_DELETE_SECTOR_SIZE );

	tag_iterator = 0;
	new_tag_location = -1;

	/* scroll through the array until you find the first -1 */
	while(tag_iterator < (PERSIST_DELETE_SECTOR_SIZE/4))
	{
		if(ram_copy_buffer[tag_iterator] == -1)
		{
			/* write new value here */
			new_tag_location = tag_iterator;
			break;
		}

		tag_iterator++;
	}

	if(new_tag_location >= 0)
	{
        /* write new value into array then erase the delete sector and rewrite */
		ram_copy_buffer[new_tag_location] = tag;

		/* delete the delete sector */
		if( Persist_flash_erase_range( (void *)PERSIST_DELETE_SECTOR_BASE, PERSIST_DELETE_SECTOR_SIZE ) < 0 )
		{
			trouble = -1;
			goto MARK_RECORD_FOR_DELETION_ERROR_HANDLER;
		}

		/* now write the data to the sector */

    	/*************************************************/
    	/************PROGRAM BUFFER INTO FLASH************/

        result = FLASH_Program(&s_flashDriver, (uint32_t)PERSIST_DELETE_SECTOR_BASE, (uint32_t *)(ram_copy_buffer), PERSIST_DELETE_SECTOR_SIZE);
        if (kStatus_FLASH_Success != result)
        {
        	/* return persist flash record program failure */
        	trouble = -1;
			goto MARK_RECORD_FOR_DELETION_ERROR_HANDLER;
        }

        /* Verify programming by Program Check command with user margin levels */
        result = FLASH_VerifyProgram(	&s_flashDriver,
        								(uint32_t)PERSIST_DELETE_SECTOR_BASE,
										PERSIST_DELETE_SECTOR_SIZE,
										(const uint32_t *)(ram_copy_buffer), //MI: was (uint8_t *)&s_persistRecord,
										kFLASH_MarginValueUser,
										&failAddr, &failDat
									);
        if (kStatus_FLASH_Success != result)
        {
        	/* program verify failure */
        	trouble = -1;
			goto MARK_RECORD_FOR_DELETION_ERROR_HANDLER;
        }
    	/*************************************************/
    	/*************************************************/

        trouble = 1;
	}
	else
	{
		trouble = -1;
	}

MARK_RECORD_FOR_DELETION_ERROR_HANDLER:
	return trouble;
}

/*FUNCTION**********************************************************************
 * Function Name : Persist_flash_unmark_record_for_deletion
 * Description   : Takes a tag meant to be deleted out of the deletion list.
 *
 * Parameters:
 * -uint32_t tag
 *
 * Return value:
 * -1 if tag unmarked
 * -0 if corresponding tag not found
 * -<0 delete procedure failed: write failure
 *END**************************************************************************/
int32_t Persist_flash_unmark_record_for_deletion( uint32_t tag )
{
	uint32_t trouble = 0;
	uint32_t result  = 0;

	int32_t * tags_marked_for_deletion; //[PERSIST_DELETE_SECTOR_SIZE/4] = {0};

	int32_t ram_copy_buffer[PERSIST_DELETE_SECTOR_SIZE/4];
	int32_t ram_unmarked_copy_buffer[PERSIST_DELETE_SECTOR_SIZE/4];


	int32_t tag_iterator;
	int32_t  marked_tag_location;

    /* read in the current list */
	tags_marked_for_deletion = (int32_t *)PERSIST_DELETE_SECTOR_BASE;

	marked_tag_location = -1;
	for(tag_iterator = 0; tag_iterator < PERSIST_DELETE_SECTOR_SIZE/4; tag_iterator++)
	{
		if(tags_marked_for_deletion[tag_iterator] == (int32_t)tag)
		{
			/* found the tag to be unmarked */
			marked_tag_location = tag_iterator;
		}
	}

	if(marked_tag_location >= 0)
	{
		/* we found a tag that needs to be unmarked */

		/* copy and write back */
		memcpy(ram_copy_buffer, tags_marked_for_deletion, PERSIST_DELETE_SECTOR_SIZE );

		/* iterate and copy to another buffer */
		for(tag_iterator = 0; tag_iterator < PERSIST_DELETE_SECTOR_SIZE/4; tag_iterator++)
		{
			if(tag_iterator == marked_tag_location)
			{
				ram_unmarked_copy_buffer[tag_iterator] = -1;
			}
			else
			{
				ram_unmarked_copy_buffer[tag_iterator] = ram_copy_buffer[tag_iterator];
			}
		}

		/* delete the delete sector */
		if( Persist_flash_erase_range( (void *)PERSIST_DELETE_SECTOR_BASE, PERSIST_DELETE_SECTOR_SIZE ) < 0 )
		{
			trouble = -1;
			goto MARK_RECORD_FOR_DELETION_ERROR_HANDLER;
		}

	   	/*************************************************/
	    /************PROGRAM BUFFER INTO FLASH************/
	    result = FLASH_Program(&s_flashDriver, (uint32_t)PERSIST_DELETE_SECTOR_BASE, (uint32_t *)(ram_unmarked_copy_buffer), PERSIST_DELETE_SECTOR_SIZE);
	    if (kStatus_FLASH_Success != result)
	    {
	        /* return persist flash record program failure */
	        trouble = -1;
			goto MARK_RECORD_FOR_DELETION_ERROR_HANDLER;
	    }

	    /* Verify programming by Program Check command with user margin levels */
	    result = FLASH_VerifyProgram(	&s_flashDriver,
	        							(uint32_t)PERSIST_DELETE_SECTOR_BASE,
										PERSIST_DELETE_SECTOR_SIZE,
										(const uint32_t *)(ram_unmarked_copy_buffer),
										kFLASH_MarginValueUser,
										&failAddr, &failDat
										);
	    if (kStatus_FLASH_Success != result)
	    {
	        /* program verify failure */
	        trouble = -1;
	        goto MARK_RECORD_FOR_DELETION_ERROR_HANDLER;
	    }
	    /*************************************************/
	    /*************************************************/

	    trouble = 1;
	}
	else
	{
		/* tag doesn't need to be unmarked since it doesn't exist */
		trouble = 0;
	}

MARK_RECORD_FOR_DELETION_ERROR_HANDLER:
	return trouble;

}

/*FUNCTION**********************************************************************
 * Function Name : Persist_flash_get_records_marked_for_deletion
 * Description   : returns a pointer to the location in flash where the array
 *                 of marked tags for deletion are. Returns the number of
 *                 records that exist in the array
 *
 * Parameters:
 * -int32_t * tag_buffer
 *
 * Return value:
 * -# of records marked for deletion
 * -<0 if error
 *
 * Return pass by reference:
 * -int32_t * tag_buffer
 *END**************************************************************************/
int32_t Persist_flash_get_records_marked_for_deletion( int32_t ** tag_buffer)
{
	int32_t * tags_marked_for_deletion;
	uint32_t records_marked_count;

	uint32_t record_tag_iterator;

    /* read in the current list */
	tags_marked_for_deletion = (int32_t *)PERSIST_DELETE_SECTOR_BASE;
	records_marked_count = 0;

    for(record_tag_iterator = 0; record_tag_iterator < (PERSIST_DELETE_SECTOR_SIZE/4); record_tag_iterator++)
	{
    	if(tags_marked_for_deletion[record_tag_iterator] != -1)
    	{
    		records_marked_count++;
    	}
	}

    *tag_buffer = (int32_t *)PERSIST_DELETE_SECTOR_BASE;
	return records_marked_count;
}

/*FUNCTION**********************************************************************
 * Function Name : Persist_flash_initialize_delete_sector
 * Description   : initializes the array in the delete sector to all -1's.
 *
 * Parameters:
 * -void
 *
 * Return value:
 * -0 if successful
 * -<0 if error in programming
 *END**************************************************************************/
#if (defined(FLASH_DEBUG_MODE) && !FLASH_DEBUG_MODE)
static int32_t Persist_flash_initialize_delete_sector( void )
#else
int32_t Persist_flash_initialize_delete_sector( void )
#endif
{
	uint32_t result  = 0;
	uint32_t trouble = 0;

	/* initialize array to all 0's */
	uint32_t tags_marked_initial_array[PERSIST_DELETE_SECTOR_SIZE/4] = {[0 ... ((PERSIST_DELETE_SECTOR_SIZE/4) - 1)] = -1};

	/* delete the delete sector */
	if( Persist_flash_erase_range( (void *)PERSIST_DELETE_SECTOR_BASE, PERSIST_DELETE_SECTOR_SIZE ) < 0 )
	{
		trouble = -1;
		goto PERSIST_FLASH_INITIALIZE_ERROR_HANDLER;
	}

   	/*************************************************/
    /************PROGRAM BUFFER INTO FLASH************/
    result = FLASH_Program(&s_flashDriver, (uint32_t)PERSIST_DELETE_SECTOR_BASE, (uint32_t *)(tags_marked_initial_array), PERSIST_DELETE_SECTOR_SIZE);
    if (kStatus_FLASH_Success != result)
    {
        /* return persist flash record program failure */
        trouble = -1;
		goto PERSIST_FLASH_INITIALIZE_ERROR_HANDLER;
    }

    /* Verify programming by Program Check command with user margin levels */
    result = FLASH_VerifyProgram(	&s_flashDriver,
        							(uint32_t)PERSIST_DELETE_SECTOR_BASE,
									PERSIST_DELETE_SECTOR_SIZE,
									(const uint32_t *)(tags_marked_initial_array),
									kFLASH_MarginValueUser,
									&failAddr, &failDat
									);
    if (kStatus_FLASH_Success != result)
    {
        /* program verify failure */
        trouble = -1;
        goto PERSIST_FLASH_INITIALIZE_ERROR_HANDLER;
    }
    /*************************************************/
    /*************************************************/

    trouble = 0;

PERSIST_FLASH_INITIALIZE_ERROR_HANDLER:
	return trouble;
}
#if 0  /* not used */
/***********************************************************************************************************************************************************
 *                                                                COMPILER ASSERTIONS
 *                                                          **DO NOT USE THESE FUNCTIONS**
 *
 *
 *
 ************************************************************************************************************************************************************/
static void __ASSERT_PERSIST_RECORD__(void)
{
	/* IF THE RECORD IS NOT 8 BYTE ALLIGNED, THEN COMPILER ERROR */
	STATIC_ASSERT((sizeof(persistRecord_t) % 8) == 0,"FLASH RECORD NOT 8 BYTE ALLIGNED");

	/* IF THE RECORD FIELD [TYPE][TIME][DATA_LENGTH][TAG] IS NOT ALLIGED */
	//STATIC_ASSERT((RECORD_WIDTH_VALIDATION) == PERSIST_RECORD_ID_WIDTH,"RECORD FIELD ALLIGNMENT MISMATCH");

	/* IF THE SIZE OF PERSIST RECORD DIFFERS FROM THE PERSIST RECORD SIZE MACRO, MACRO NOT UPDATED, COMPILER ERROR */
	STATIC_ASSERT(sizeof(persistRecord_t) == RAM_RECORD_SIZE,"FLASH RECORD MACRO DOES NOT EQUAL ACTUAL RECORD SIZE");

	/* IF THE SIZE OF MIN FLASH RECORD SIZE MACRO DOESN'T ALIGN WITH ACTUAL MIN RECORD SIZE, COMPILER ERROR */
	STATIC_ASSERT((sizeof(persistRecordHeader_t) + sizeof(uint64_t) + (8 * sizeof(char))) == FLASH_RECORD_MIN_SIZE,"FLASH RECORD MACRO DOES NOT EQUAL ACTUAL RECORD SIZE");

}
#endif


/*FUNCTION*********************************************************************
 * Description   :
 *END**************************************************************************/
int Persist_flash_Clear( void )
{
	int trouble = SUCCESS;

	flash_config_t s_flashDriver;
/* TODO:  bust up function into smaller logical functions (some might be reusable?)
 * TODO:  error handling at bottom of function, add error codes.
 * TODO:  adding command arguments to indicate what is to be run.
 */
	/* erase persistent flash sectors */

	flash_security_state_t securityStatus = kFLASH_SecurityStateNotSecure; /* Return protection status */
	status_t result;    /* Return code from each flash driver function */
//	uint32_t destAdrss; /* Address of the target location */
//	uint32_t i, failAddr, failDat;
    uint32_t pflashSectorSize = 0;

    /* Clean up Flash driver Structure*/
    memset( &s_flashDriver, 0, sizeof( flash_config_t ) );

    /* Setup flash driver structure for device and initialize variables. */
    result = FLASH_Init( &s_flashDriver );
    if ( kStatus_FLASH_Success != result )
    {
		trouble = -EC_FLASH_INIT;
		goto DP_Top_Persist_Clear_ERROR_HANDLER;
    }

    FLASH_GetProperty( &s_flashDriver, kFLASH_PropertyPflashSectorSize, &pflashSectorSize );

	/* Check security status. */
	result = FLASH_GetSecurityState( &s_flashDriver, &securityStatus );
	if ( kStatus_FLASH_Success != result )
	{
		trouble = -EC_FLASH_SECURITY;
		goto DP_Top_Persist_Clear_ERROR_HANDLER;
	}

	/* we don't care about any arguments here */

	/* Erase the entire persistent section */
    if ( kFLASH_SecurityStateNotSecure == securityStatus )
    {
    	/* Erase Persistent section. */
        result = FLASH_Erase( 	&s_flashDriver,
        						(uint32_t)PERSIST_FLASH_BASE,
								(uint32_t)(PERSIST_TOTAL_BANK_SIZE),
								kFLASH_ApiEraseKey
							);
        if ( kStatus_FLASH_Success != result )
        {
    		trouble = -EC_FLASH_ERASE;
    		goto DP_Top_Persist_Clear_ERROR_HANDLER;
        }

        /* Verify sector if it's been erased. */
        result = FLASH_VerifyErase( &s_flashDriver,
        							(uint32_t)PERSIST_FLASH_BASE,
									(uint32_t)(PERSIST_TOTAL_BANK_SIZE),
									kFLASH_MarginValueUser
								  );
        if (kStatus_FLASH_Success != result)
        {
    		trouble = -EC_FLASH_ERASE_VERIFY;
    		goto DP_Top_Persist_Clear_ERROR_HANDLER;
        }
    }
    else
    {
		trouble = -EC_FLASH_SECURITY_STATUS;
		goto DP_Top_Persist_Clear_ERROR_HANDLER;
    }

    Persist_flash_initialize();

DP_Top_Persist_Clear_ERROR_HANDLER:
	return trouble;
}
