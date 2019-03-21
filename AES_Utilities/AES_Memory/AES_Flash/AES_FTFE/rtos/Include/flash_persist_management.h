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

#include "stdint.h"
#include "APEX_timestamp.h"
#include "buffer_shared.h"
#include "stdbool.h"
#include "spsc_fifo.h"
#include "Persist_Config.h"
#include "AES_Queue.h"
#include "Persist_Config.h"


#ifndef SOURCE_FLASH_PERSIST_MANAGEMENT_H_
#define SOURCE_FLASH_PERSIST_MANAGEMENT_H_

#define FLASH_DEBUG_MODE						1
#define FLASH_RTOS_ENABLED                      1 /* enables delegation of Flash resources to persistence and idle tasks */
#define FLASH_BUFFERED_PERSISTENCE              1 /* defers read and write requests via queues and fifos */


/*MACRO**********************************************************************
 * Description   : The Subversion (SVN) time/date marker which is updated
 * during commit of the source file to the repository.
 *END**************************************************************************/
#define FLASH_PERSIST_MANAGEMENT_C_SVN_REV  "$Date: 2017-02-13 19:07:04 -0600 (Mon, 13 Feb 2017) $"

/***********************************************************************************************************************************************************
 *                                                                PERSISTENCE WRITE FIFOS
 *
 *
 *
 *
 ************************************************************************************************************************************************************/
CircularQueue_t Persist_Write_Queue;    /* first level write queue */
CircularQueue_t Persist_Commit_Queue;   /* overflow fifo           */

uint8_t Persist_Write_Queue_Buffer[PERSIST_WRITE_QUEUE_BUFFER_SIZE];
uint8_t Persist_Commit_Queue_Buffer[PERSIST_COMMIT_QUEUE_BUFFER_SIZE];

/***********************************************************************************************************************************************************
 *                                                                PERSISTENCE TASK MESSAGES
 *
 *
 *
 *
 ************************************************************************************************************************************************************/
#define WRITE_QUEUE_NOT_EMPTY                   x01
#define COMMIT_QUEUE_NOT_EMPTY                  x02

/***********************************************************************************************************************************************************
 *                                                                PERSISTENCE RECORD MACROS
 *
 *
 *
 *
 ************************************************************************************************************************************************************/
#define CONTAINER_BYTE_LENGTH 					1024         /* **CHANGED TO 4Kb 772017** Defines container byte width - USER CHANGEABLE */
//#define CONTAINER_BYTE_LENGTH 					32        /* **CHANGED TO 4Kb 772017** Defines container byte width - USER CHANGEABLE */

#define CONTAINER_BYTE_MIN_WRITE_LENGTH			8
#define RAM_RECORD_SIZE							16 + CONTAINER_BYTE_LENGTH + 8 /* defines the size of a record - **DO NOT CHANGE**/
#define FLASH_RECORD_MIN_SIZE					16 + CONTAINER_BYTE_MIN_WRITE_LENGTH + 8
#define RECORD_STRUCT_ID			    		0x965883AE /* static structure identifier to be able to flag records in flash */
#define MAX_PERSIST_RECORDS_ALLOWED				( (PERSIST_FIRST_BANK_SECTOR_COUNT) * (PERSIST_FLASH_SECTOR_SIZE) ) / ( FLASH_RECORD_MIN_SIZE )
#define PERSIST_FLASH_MAX_HISTORY				3
#define PERSIST_FLASH_FILL_THRESHOLD            75 /* percent */

/* FORMAT
 *
 * uint32_t - 32 bit total encoding
 * [TYPE][TIMECONFIG][DATA_LENGTH][TAG]
 *
 */
#define PERSIST_RECORD_ID_WIDTH					32
#define PERSIST_RECORD_TYPE_WIDTH				BUFFER_SHARED_WIDTH
#define PERSIST_RECORD_TIMECONFIG_WIDTH			4//(APEX_NUMBER_TIMESTAMP_CONFIGURATIONS)
#define PERSIST_RECORD_DATA_LENGTH_WIDTH		12//(BITS_TO_REPRESENT(CONTAINER_BYTE_LENGTH))
#define PERSIST_RECORD_TAG_WIDTH				(PERSIST_RECORD_ID_WIDTH - (PERSIST_RECORD_TYPE_WIDTH + PERSIST_RECORD_DATA_LENGTH_WIDTH + PERSIST_RECORD_TIMECONFIG_WIDTH))

#define PERSIST_RECORD_TYPE_MASK				( ( 1 << PERSIST_RECORD_TYPE_WIDTH        ) - 1 )
#define PERSIST_RECORD_TIMECONFIG_MASK			( ( 1 << PERSIST_RECORD_TIMECONFIG_WIDTH  ) - 1 )
#define PERSIST_RECORD_DATA_LENGTH_MASK			( ( 1 << PERSIST_RECORD_DATA_LENGTH_WIDTH ) - 1 )
#define PERSIST_RECORD_TAG_MASK					( ( 1 << PERSIST_RECORD_TAG_WIDTH         ) - 1 )

#define PERSIST_RECORD_TAG_SHIFT				0
#define PERSIST_RECORD_DATA_LENGTH_SHIFT		(PERSIST_RECORD_TAG_SHIFT         + PERSIST_RECORD_TAG_WIDTH         )
#define PERSIST_RECORD_TIMECONFIG_SHIFT			(PERSIST_RECORD_DATA_LENGTH_SHIFT + PERSIST_RECORD_DATA_LENGTH_WIDTH )
#define PERSIST_RECORD_TYPE_SHIFT				(PERSIST_RECORD_TIMECONFIG_SHIFT  + PERSIST_RECORD_TIMECONFIG_WIDTH  )

#define PERSIST_RECORD_ID_BUILD(type, timeconfig, data_length, tag)	  										\
		(	                                              													\
		( ( ( type        ) & PERSIST_RECORD_TYPE_MASK        )	<<	PERSIST_RECORD_TYPE_SHIFT               ) |	\
		( ( ( timeconfig  ) & PERSIST_RECORD_TIMECONFIG_MASK  )	<<	PERSIST_RECORD_TIMECONFIG_SHIFT 		) |	\
		( ( ( data_length ) & PERSIST_RECORD_DATA_LENGTH_MASK )	<<	PERSIST_RECORD_DATA_LENGTH_SHIFT 		) |	\
		( ( ( tag         ) & PERSIST_RECORD_TAG_MASK         )	<<	PERSIST_RECORD_TAG_SHIFT               	)   \
			                                                         	 	 	 	                        )


#define PERSIST_RECORD_TAG_GET(x)				( ( (x) >> PERSIST_RECORD_TAG_SHIFT) & PERSIST_RECORD_TAG_MASK )
#define PERSIST_RECORD_TYPE_GET(x)				( ( (x) >> PERSIST_RECORD_TYPE_SHIFT) & PERSIST_RECORD_TYPE_MASK)
#define PERSIST_RECORD_TIMECONFIG_GET(x)		( ( (x) >> PERSIST_RECORD_TIMECONFIG_SHIFT ) & PERSIST_RECORD_TIMECONFIG_MASK)
#define PERSIST_RECORD_DATA_LENGTH_GET(x)		( ( (x) >> PERSIST_RECORD_DATA_LENGTH_SHIFT ) & PERSIST_RECORD_DATA_LENGTH_MASK)

#define PERSIST_RECORD_DATA_LENGTH_PADDING(x)   ((8 - ((x) & 0x07)) & 0x07) //additional 0x07 mask to mask out 8

/***********************************************************************************************************************************************************
 *                                                                PERSISTENCE BANK CONFIG
 *
 *
 *
 *
 ************************************************************************************************************************************************************/
/* definitions for persistence setup */
#define P_FLASH_FINAL_ALIGNED_ADDRESS           0x000FFFF0
#define __ONE_KB								1024

#define PERSIST_FLASH_SECTOR_SIZE				(4 * __ONE_KB) /* 0x00001000 */
#define PERSIST_FIRST_BANK_SECTOR_COUNT			2/* 4 */ /* number of sectors per bank */
#define PERSIST_SECOND_BANK_SECTOR_COUNT        2/* 4 */ /* number of sectors per bank */

#define PERSIST_REGION_SIZE						(( (PERSIST_FIRST_BANK_SECTOR_COUNT + PERSIST_SECOND_BANK_SECTOR_COUNT) * PERSIST_FLASH_SECTOR_SIZE ) + (4 * __ONE_KB)) /* +4 KB for house keeping which will be the first section*/
#define PERSIST_REGION_BASE						(P_FLASH_FINAL_ALIGNED_ADDRESS - PERSIST_REGION_SIZE)
#define PERSIST_FLASH_BASE						(PERSIST_REGION_BASE + __ONE_KB) //p-flash limit is x000F_FFFF, we are using the last 9kb  /*0x000FC000	changed April 11, 2018 *//* changed from 0x000F8000 9/30/17 */

/****************************************************************************************************************************************************************/
/****************************************************************************************************************************************************************/
#define PERSIST_DELETE_SECTOR_BASE              0x000F7808  /* determined mathematically using PERSIST_DELETE_SECTOR_BASE_CALCULATE */ //TODO: FIX ADDRESS APRIL 11, 2018
#define PERSIST_DELETE_SECTOR_SIZE              (4 * MAX_PERSIST_RECORDS_ALLOWED) + PERSIST_RECORD_DATA_LENGTH_PADDING(4 * MAX_PERSIST_RECORDS_ALLOWED)
#define PERSIST_DELETE_SECTOR_BASE_CALCULATE    PERSIST_FLASH_BASE - PERSIST_DELETE_SECTOR_SIZE
/****************************************************************************************************************************************************************/
/****************************************************************************************************************************************************************/

#define PERSIST_FIRST_BANK_SIZE					(PERSIST_FIRST_BANK_SECTOR_COUNT * PERSIST_FLASH_SECTOR_SIZE)
#define PERSIST_SECOND_BANK_SIZE				(PERSIST_SECOND_BANK_SECTOR_COUNT * PERSIST_FLASH_SECTOR_SIZE)

#define PERSIST_TOTAL_SECTORS					(PERSIST_FIRST_BANK_SECTOR_COUNT + PERSIST_SECOND_BANK_SECTOR_COUNT)
#define PERSIST_TOTAL_BANK_SIZE					PERSIST_FLASH_SECTOR_SIZE * PERSIST_TOTAL_SECTORS

/* PERSISTENCE MACROS */
#define PERSIST_FLASH_FIRST_BANK_BASE   		PERSIST_FLASH_BASE
#define PERSIST_FLASH_SECOND_BANK_BASE  		(PERSIST_FLASH_BASE + (PERSIST_FIRST_BANK_SIZE))
#define PERSIST_FLASH_END_OF_PERSISTENCE		(PERSIST_FLASH_SECOND_BANK_BASE + (4 * PERSIST_FLASH_SECTOR_SIZE))

/* GENERAL FLASH MACROS */
#define K64_FLASH_START_ADDR                    0x00000000
#define K64_FLASH_END_ADDR                      0x000FFFFF

/***********************************************************************************************************************************************************
 *                                                                FLASH MEMORY CONFIGURATION
 *
 *
 *
 *
 ************************************************************************************************************************************************************/
/* Macros defining end of flash */
#define FLASH_MEMORY_ORIGIN						0x00000410
#define FLASH_MEMORY_LENGTH						0x000FFBF0
#define END_OF_FLASH_MEMORY						FLASH_MEMORY_ORIGIN + FLASH_MEMORY_LENGTH

#define PERSIST_FLASH_TIMESTAMP_MODE			( (uint32_t) APEX_TIMESTAMP__BASIC_COUNT )
#define PERSIST_FLASH_TIMESTAMP_MODE_MASK		0x0000000F
#define CHECKSUM_FLETCHER_32					1

/* persist region init header */
typedef struct persistInitHeader
{
	uint32_t primary_bank_ptr;
	uint32_t last_operation_valid;
	uint64_t checksum;
}persistInitHeader_t;

/* persist record header */
typedef struct persistRecordHeader
{
		uint32_t struct_id;  /* identify if its a record */
		uint32_t record_id;  /* identify the record type */
		uint32_t timestamp;  /* identify when the record was made */
		uint32_t record_tag; /* identify the record itself */

}persistRecordHeader_t;

/* Persist record CONTAINER */
typedef struct persistRecord
{
	persistRecordHeader_t persist_record_header;

	char container[CONTAINER_BYTE_LENGTH]; /* Persistent record book keeping */

	uint64_t checksum;  /* checksum for the entire structure */

}persistRecord_t;

/***********************************************************************************************************************************************************
 *                                                                PUBLIC FUNCTION PROTOTYPES
 *
 *
 *
 *
 ************************************************************************************************************************************************************/
/* [  ] */ int32_t Persist_flash_initialize(void);
/* [X+] */ int32_t Persist_flash_get_valid_record_count(void ** recordPointer);
/* [X+] */ persistRecord_t * Persist_Flash_Get_Recent_Record(int32_t tag, apexTimestamp_t * timestamp);
/* [X+] */ persistRecord_t * Persist_flash_get_historical_record(int32_t tag, uint32_t recent, apexTimestamp_t * timestamp, uint32_t * number_of_type_records);
/* [X+] */ int32_t Persist_flash_get_record_history(int32_t tag, size_t bufferSize, persistRecord_t ** buffer);
/* [X+] */ int32_t Persist_Flash_Create_Record(persistRecord_t * record_in_ram, int type, int32_t tag, uint32_t data_length, char * data);
/* [X+] */ int32_t Persist_Flash_Record_Commit(persistRecord_t * record_in_ram);
/* [X+] */ int32_t Persist_Flash_Record_Copy_To_RAM(persistRecord_t * record_in_ram, void * record_in_flash);

		   int32_t Persist_flash_delete_all_records( void );
		   int32_t Persist_flash_delete_record_history_by_tag( uint32_t tag, bool keep_most_recent_record);
		   int32_t Persist_flash_delete_history_keep_recent_all( void );
		   void * persist_flash_get_primary_bank( void );



#if (defined(FLASH_DEBUG_MODE) && FLASH_DEBUG_MODE)
 /* [X+] */ int32_t Persist_flash_record_valid(void * recordPtr);
 /* [X+] */ int32_t Persist_flash_record_exists(persistRecord_t * recordPtr);
 /* [X+] */ int32_t Persist_ram_record_valid(persistRecord_t * recordPtr);
 /* [X+] */ uint64_t Persist_record_checksum(void * record);
 /* [X+] */	uint64_t fletcher_checksum( uint32_t number_of_words, void * data_ptr);
 /* [X+] */ void Persist_get_timestamp_config(apexTimestamp_t * t_config);
 /* [X+]  enum buffer_shared_type Persist_get_record_type(persistRecord_t * record, uint8_t * isString); */
 /* [X+] */ int32_t Persist_get_record_tag(persistRecord_t * record);
 /* [X+] */ int32_t Persist_get_record_size_internal(persistRecord_t * record);
 /* [depricated] */ int32_t Persist_flash_get_total_record_count(void * base, uint32_t bound_length_bytes);
 /* [todo::extend] */ int32_t Persist_ram_compare_record(persistRecord_t * record_a, persistRecord_t * record_b);
 /* [*X] */ int32_t Persist_flash_write_record(persistRecord_t * record_in_ram, void * base);
 int32_t Persist_flash_record_bank_clean_request(persistRecord_t ** last_valid_record);
 /* [X+] */ int32_t Persist_flash_range_check(void * base, uint32_t bounds );
 /* [X+] */ int32_t Persist_flash_threshold_check(void * base, uint32_t bounds, uint32_t fill_threshold_percent );
 /* [X+] */ int32_t Persist_flash_unique_tags_get(void * base, uint32_t bounds, uint32_t * tag_buffer, uint32_t tag_buffer_size );
 /* [X+] */ int32_t Persist_flash_erase_all_banks( void );
 /* [X+] */ int32_t Persist_flash_erase_range( void * base, uint32_t bounds );
 /* [X+] */ int32_t Persist_flash_determine_primary_bank_and_recover( void );
 /* [X+] */ int32_t Persist_flash_primary_sector_clean_and_shift(void * base, uint32_t bounds, uint32_t force_clean, uint32_t threshold, uint32_t history );
 /* [X+] */ int32_t Persist_flash_initialize_delete_sector( void );
 /* [X+] */ int32_t Persist_flash_mark_record_for_deletion( uint32_t tag );
 /* [X+] */ int32_t Persist_flash_unmark_record_for_deletion( uint32_t tag );
 /* [X+] */ int32_t Persist_flash_get_records_marked_for_deletion( int32_t ** tag_buffer);
 /* [x ] */ int32_t Persist_flash_region_initialize( void ** primary_bank_ptr );
 /* [  ] */ int32_t Persist_flash_garbage_collect( void );
 /* [  ] */ int32_t Persist_flash_current_bank_records_get( persistRecord_t ** buffer, uint32_t buffer_size );


 #endif


 enum
 {
	 __PERSIST_ALL_BANKS_ERASED,
	 __PERSIST_BANK_1_ERASED,
	 __PERSIST_BANK_2_ERASED
 };


int Persist_flash_Clear( void );


#endif /* SOURCE_FLASH_PERSIST_MANAGEMENT_H_ */
