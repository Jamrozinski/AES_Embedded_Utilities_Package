

/********************************************************************************
 * Description:
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




#include <stdlib.h>   /* atoi()     */
#include <string.h>	  /* strcmp 	*/
#include <strings.h>  /* strcasecmp */

#include "fsl_shell.h"
#include "fsl_clock.h"
#include "error_info.h"
//#include "adc_user_commands.h"
#include "stdbool.h"
#include "pit_timer_init.h"
#include "fsl_shell.h"
#include "fsl_uart_freertos.h"

#include "shell_telnet.h"
#include "shell_uart4.h"
#include "shell_uart0.h"
#include "APEX_Hardware_config.h"

#include "flash_abstraction.h"

#include "flash_persist_management.h"
#include "buffer_shared.h"


/*MACRO**********************************************************************
 * Description   : The Subversion (SVN) time/date marker which is updated
 * during commit of the source file to the repository.
 *END**************************************************************************/
#define FLASH_USER_COMMANDS_C_SVN_REV  "$Date: 2017-04-21 14:57:08 -0500 (Fri, 21 Apr 2017) $"

/*FUNCTION**********************************************************************
 * Description   :
 *END**************************************************************************/
//uint32_t FLASH_CFG_DMP( p_shell_context_t context, int32_t argc, char **argv )
int32_t Flash_Configuration_Dump( p_shell_context_t context, int32_t argc, char **argv )
{
	int32_t trouble = SUCCESS;
	flash_config_t s_uflashDriver;
	(void) argc;
	(void) argv;

	/* Clean up Flash driver Structure*/
	memset(&s_uflashDriver, 0, sizeof(flash_config_t));

	/* Setup flash driver structure for device and initialize variables. */
	trouble = FLASH_Init(&s_uflashDriver);
	if ( kStatus_FLASH_Success != trouble )
	{
		context->printf_data_func("Error: Flash Initialization Error \r\n");
		goto Flash_Configuration_Dump_ERROR_HANDLER;
	}

	flash_info_t s_dflashCfg;
	flash_info_t s_pflashCfg;

    /* Clean up Flash driver Structure*/
    memset(&s_dflashCfg, 0, sizeof(flash_info_t));
    memset(&s_pflashCfg, 0, sizeof(flash_info_t));

    uFlash_info(&s_uflashDriver, &s_dflashCfg, kFlash_User);
    uFlash_info(&s_uflashDriver, &s_pflashCfg, kFlash_Program);

	//continue reading FIFO until there's nothing more remaining
	context->printf_data_func("User Flash Block Base Addr Hex: \t\t(0x%x) \r\n", s_dflashCfg.flashBlockBaseAddr);
	context->printf_data_func("Total User Block Count:\t\t%d Blocks \r\n", s_dflashCfg.flashBlockCount);
	context->printf_data_func("Total User Flash Block Size:\t\t%d KB, Hex: (0x%x) \r\n", (s_dflashCfg.flashBlockSize/1024), s_dflashCfg.flashBlockSize);
	context->printf_data_func("Total User Flash Sector Size:\t%d KB, Hex: (0x%x) \r\n", (s_dflashCfg.flashSectorSize /1024), s_dflashCfg.flashSectorSize);
	context->printf_data_func("Total User Flash Size:\t\t%d KB, Hex: (0x%x) \r\n\n", (s_dflashCfg.flashTotalSize / 1024), s_dflashCfg.flashTotalSize);

	//continue reading FIFO until there's nothing more remaining
	context->printf_data_func("Program Flash Block Base Addr Hex: \t\t(0x%x) \r\n", s_pflashCfg.flashBlockBaseAddr);
	context->printf_data_func("Total Program Block Count:\t\t%d Blocks \r\n", s_pflashCfg.flashBlockCount);
	context->printf_data_func("Total Program Flash Block Size:\t\t%d KB, Hex: (0x%x) \r\n", (s_pflashCfg.flashBlockSize/1024), s_pflashCfg.flashBlockSize);
	context->printf_data_func("Total Program Flash Sector Size:\t%d KB, Hex: (0x%x) \r\n", (s_pflashCfg.flashSectorSize /1024), s_pflashCfg.flashSectorSize);
	context->printf_data_func("Total Program Flash Size:\t\t%d KB, Hex: (0x%x) \r\n", (s_pflashCfg.flashTotalSize / 1024), s_pflashCfg.flashTotalSize);

Flash_Configuration_Dump_ERROR_HANDLER:
//TODO: translate freescale error code to error_info general error code.
	return trouble;
}

/*FUNCTION**********************************************************************
 * Description   :
 *END**************************************************************************/
//uint32_t FLS_TEST_WRITE(p_shell_context_t context, int32_t argc, char ** argv)
int32_t Flash_Test_Write( p_shell_context_t context, int32_t argc, char ** argv )
{
	int32_t trouble = SUCCESS;
	uint32_t s_buffer[4];
	uint32_t s_buffer_rbc[4];
	(void) argc;
	(void) argv;

	flash_security_state_t securityStatus = kFLASH_SecurityStateNotSecure; /* Return protection status */
	status_t result;    /* Return code from each flash driver function */
	uint32_t destAdrss; /* Address of the target location */
	uint32_t i, failAddr, failDat;

	flash_config_t s_flashDriver;

	/* Clean up Flash driver Structure*/
	memset(&s_flashDriver, 0, sizeof(flash_config_t));

    uint32_t pflashBlockBase = 0;
    uint32_t pflashTotalSize = 0;
    uint32_t pflashSectorSize = 0;

    /* Clean up Flash driver Structure*/
    memset(&s_flashDriver, 0, sizeof(flash_config_t));

    /* Setup flash driver structure for device and initialize variables. */
    result = FLASH_Init(&s_flashDriver);
    if (kStatus_FLASH_Success != result)
    {
    	context->printf_data_func("Error: Error during Flash Init\r\n");
    	trouble = result; //TODO: translate.
    	goto Flash_Test_Write_ERROR_HANDLER;
    }
    /* Get flash properties*/
    FLASH_GetProperty(&s_flashDriver, kFLASH_PropertyPflashBlockBaseAddr, &pflashBlockBase);
    FLASH_GetProperty(&s_flashDriver, kFLASH_PropertyPflashTotalSize, &pflashTotalSize);
    FLASH_GetProperty(&s_flashDriver, kFLASH_PropertyPflashSectorSize, &pflashSectorSize);

    /* Check security status. */
    result = FLASH_GetSecurityState(&s_flashDriver, &securityStatus);
    if (kStatus_FLASH_Success != result)
    {
    	context->printf_data_func("Error: Error during security state retrieval\r\n");
    	trouble = result;  //TODO: translate.
    	goto Flash_Test_Write_ERROR_HANDLER;
    }
    /* Print security status. */
    switch (securityStatus)
    {
        case kFLASH_SecurityStateNotSecure:
        	context->printf_data_func("\r\n Flash is UNSECURE!");
            break;
        case kFLASH_SecurityStateBackdoorEnabled:
        	context->printf_data_func("\r\n Flash is SECURE, BACKDOOR is ENABLED!");
            break;
        case kFLASH_SecurityStateBackdoorDisabled:
        	context->printf_data_func("\r\n Flash is SECURE, BACKDOOR is DISABLED!");
            break;
        default:
            break;
    }
    context->printf_data_func("\r\n");

    /* Test pflash basic opeation only if flash is unsecure. */
    if (kFLASH_SecurityStateNotSecure == securityStatus)
    {
        /* Debug message for user. */
        /* Erase several sectors on upper pflash block where there is no code */
    	context->printf_data_func("\r\n Erase a sector of flash\r\n");

/* Erase a sector from destAdrss. */
#if defined(FSL_FEATURE_FLASH_HAS_PFLASH_BLOCK_SWAP) && FSL_FEATURE_FLASH_HAS_PFLASH_BLOCK_SWAP
        /* Note: we should make sure that the sector shouldn't be swap indicator sector*/
        destAdrss = pflashBlockBase + (pflashTotalSize - (pflashSectorSize * 2));
#else
        destAdrss = pflashBlockBase + (pflashTotalSize - pflashSectorSize);
#endif
        result = FLASH_Erase(&s_flashDriver, destAdrss, pflashSectorSize, kFLASH_ApiEraseKey);
        if (kStatus_FLASH_Success != result)
        {
        	context->printf_data_func("Error: Error during flash erase\r\n");
        	trouble = result;
        	//goto Flash_Test_Write_ERROR_HANDLER;
        }

        /* Verify sector if it's been erased. */
        result = FLASH_VerifyErase(&s_flashDriver, destAdrss, pflashSectorSize, kFLASH_MarginValueUser);
        if (kStatus_FLASH_Success != result)
        {
        	context->printf_data_func("Error: Error during flash erase verify\r\n");
        	trouble = result;
        	//goto Flash_Test_Write_ERROR_HANDLER;
        }

        /* Print message for user. */
        context->printf_data_func("\r\n Successfully Erased Sector 0x%x -> 0x%x\r\n", destAdrss, (destAdrss + pflashSectorSize));

        /* Print message for user. */
        context->printf_data_func("\r\n Program a buffer to a sector of flash \r\n");
        /* Prepare user buffer. */
        for (i = 0; i < 4; i++)
        {
            s_buffer[i] = i;
        }
        /* Program user buffer into flash*/
        result = FLASH_Program(&s_flashDriver, destAdrss, s_buffer, sizeof(s_buffer));
        if (kStatus_FLASH_Success != result)
        {
        	context->printf_data_func("Error: Error during flash program\r\n");
        }

        /* Verify programming by Program Check command with user margin levels */
        result = FLASH_VerifyProgram(&s_flashDriver, destAdrss, sizeof(s_buffer), s_buffer, kFLASH_MarginValueUser,
                                     &failAddr, &failDat);
        if (kStatus_FLASH_Success != result)
        {
        	context->printf_data_func("Error: Error during flash program verify\r\n");
        	trouble = result;
        	//goto Flash_Test_Write_ERROR_HANDLER;
        }
        /* Verify programming by reading back from flash directly*/
        for (uint32_t i = 0; i < 4; i++)
        {
            s_buffer_rbc[i] = *(volatile uint32_t *)(destAdrss + i * 4);
            if (s_buffer_rbc[i] != s_buffer[i])
            {
            	context->printf_data_func("Error: Error during flash read-after-program! Data read does not match data written!\r\n");
            }
        }

        context->printf_data_func("\r\n Successfully Programmed and Verified Location 0x%x -> 0x%x \r\n", destAdrss,
               (destAdrss + sizeof(s_buffer)));

        /* Erase the context we have progeammed before*/
        /* Note: we should make sure that the sector which will be set as swap indicator should be blank*/
        FLASH_Erase(&s_flashDriver, destAdrss, pflashSectorSize, kFLASH_ApiEraseKey);
    }
    else
    {
    	context->printf_data_func("\r\n Erase/Program operation will not be executed, as Flash is SECURE!\r\n");
    }
Flash_Test_Write_ERROR_HANDLER:
//TODO: translate freescale error code to error_info general error code.
	return trouble;
}

/*FUNCTION**********************************************************************
 * Description   :
 *END**************************************************************************/
//uint32_t FLS_PERSIST_WR(p_shell_context_t context, int32_t argc, char ** argv)
int32_t Flash_Persist_Write( p_shell_context_t context, int32_t argc, char ** argv )
{
	int32_t trouble = SUCCESS;
	/* ensure the user passed in appropriate arguments */
	uint32_t argument_index;
//	uint32_t phrase_length;
	persistRecord_t s_persistRecord;
//	persistRecord_t * s_persistRecordPtr;
	char * strBuf;
	char copy_char;

	strBuf = &(s_persistRecord.container[0]);
	argument_index = 1;

	if(argc != 2)
	{
		context->printf_data_func("\r\n Argument Error: FLS_pWR \"Example String\" \n\r");
		return 0;
	}
	else
	{

		/* basic lexical analysis */
		uint32_t currentCharacter = 0;
		for(currentCharacter = 0; currentCharacter < CONTAINER_BYTE_LENGTH; currentCharacter++)
		{
			copy_char = argv[argument_index][currentCharacter];

			*(strBuf + currentCharacter) = copy_char;

		}

		/* end with null terminator */
		*(strBuf + (CONTAINER_BYTE_LENGTH - 1)) = '\0';

		context->printf_data_func("\r\n Storing string: \"%s\"\n\r", strBuf);
#if(0)
		if(argv[argument_index][0] == '\"')
		{
			/* start of phrase */
			phrase_length = 0;
			uint32_t character_index = 0;
			for(argument_index = 1; argument_index < argc; argument_index++)
			{
				character_index = 0;
				copy_char = argv[argument_index][character_index];
				while(copy_char != '\0')
				{
					/* arguments are guaranteed to be null terminated */

					/* ignore first quotation mark */
					if(argument_index == 1 && character_index == 0 && copy_char == '\"')
					{
						character_index++;
						copy_char = argv[argument_index][character_index];
					}
					else
					{
						if(copy_char == '\"')
						{
							/* don't increase phrase length, end of phrase reached*/

							if(!(argument_index + 1) == argc)
							{
								/* theres still arguments after the last delimiter */
								context->printf_data_func("\r\n Error: FLS_pWR \"Example String\"\n\r");
								return 0;
							}
							else
							{
								/* theres still characters on the last argument after the last delimiter */
								if(argv[argument_index][character_index + 1] != '\0')
								{
									context->printf_data_func("\r\n Error: FLS_pWR \"Example String\"\n\r");
									return 0;
								}
								else
								{
									/* don't increment phrase length */
									character_index++;
									copy_char = argv[argument_index][character_index];
								}
							}
						}
						else
						{
							/* otherwise we simply have a character */
							phrase_length++;

							if(phrase_length >  RECORD_STRING_LENGTH)
							{
								context->printf_data_func("\r\n Error: Phrase length longer than %d characters\n\r", RECORD_STRING_LENGTH);
								return 0;
							}
							else
							{
								*(strBuf + (phrase_length -1)) = copy_char; /* strBuf[phrase_length -1] = copy_char; */
								character_index++;
								copy_char = argv[argument_index][character_index];

							}
						}
					}
				}

				/* write a space character for all arguments that aren't the last one */
				if(argument_index != (argc - 1) )
				{
					/* otherwise we simply have a character */
					phrase_length++;

					if(phrase_length >  RECORD_STRING_LENGTH)
					{
						context->printf_data_func("\r\n Error: Phrase length longer than %d characters\n\r", RECORD_STRING_LENGTH);
						return 0;
					}
					else
					{
						*(strBuf + (phrase_length -1)) = ' '; /* strBuf[phrase_length -1] = copy_char; */
					}
				}
			}
		}
		else
		{
			context->printf_data_func("\r\n Error: FLS_pWR \"Example String\"\n\r");
		}
#endif
	}

	/* set the static struct ID to recognize a record during flashScan */
	//s_persistRecord->persistRecord_header.struct_id = RECORD_STRUCT_ID;

	/* set rest of parameters */
	//s_persistRecord->persistRecord_header.timestamp = 500;
	s_persistRecord.checksum = 5;

	flash_security_state_t securityStatus = kFLASH_SecurityStateNotSecure; /* Return protection status */
	status_t result;    /* Return code from each flash driver function */
//	uint32_t destAdrss; /* Address of the target location */
//	uint32_t i, failAddr, failDat;

	flash_config_t s_flashDriver;

//	persistRecord_t uFlashRecord;

	/* Clean up Flash driver Structure*/
	memset(&s_flashDriver, 0, sizeof(flash_config_t));

    uint32_t pflashBlockBase = 0;
    uint32_t pflashTotalSize = 0;
    uint32_t pflashSectorSize = 0;

    /* Clean up Flash driver Structure*/
    memset(&s_flashDriver, 0, sizeof(flash_config_t));

    /* Setup flash driver structure for device and initialize variables. */
    result = FLASH_Init(&s_flashDriver);
    if (kStatus_FLASH_Success != result)
    {
    	context->printf_data_func("Error: Error during Flash Init\r\n");
    	trouble = result;  //TODO: translate.
    	goto Flash_Persist_Write_ERROR_HANDLER;
    }
    /* Get flash properties*/
    FLASH_GetProperty(&s_flashDriver, kFLASH_PropertyPflashBlockBaseAddr, &pflashBlockBase);
    FLASH_GetProperty(&s_flashDriver, kFLASH_PropertyPflashTotalSize, &pflashTotalSize);
    FLASH_GetProperty(&s_flashDriver, kFLASH_PropertyPflashSectorSize, &pflashSectorSize);

    /* Check security status. */
    result = FLASH_GetSecurityState(&s_flashDriver, &securityStatus);
    if (kStatus_FLASH_Success != result)
    {
    	context->printf_data_func("Error: Error during security state retrieval\r\n");
    	trouble = result;  //TODO: translate.
    	goto Flash_Persist_Write_ERROR_HANDLER;
    }
    /* Print security status. */
    switch (securityStatus)
    {
        case kFLASH_SecurityStateNotSecure:
        	context->printf_data_func("\r\n Flash is UNSECURE!");
            break;
        case kFLASH_SecurityStateBackdoorEnabled:
        	context->printf_data_func("\r\n Flash is SECURE, BACKDOOR is ENABLED!");
            break;
        case kFLASH_SecurityStateBackdoorDisabled:
        	context->printf_data_func("\r\n Flash is SECURE, BACKDOOR is DISABLED!");
            break;
        default:
            break;
    }
    context->printf_data_func("\r\n");

    /* Test pflash basic opeation only if flash is unsecure. */
    if (kFLASH_SecurityStateNotSecure == securityStatus)
    {
        /* Debug message for user. */
        /* Erase several sectors on upper pflash block where there is no code */
    	context->printf_data_func("\r\n****Erasing first sector of persistent flash region****\r\n");

/* Erase a sector from destAdrss. */
        result = FLASH_Erase(&s_flashDriver, 0x000FE000, pflashSectorSize, kFLASH_ApiEraseKey);
        if (kStatus_FLASH_Success != result)
        {
        	context->printf_data_func("Error: Error during flash erase\r\n");
        }

        /* Verify sector if it's been erased. */
        result = FLASH_VerifyErase(&s_flashDriver, 0x000FE000, pflashSectorSize, kFLASH_MarginValueUser);
        if (kStatus_FLASH_Success != result)
        {
        	context->printf_data_func("Error: Error during flash erase verify\r\n");
        }

        /* Print message for user. */
        context->printf_data_func("\r\n Successfully Erased Sector 0x%x -> 0x%x\r\n", 0x000FE000, (0x000FE000 + pflashSectorSize));

        /* Print message for user. */
        context->printf_data_func("\r\n****Program a buffer to a sector of flash****\r\n");
        /* Prepare user buffer. */
        //uFlashRecord.


        /* Program user buffer into flash*/
        result = FLASH_Program(&s_flashDriver, 0x000FE000, (uint32_t *)&s_persistRecord, sizeof(s_persistRecord));
        if (kStatus_FLASH_Success != result)
        {
        	context->printf_data_func("Error: Error during flash program\r\n");
        }

        /* Verify programming by Program Check command with user margin levels */
        result = FLASH_VerifyProgram(	&s_flashDriver,
        								0x000FE000,
        								sizeof(s_persistRecord),
										/* not sure if this is what you intend - but there was a warning */
										(const uint32_t *) &s_persistRecord, //MI: was (uint8_t *)&s_persistRecord,
										kFLASH_MarginValueUser,
										NULL,		/* optional fail address */
										NULL		/* optional fail data    */
									);
        if (kStatus_FLASH_Success != result)
        {
        	context->printf_data_func("Error: Error during flash program verify\r\n");
        }


        //s_persistRecordPtr = (persistRecord_t *)0x000FE000;

        //context->printf_data_func("\n\r Read Struct ID: 0x%x\n\r", s_persistRecordPtr->struct_id);
        //context->printf_data_func("\n\r Read Timestamp: %d\n\r", s_persistRecordPtr->timestamp);
        //context->printf_data_func("\n\r Read User String: %s . Is this correct?\n\r", s_persistRecordPtr->uString);
        //context->printf_data_func("\n\r Read Struct ID: 0x%x\n\r", s_persistRecordPtr->struct_id);

        context->printf_data_func("\r\n Successfully Programmed and Verified Location 0x%x -> 0x%x \r\n", 0x000FE000,
               (0x000FE000 + sizeof(s_persistRecord)));

        /* Erase the context we have progeammed before*/
        /* Note: we should make sure that the sector which will be set as swap indicator should be blank*/
        FLASH_Erase(&s_flashDriver, 0x000FE000, pflashSectorSize, kFLASH_ApiEraseKey);
    }
    else
    {
    	context->printf_data_func("\r\n Erase/Program operation will not be executed, as Flash is SECURE!\r\n");
    }
Flash_Persist_Write_ERROR_HANDLER:
	return trouble;
}

/*FUNCTION**********************************************************************
 * Description   :
 *END**************************************************************************/
int32_t Flash_Persist_Test_Garbage_Collection( p_shell_context_t context, int32_t argc, char ** argv )
{
	(void) argc;
	(void) argv;

	persistRecord_t my_record;
	static char my_data[1024] = {'a'};
	uint32_t filler_size = sizeof(my_data);

	int32_t get_result = 0;
	uint32_t my_tag = 0xDEADBEEF;

	get_result = Persist_Flash_Create_Record(&my_record, BUFFER_SHARED_TYPE__CHAR, my_tag, filler_size, (char *)&my_data);

	if(get_result < 0)
	{
		/*todo:: return more specific errors!!!!*/
		return -1;
	}

	context->printf_data_func("\r\n WRITING THE RECORD MULTIPLE TIMES TO TRIGGER GARBAGE COLLECTION\r\n");

	for(int x = 0; x < 25; x++)
	{
		get_result = Persist_Flash_Record_Commit(&my_record);

		if(get_result < 0)
		{
			context->printf_data_func("\r\n ERROR: WRITING THE RECORD FAILED \r\n");
		}
	}

	context->printf_data_func("\r\n TEST COMPLETE\r\n");

    return 0;
}
/*FUNCTION**********************************************************************
 * Description   :
 *END**************************************************************************/
int32_t Flash_Persist_Clear( p_shell_context_t context, int32_t argc, char ** argv )
{
	(void) argc;
	(void) argv;

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
    memset(&s_flashDriver, 0, sizeof(flash_config_t));

    /* Setup flash driver structure for device and initialize variables. */
    result = FLASH_Init(&s_flashDriver);
    if (kStatus_FLASH_Success != result)
    {
    	context->printf_data_func("Error: Error during Flash Init\r\n");
    }

    FLASH_GetProperty(&s_flashDriver, kFLASH_PropertyPflashSectorSize, &pflashSectorSize);

	/* Check security status. */
	result = FLASH_GetSecurityState(&s_flashDriver, &securityStatus);
	if (kStatus_FLASH_Success != result)
	{
		context->printf_data_func("Error: Error during security state retrieval\r\n");
		return -1;
	}

	/* we don't care about any arguments here */

	/* Erase the entire persistent section */
    if (kFLASH_SecurityStateNotSecure == securityStatus)
    {
        /* Debug message for user. */
        /* Erase several sectors on upper pflash block where there is no code */
    	context->printf_data_func("\r\n****Erasing persistent flash region****\r\n");

    	/* Erase Persistent section. */
        result = FLASH_Erase(&s_flashDriver, (uint32_t)PERSIST_FLASH_BASE, (uint32_t)(PERSIST_TOTAL_BANK_SIZE), kFLASH_ApiEraseKey);
        if (kStatus_FLASH_Success != result)
        {
        	context->printf_data_func("Error: Error during flash erase\r\n");
        	return -1;
        }

        /* Verify sector if it's been erased. */
        result = FLASH_VerifyErase(&s_flashDriver, (uint32_t)PERSIST_FLASH_BASE, (uint32_t)(PERSIST_TOTAL_BANK_SIZE), kFLASH_MarginValueUser);
        if (kStatus_FLASH_Success != result)
        {
        	context->printf_data_func("Error: Error during flash erase verify\r\n");
        	return -1;
        }

        /* Print message for user. */
        context->printf_data_func("\r\n Successfully Erased Sector 0x%x -> 0x%x\r\n", PERSIST_FLASH_BASE, (PERSIST_FLASH_BASE + (PERSIST_TOTAL_BANK_SIZE)));
    }
    else
    {
    	context->printf_data_func("\r\n Erase/Program operation will not be executed, as Flash is SECURE!\r\n");
    	return -1;
    }

    Persist_flash_initialize();

    return 0;
}

/*FUNCTION**********************************************************************
 * Description   :
 *END**************************************************************************/
//MI: changed signature to be compatible with command decode.
//uint32_t FLS_PERSIST_TEST(p_shell_context_t context, int32_t argc, char ** argv)
int32_t Flash_Persist_Test( p_shell_context_t context, int32_t argc, char ** argv )
{
	(void) argc;
	(void) argv;
#if(1)
	flash_config_t s_flashDriver;
/* TODO:  bust up function into smaller logical functions (some might be reusable?)
 * TODO:  error handling at bottom of function, add error codes.
 * TODO:  adding command arguments to indicate what is to be run.
 */

	/* tag */
	uint32_t my_tag = 15;
	/* erase persistent flash sectors */

	flash_security_state_t securityStatus = kFLASH_SecurityStateNotSecure; /* Return protection status */
	status_t result;    /* Return code from each flash driver function */
//	uint32_t destAdrss; /* Address of the target location */
//	uint32_t i, failAddr, failDat;
    uint32_t pflashSectorSize = 0;

    /* Clean up Flash driver Structure*/
    memset(&s_flashDriver, 0, sizeof(flash_config_t));

    /* Setup flash driver structure for device and initialize variables. */
    result = FLASH_Init(&s_flashDriver);
    if (kStatus_FLASH_Success != result)
    {
    	context->printf_data_func("Error: Error during Flash Init\r\n");
    }

    FLASH_GetProperty(&s_flashDriver, kFLASH_PropertyPflashSectorSize, &pflashSectorSize);

	/* Check security status. */
	result = FLASH_GetSecurityState(&s_flashDriver, &securityStatus);
	if (kStatus_FLASH_Success != result)
	{
		context->printf_data_func("Error: Error during security state retrieval\r\n");
		return -1;
	}

	/* we don't care about any arguments here */

	/* Erase the entire persistent section */
    if (kFLASH_SecurityStateNotSecure == securityStatus)
    {
        /* Debug message for user. */
        /* Erase several sectors on upper pflash block where there is no code */
    	context->printf_data_func("\r\n****Erasing persistent flash region****\r\n");

    	/* Erase Persistent section. */
        result = FLASH_Erase(&s_flashDriver, (uint32_t)PERSIST_FLASH_BASE, (uint32_t)(PERSIST_TOTAL_BANK_SIZE), kFLASH_ApiEraseKey);
        if (kStatus_FLASH_Success != result)
        {
        	context->printf_data_func("Error: Error during flash erase\r\n");
        	return -1;
        }

        /* Verify sector if it's been erased. */
        result = FLASH_VerifyErase(&s_flashDriver, (uint32_t)PERSIST_FLASH_BASE, (uint32_t)(PERSIST_TOTAL_BANK_SIZE), kFLASH_MarginValueUser);
        if (kStatus_FLASH_Success != result)
        {
        	context->printf_data_func("Error: Error during flash erase verify\r\n");
        	return -1;
        }

        /* Print message for user. */
        context->printf_data_func("\r\n Successfully Erased Sector 0x%x -> 0x%x\r\n", PERSIST_FLASH_BASE, (PERSIST_FLASH_BASE + (PERSIST_TOTAL_BANK_SIZE)));
    }
    else
    {
    	context->printf_data_func("\r\n Erase/Program operation will not be executed, as Flash is SECURE!\r\n");
    	return -1;
    }


    Persist_flash_initialize();

	/* check if valid record count returns 0 */
    persistRecord_t * test_pointer;
    void * opaque_test_pointer;
    int32_t test_count = 0;

    test_count = Persist_flash_get_valid_record_count(&opaque_test_pointer);
    if ( test_count < 0 )
    {
    	return -1; //TODO
    }
    else
    if(test_count != 0)
    {
    	context->printf_data_func("\r\n ERROR:: INVALID RECORD COUNT::**EXPECTED**: %d **ACTUAL**: %d!\r\n", 0, test_count);
    	return -1;
    }
    else
    if( opaque_test_pointer != NULL)
    {
    	context->printf_data_func("\r\n ERROR:: INVALID POINTER ON EMPTY BANK::**EXPECTED**: 0x%x **ACTUAL**: 0x%x!\r\n", NULL, opaque_test_pointer);
    	return -1;
    }

	/* check that get recent record gives a null pointer */
    apexTimestamp_t test_timestamp;
    test_pointer = Persist_Flash_Get_Recent_Record(my_tag, &test_timestamp);

    if(test_pointer != NULL)
    {
    	context->printf_data_func("\r\n ERROR:: INVALID POINTER ON EMPTY BANK::**EXPECTED**: 0x%x **ACTUAL**: 0x%x!\r\n", NULL, test_pointer);
    	return -1;
    }
    else
    if(test_timestamp.timestamp_configuration != 0 || test_timestamp.timestamp_value != 0)
    {
    	context->printf_data_func("\r\n ERROR:: TIMESTAMP CONFIGURATION ERROR::**EXPECTED**: %d **ACTUAL**: %d!\r\n", 0, test_timestamp.timestamp_configuration);
    	context->printf_data_func("\r\n ERROR:: TIMESTAMP VALUE ERROR::**EXPECTED**: %d **ACTUAL**: %d!\r\n", 0, test_timestamp.timestamp_value);

    	return -1;
    }

	/* check that get historical record gives a null pointer */
    uint32_t test_number_of_records = 0;
    test_pointer = Persist_flash_get_historical_record(my_tag, 1, &test_timestamp, &test_number_of_records);

    if(test_pointer != NULL)
    {
    	context->printf_data_func("\r\n ERROR:: INVALID POINTER ON EMPTY BANK::**EXPECTED**: 0x%x **ACTUAL**: 0x%x!\r\n", NULL, test_pointer);
    	return -1;
    }
    else
    if(test_timestamp.timestamp_configuration != 0 || test_timestamp.timestamp_value != 0)
    {
    	context->printf_data_func("\r\n ERROR:: TIMESTAMP CONFIGURATION ERROR::**EXPECTED**: %d **ACTUAL**: %d!\r\n", 0, test_timestamp.timestamp_configuration);
    	context->printf_data_func("\r\n ERROR:: TIMESTAMP VALUE ERROR::**EXPECTED**: %d **ACTUAL**: %d!\r\n", 0, test_timestamp.timestamp_value);

    	return -1;
    }
    else
    if(test_number_of_records != 0)
    {
    	context->printf_data_func("\r\n ERROR:: NUMBER OF RECORDS ERROR::**EXPECTED**: %d **ACTUAL**: %d!\r\n", NULL, test_number_of_records);
    	return -1;
    }

	/* check that get history returns you with no history */
    uint32_t record_list_request = 5;
    persistRecord_t * record_list[record_list_request];

   test_number_of_records = Persist_flash_get_record_history(my_tag, record_list_request, record_list);

   if(test_number_of_records != 0)
   {
   	context->printf_data_func("\r\n ERROR:: NUMBER OF RECORDS ERROR::**EXPECTED**: %d **ACTUAL**: %d!\r\n", NULL, test_number_of_records);
   	return -1;
   }

	/* create a single record */
   int32_t get_result = 0;
   persistRecord_t my_real_test_record;
   char my_string[] = "this is my string which is too long for the internal buffer\0";

   get_result = Persist_Flash_Create_Record(&my_real_test_record, BUFFER_SHARED_TYPE__CHAR, my_tag, sizeof(my_string), (void*)my_string);

   if(get_result < 0)
   {
	   /*todo:: return more specific errors!!!!*/
	   	context->printf_data_func("\r\n ERROR:: ERROR CREATING RECORD \r\n");
	   	return -1;
   }

   char my_new_string[] = "this is a string";
   get_result = Persist_Flash_Create_Record(&my_real_test_record, BUFFER_SHARED_TYPE__CHAR, my_tag, sizeof(my_new_string), (void*)my_new_string);

   if(get_result < 0)
   {
	   /*todo:: return more specific errors!!!!*/
	   	context->printf_data_func("\r\n ERROR:: ERROR CREATING RECORD -- RECORD SHOULD HAVE BEEN ACCEPTED\r\n");
	   	return -1;
   }

   /* check the parameters of the record */
   if(Persist_flash_record_valid(&my_real_test_record))
   {
	   /*todo:: return more specific errors!!!!*/
	   	context->printf_data_func("\r\n ERROR:: RECORD ERROR -- RETURNED VALID WHEN RECORD IS REALLY INVALID\r\n");
	   	return -1;
   }

   if(!Persist_flash_record_exists(&my_real_test_record))
   {
	   /*todo:: return more specific errors!!!!*/
	   	context->printf_data_func("\r\n ERROR:: RECORD ERROR -- RETURNED 'NOT EXIST' WHEN RECORD DOES EXIST\r\n");
	   	return -1;
   }

	/* commit a single record */
   get_result = Persist_Flash_Record_Commit(&my_real_test_record);

   if(get_result < 0)
   {
	   /*todo:: return more specific errors!!!!*/
	   	context->printf_data_func("\r\n ERROR:: RECORD COMMIT ERROR -- FAILED TO COMMIT\r\n");
	   	return -1;
   }

	/* check if valid record count returns 1 */

   test_count = Persist_flash_get_valid_record_count(&opaque_test_pointer);
   if ( test_count < 0 )
   { /* error occurred */
	   return -1;
   }
   else
   if(test_count != 1)
   {
   	context->printf_data_func("\r\n ERROR:: INVALID RECORD COUNT::**EXPECTED**: %d **ACTUAL**: %d!\r\n", 1, test_count);
   	return -1;
   }
   else
   if( opaque_test_pointer == NULL)
   {
   	context->printf_data_func("\r\n ERROR:: INVALID POINTER -- NULL POINTER RECEIEVED\r\n");
   	return -1;
   }
#if 0
   int32_t total_record_count = Persist_flash_get_total_record_count((void *)PERSIST_FLASH_FIRST_BANK_BASE, PERSIST_FIRST_BANK_SIZE);
   if ( total_record_count < 0 )
   { /* error occurred */
	   return -1;
   }
   else
   if ( test_count != total_record_count )
   {
	   	context->printf_data_func("\r\n ERROR:: RECORD COUNT DOES NOT EQUAL VALID COUNT WHEN EXPECTED::**#VALID**: %d **#RECORDS**: %d!\r\n", 1, test_count);
	   	return -1;
   }
#endif
	/* check if get record recent returns the record when requesting the type */


	/* check that get recent record gives a null pointer when fetching a tag that doesnt exist */
   /*apexTimestamp_t test_timestamp;*/
   test_pointer = Persist_Flash_Get_Recent_Record(my_tag + 1, &test_timestamp);

   if(test_pointer != NULL)
   {
   	context->printf_data_func("\r\n ERROR:: NULL POINTER RETURNED WHEN VALID POINTER EXPECTED\r\n", NULL, test_pointer);
   	return -1;
   }
   else
   if(test_timestamp.timestamp_configuration != 0 || test_timestamp.timestamp_value != 0)
   {
   	context->printf_data_func("\r\n ERROR:: TIMESTAMP CONFIGURATION ERROR::**EXPECTED**: %d **ACTUAL**: %d!\r\n", 0, test_timestamp.timestamp_configuration);
   	context->printf_data_func("\r\n ERROR:: TIMESTAMP VALUE ERROR::**EXPECTED**: %d **ACTUAL**: %d!\r\n", 0, test_timestamp.timestamp_value);

   	return -1;
   }

   /*now fetch a pointer that exists*/
   test_pointer = Persist_Flash_Get_Recent_Record(my_tag, &test_timestamp);

   if(test_pointer == NULL)
   {
   	context->printf_data_func("\r\n ERROR:: NULL POINTER RETURNED WHEN VALID POINTER EXPECTED\r\n", NULL, test_pointer);
   	return -1;
   }
   else
   if(test_timestamp.timestamp_configuration != 0 || test_timestamp.timestamp_value != 0)
   {
   	context->printf_data_func("\r\n ERROR:: TIMESTAMP CONFIGURATION ERROR::**EXPECTED**: %d **ACTUAL**: %d!\r\n", 0, test_timestamp.timestamp_configuration);
   	context->printf_data_func("\r\n ERROR:: TIMESTAMP VALUE ERROR::**EXPECTED**: %d **ACTUAL**: %d!\r\n", 0, test_timestamp.timestamp_value);

   	return -1;
   }
   else
   if(strcmp(my_new_string, test_pointer->container) != 0)
   {
	   	context->printf_data_func("\r\n ERROR:: STORED DATA ERROR::**EXPECTED**: %s **ACTUAL**: %s!\r\n", my_string, test_pointer->container);
	   	return -1;
   }


	/* check if get record historical returns null if we request all other types */
   uint32_t historical_number_of_records;
   test_pointer = Persist_flash_get_historical_record(my_tag, 1, &test_timestamp, &historical_number_of_records);

   if(test_pointer == NULL)
   {
   	context->printf_data_func("\r\n ERROR:: NULL POINTER RETURNED WHEN VALID POINTER EXPECTED\r\n", NULL, test_pointer);
   	return -1;
   }
   else
   if(test_timestamp.timestamp_configuration != 0 || test_timestamp.timestamp_value != 0)
   {
   	context->printf_data_func("\r\n ERROR:: TIMESTAMP CONFIGURATION ERROR::**EXPECTED**: %d **ACTUAL**: %d!\r\n", 0, test_timestamp.timestamp_configuration);
   	context->printf_data_func("\r\n ERROR:: TIMESTAMP VALUE ERROR::**EXPECTED**: %d **ACTUAL**: %d!\r\n", 0, test_timestamp.timestamp_value);

   	return -1;
   }
   else
   if(strcmp(my_new_string, test_pointer->container) != 0)
   {
	   	context->printf_data_func("\r\n ERROR:: STORED DATA ERROR::**EXPECTED**: %s **ACTUAL**: %s!\r\n", my_string, test_pointer->container);
	   	return -1;
   }
   else
   if(historical_number_of_records != 1)
   {
	   	context->printf_data_func("\r\n ERROR:: STORED DATA ERROR::**EXPECTED**: %d **ACTUAL**: %d!\r\n", 1, historical_number_of_records);
	   	return -1;
   }

   /* test getting a second historical record when it doesn't exist */
   test_pointer = Persist_flash_get_historical_record(my_tag, 10, &test_timestamp, &historical_number_of_records);

   if(test_pointer == NULL)
    {
    	context->printf_data_func("\r\n ERROR:: NULL POINTER RETURNED WHEN VALID POINTER EXPECTED\r\n", NULL, test_pointer);
    	return -1;
    }
    else
    if(test_timestamp.timestamp_configuration != 0 || test_timestamp.timestamp_value != 0)
    {
    	context->printf_data_func("\r\n ERROR:: TIMESTAMP CONFIGURATION ERROR::**EXPECTED**: %d **ACTUAL**: %d!\r\n", 0, test_timestamp.timestamp_configuration);
    	context->printf_data_func("\r\n ERROR:: TIMESTAMP VALUE ERROR::**EXPECTED**: %d **ACTUAL**: %d!\r\n", 0, test_timestamp.timestamp_value);

    	return -1;
    }
    else
    if(strcmp(my_new_string, test_pointer->container) != 0)
    {
 	   	context->printf_data_func("\r\n ERROR:: STORED DATA ERROR::**EXPECTED**: %s **ACTUAL**: %s!\r\n", my_string, test_pointer->container);
 	   	return -1;
    }
    else
    if(historical_number_of_records != 1)
    {
 	   	context->printf_data_func("\r\n ERROR:: STORED DATA ERROR::**EXPECTED**: %d **ACTUAL**: %d!\r\n", 1, historical_number_of_records);
 	   	return -1;
    }

   /* test getting a second historical record when it doesn't exist */
   test_pointer = Persist_flash_get_historical_record(my_tag + 5, 1, &test_timestamp, &historical_number_of_records);

   if(test_pointer != NULL)
   {
   	context->printf_data_func("\r\n ERROR:: VALID ADDRESS RETURNED WHEN NULL EXPECTED\r\n", NULL, test_pointer);
   	return -1;
   }

	/* check if we get a history with only one item when requesting more than one item */

   persistRecord_t * my_list_of_chars[10] = {NULL};
   get_result = Persist_flash_get_record_history(my_tag, sizeof(my_list_of_chars), my_list_of_chars);

   /* should get at least one result */
   if(get_result != 1)
   {
	   	context->printf_data_func("\r\n ERROR:: HISTORY RETRIEVAL ERROR -- INCORRECT NUMBER OF RECORDS::**EXPECTED**: %d **ACTUAL**: %d!\r\n", 1, get_result);
	   	return -1;
   }
   else
   {
	   /* otherwise we have at least one record */
	   if(strcmp(my_list_of_chars[0]->container, my_new_string) != 0)
	   {
		   	context->printf_data_func("\r\n ERROR:: RETURNED A HISTORY BUT DATA INCORRECT::**EXPECTED**: %s **ACTUAL**: %s!\r\n", my_new_string, my_list_of_chars[0]->container);
		   	return -1;
	   }
   }


	/* check if we get an empty history if we request with size 0 */

   my_list_of_chars[0] = NULL;
   get_result = Persist_flash_get_record_history(my_tag, 0, my_list_of_chars);

   /* should get at least one result */
   if(get_result != -1)
   {
	   	context->printf_data_func("\r\n ERROR:: HISTORY RETRIEVAL ERROR -- EXPECTED NO HISTORY REQUEST ERROR::**EXPECTED**: %d **ACTUAL**: %d!\r\n", -1, get_result);
	   	return -1;
   }
   else
   {
	   /* otherwise we have at least one record */
	   if(my_list_of_chars[0] != NULL)
	   {
		   	context->printf_data_func("\r\n ERROR:: EXPECTED NULL POINTER BUT GOT RETURNED AN ADDRESS\r\n");
		   	return -1;
	   }
   }

   /* check that we get returned 0 when requesting a type that doesn't exist*/

   get_result = Persist_flash_get_record_history(my_tag - 1, 10, my_list_of_chars);

   /* should get at least one result */
   if(get_result != 0)
   {
	   	context->printf_data_func("\r\n ERROR:: HISTORY RETRIEVAL ERROR -- INVALID NUMBER RECORDS RETURNED::**EXPECTED**: %d **ACTUAL**: %d!\r\n", 0, get_result);
	   	return -1;
   }
   else
   {
	   /* otherwise we have at least one record */
	   if(my_list_of_chars[0] != NULL)
	   {
		   	context->printf_data_func("\r\n ERROR:: EXPECTED NULL POINTER BUT GOT RETURNED AN ADDRESS\r\n");
		   	return -1;
	   }
   }


   /* erase flash again */
	/* Erase the entire persistent section */
   if (kFLASH_SecurityStateNotSecure == securityStatus)
   {
       /* Debug message for user. */
       /* Erase several sectors on upper pflash block where there is no code */
   	context->printf_data_func("\r\n****Erasing persistent flash region****\r\n");

   	/* Erase Persistent section. */
       result = FLASH_Erase(&s_flashDriver, (uint32_t)PERSIST_FLASH_BASE, (uint32_t)(PERSIST_TOTAL_BANK_SIZE), kFLASH_ApiEraseKey);
       if (kStatus_FLASH_Success != result)
       {
       	context->printf_data_func("Error: Error during flash erase\r\n");
       	return -1;
       }

       /* Verify sector if it's been erased. */
       result = FLASH_VerifyErase(&s_flashDriver, (uint32_t)PERSIST_FLASH_BASE, (uint32_t)(PERSIST_TOTAL_BANK_SIZE), kFLASH_MarginValueUser);
       if (kStatus_FLASH_Success != result)
       {
       	context->printf_data_func("Error: Error during flash erase verify\r\n");
       	return -1;
       }

       /* Print message for user. */
       context->printf_data_func("\r\n Successfully Erased Sector 0x%x -> 0x%x\r\n", PERSIST_FLASH_BASE, (PERSIST_FLASH_BASE + (PERSIST_TOTAL_BANK_SIZE)));

   }
   else
   {
   	context->printf_data_func("\r\n Erase/Program operation will not be executed, as Flash is SECURE!\r\n");
   	return -1;
   }

	/* write out a record for each type multiple times*/
//   persistRecord_t my_uint8_record;
   uint32_t my_uint8_tag = 10;

//   persistRecord_t my_char_record;
   uint32_t my_uint16_tag = 11;

//   persistRecord_t my_uint16_record;
   uint32_t my_char_tag = 12;

   char record_char_data = 'A';
   uint8_t record_uint8_data = 5;
   uint16_t record_uint16_data = 60000;

   uint32_t total_record_internal_count = 0;
   uint32_t actual_valid_record_count = 0;

   /* write 50 of each record */
   uint32_t max_records = 5;
   uint32_t record_write;
//   uint32_t record_index;
   int32_t commit_result;

   persistRecord_t temp;

   persistRecordHeader_t headers[15];
   uint64_t checksums[15];


   /* create all records */
   for(record_write = 0; record_write < 15; record_write = record_write + 3)
   {
	   /* commit uint8 record */
	   get_result = Persist_Flash_Create_Record(&temp, BUFFER_SHARED_TYPE__UINT8, my_uint8_tag, sizeof(record_uint8_data), (void*)&(record_uint8_data));

	   if(get_result < 0)
	   {
		   /*todo:: return more specific errors!!!!*/
		   	context->printf_data_func("\r\n ERROR:: ERROR CREATING RECORD -- RECORD SHOULD HAVE BEEN ACCEPTED\r\n");
		   	return -1;
	   }

	   /* commit uint8 record */
	   commit_result = Persist_Flash_Record_Commit(&temp);

	   if ( commit_result < 0 )
	   {
		   /*todo:: return more specific errors!!!!*/
		   	context->printf_data_func("\r\n ERROR:: RECORD COMMIT ERROR -- FAILED TO COMMIT\r\n");
		   	return -1;
	   }

	   /* copy over the header */
	   memcpy((void *)(&headers[record_write]), (void *)(&temp.persist_record_header), sizeof(persistRecordHeader_t));
	   memcpy((void *)(&checksums[record_write]), (void *)(&temp.checksum), sizeof(uint64_t));

	   /* commit uint16 record */

	   get_result = Persist_Flash_Create_Record(&temp, BUFFER_SHARED_TYPE__UINT16, my_uint16_tag, sizeof(record_uint16_data), (void*)&(record_uint16_data));

	   if(get_result < 0)
	   {
		   /*todo:: return more specific errors!!!!*/
		   	context->printf_data_func("\r\n ERROR:: ERROR CREATING RECORD -- RECORD SHOULD HAVE BEEN ACCEPTED\r\n");
		   	return -1;
	   }

	   /* commit uint16 record */
	   commit_result = Persist_Flash_Record_Commit(&temp);

	   if(commit_result < 0)
	   {
		   /*todo:: return more specific errors!!!!*/
		   	context->printf_data_func("\r\n ERROR:: RECORD COMMIT ERROR -- FAILED TO COMMIT\r\n");
		   	return -1;
	   }

	   memcpy((void *)(&headers[record_write + 1]), (void *)(&temp.persist_record_header), sizeof(persistRecordHeader_t));
	   memcpy((void *)(&checksums[record_write + 1]), (void *)(&temp.checksum), sizeof(uint64_t));


	   /* commit char record */
	   get_result = Persist_Flash_Create_Record(&temp, BUFFER_SHARED_TYPE__CHAR, my_char_tag, sizeof(record_char_data), (void*)&(record_char_data));

	   if(get_result < 0)
	   {
		   /*todo:: return more specific errors!!!!*/
		   	context->printf_data_func("\r\n ERROR:: ERROR CREATING RECORD -- RECORD SHOULD HAVE BEEN ACCEPTED\r\n");
		   	return -1;
	   }

	   commit_result = Persist_Flash_Record_Commit(&temp);

	   if(commit_result < 0)
	   {
		   /*todo:: return more specific errors!!!!*/
		   	context->printf_data_func("\r\n ERROR:: RECORD COMMIT ERROR -- FAILED TO COMMIT\r\n");
		   	return -1;
	   }

	   memcpy((void *)(&headers[record_write + 2]), (void *)(&temp.persist_record_header), sizeof(persistRecordHeader_t));
	   memcpy((void *)(&checksums[record_write + 2]), (void *)(&temp.checksum), sizeof(uint64_t));


	   total_record_internal_count += 3;
   }

	/* check if we still have a valid count */

   actual_valid_record_count = Persist_flash_get_valid_record_count(NULL);

   if(actual_valid_record_count != total_record_internal_count)
   {
	   /*todo:: return more specific errors!!!!*/
	   	context->printf_data_func("\r\n ERROR:: RECORD COMMIT ERROR -- **EXPECTED NUMBER OF RECORDS: %d **ACTUAL: %d\r\n", total_record_internal_count, actual_valid_record_count);
	   	return -1;
   }

	/* get recent record of each tag */
   uint32_t num_uint8_records;
//   uint32_t num_uin16_records;
//   uint32_t num_char_records;

   persistRecord_t * get_uint8_record;
//   persistRecord_t * get_uint16_record;
//   persistRecord_t * get_char_record;

   persistRecord_t fetched_record;


   get_uint8_record = Persist_Flash_Get_Recent_Record(my_uint8_tag, NULL);

   /* recreate record */
   get_result = Persist_Flash_Create_Record(&temp, BUFFER_SHARED_TYPE__UINT8, my_uint8_tag, sizeof(record_uint8_data), (void*)&(record_uint8_data));

   memcpy((void *)(&temp.persist_record_header), (void *)(&headers[get_uint8_record->persist_record_header.timestamp]), sizeof(persistRecordHeader_t));
   memcpy((void *)(&temp.checksum), (void *)(&checksums[get_uint8_record->persist_record_header.timestamp]), sizeof(uint64_t));


   if(get_result < 0)
   {
	   /*todo:: return more specific errors!!!!*/
	   	context->printf_data_func("\r\n ERROR:: ERROR CREATING RECORD -- RECORD SHOULD HAVE BEEN ACCEPTED\r\n");
	   	return -1;
   }

   /* copy the record back to ram */
   if(Persist_Flash_Record_Copy_To_RAM(&fetched_record, (void *)get_uint8_record) < 0)
   {
	   	context->printf_data_func("\r\n ERROR:: RECORD COPY ERROR -- **RECORD IN FLASH MUST NOT BE VALID**");
	   	return -1;
   }

   if(get_uint8_record == NULL)
   {
   	context->printf_data_func("\r\n ERROR:: NULL POINTER RETURNED WHEN VALID POINTER EXPECTED\r\n");
   	return -1;
   }
   else
   if(Persist_ram_compare_record(&fetched_record, &temp) != 0)
   {
	   /* the last record in the ram list should be the most recent written */
	   	context->printf_data_func("\r\n ERROR:: RECENT RECORD DOES NOT MATCH EXPECTED RECORD WRITTEN\r\n");
	   	return -1;
   }


   get_uint8_record = Persist_flash_get_historical_record(my_uint8_tag, 1, NULL, &num_uint8_records);

   /* copy the record back to ram */
   if(Persist_Flash_Record_Copy_To_RAM(&fetched_record, (void *)get_uint8_record) < 0)
   {
	   	context->printf_data_func("\r\n ERROR:: RECORD COPY ERROR -- **RECORD IN FLASH MUST NOT BE VALID**");
	   	return -1;
   }

   if(get_uint8_record == NULL)
   {
   	context->printf_data_func("\r\n ERROR:: NULL POINTER RETURNED WHEN VALID POINTER EXPECTED\r\n", NULL, test_pointer);
   	return -1;
   }
   else
   if(Persist_ram_compare_record(&fetched_record, &temp) != 0)
   {
	   /* the last record in the ram list should be the most recent written */
	   	context->printf_data_func("\r\n ERROR:: RECENT RECORD DOES NOT MATCH EXPECTED RECORD WRITTEN\r\n", NULL, test_pointer);
	   	return -1;
   }
   else
   if(num_uint8_records != max_records)
   {
	   /* the last record in the ram list should be the most recent written */
	   	context->printf_data_func("\r\n ERROR:: INCORRECT RECORD COUNT **EXPECTED: %d **ACTUAL: %d\r\n", max_records, num_uint8_records);
	   	return -1;
   }

   get_uint8_record = Persist_flash_get_historical_record(my_uint8_tag, 2, NULL, &num_uint8_records);

   /* recreate record */
   get_result = Persist_Flash_Create_Record(&temp, BUFFER_SHARED_TYPE__UINT8, my_uint8_tag, sizeof(record_uint8_data), (void*)&(record_uint8_data));

   if(get_result < 0)
   {
	   /*todo:: return more specific errors!!!!*/
	   	context->printf_data_func("\r\n ERROR:: ERROR CREATING RECORD -- RECORD SHOULD HAVE BEEN ACCEPTED\r\n");
	   	return -1;
   }

   memcpy((void *)(&temp.persist_record_header), (void *)(&headers[get_uint8_record->persist_record_header.timestamp]), sizeof(persistRecordHeader_t));
   memcpy((void *)(&temp.checksum), (void *)(&checksums[get_uint8_record->persist_record_header.timestamp]), sizeof(uint64_t));

   /* copy the record back to ram */
   if(Persist_Flash_Record_Copy_To_RAM(&fetched_record, (void *)get_uint8_record) < 0)
   {
	   	context->printf_data_func("\r\n ERROR:: RECORD COPY ERROR -- **RECORD IN FLASH MUST NOT BE VALID**");
	   	return -1;
   }

   if(get_uint8_record == NULL)
   {
   	context->printf_data_func("\r\n ERROR:: NULL POINTER RETURNED WHEN VALID POINTER EXPECTED\r\n", NULL, test_pointer);
   	return -1;
   }
   else
   if(Persist_ram_compare_record(&fetched_record, &temp) != 0)
   {
	   /* the last record in the ram list should be the most recent written */
	   	context->printf_data_func("\r\n ERROR:: RECENT RECORD DOES NOT MATCH EXPECTED RECORD WRITTEN\r\n", NULL, test_pointer);
	   	return -1;
   }
   else
   if(num_uint8_records != max_records)
   {
	   /* the last record in the ram list should be the most recent written */
	   	context->printf_data_func("\r\n ERROR:: INCORRECT RECORD COUNT **EXPECTED: %d **ACTUAL: %d\r\n", max_records, num_uint8_records);
	   	return -1;
   }

   uint32_t history = 10;
   persistRecord_t * uint16_history[10];
   uint32_t number_records_returned_in_list;
   number_records_returned_in_list = Persist_flash_get_record_history(my_uint16_tag, history, uint16_history);

   get_result = Persist_Flash_Create_Record(&temp, BUFFER_SHARED_TYPE__UINT16, my_uint16_tag, sizeof(record_uint16_data), (void*)&(record_uint16_data));

   if(get_result < 0)
   {
	   /*todo:: return more specific errors!!!!*/
	   	context->printf_data_func("\r\n ERROR:: ERROR CREATING RECORD -- RECORD SHOULD HAVE BEEN ACCEPTED\r\n");
	   	return -1;
   }

   if(number_records_returned_in_list != max_records)
   {
	   	context->printf_data_func("\r\n ERROR:: INCORRECT HISTORY RECORD COUNT **EXPECTED: %d **ACTUAL: %d\r\n", max_records, number_records_returned_in_list);
	   	return -1;
   }
   else
   {
	   uint32_t record_history_index;
	   for(record_history_index = 0; record_history_index < number_records_returned_in_list; record_history_index++)
	   {
		   /* copy the record back to ram */
		   if(Persist_Flash_Record_Copy_To_RAM(&fetched_record, (void *)uint16_history[record_history_index]) < 0)
		   {
			   	context->printf_data_func("\r\n ERROR:: RECORD COPY ERROR -- **RECORD IN FLASH MUST NOT BE VALID**");
			   	return -1;
		   }

		   /* recreate record */
		   get_result = Persist_Flash_Create_Record(&temp, BUFFER_SHARED_TYPE__UINT16, my_uint16_tag, sizeof(record_uint16_data), (void*)&(record_uint16_data));

		   if(get_result < 0)
		   {
			   /*todo:: return more specific errors!!!!*/
			   	context->printf_data_func("\r\n ERROR:: ERROR CREATING RECORD -- RECORD SHOULD HAVE BEEN ACCEPTED\r\n");
			   	return -1;
		   }

		   memcpy((void *)(&temp.persist_record_header), (void *)(&(headers[fetched_record.persist_record_header.timestamp])), sizeof(persistRecordHeader_t));
		   memcpy((void *)(&temp.checksum), (void *)(&(checksums[fetched_record.persist_record_header.timestamp])), sizeof(uint64_t));

		   if(Persist_ram_compare_record(&temp, &fetched_record) != 0)
		   {
			   /* the last record in the ram list should be the most recent written */
			   	context->printf_data_func("\r\n ERROR:: RETURNED HISTORY LIST DOESN'T MATCH WRITTEN LIST\r\n");
			   	return -1;
		   }
	   }
   }

  	return SUCCESS;

	/* get the historical record for a record type of historical value*/

   /*  get the most recent record */

	/* write a bunch of invalid records to flash */

	/* write a new valid record */

	/* see if we return a proper valid count */

	/* see if we return a proper record count (invalid + valid) */

	/* todo:: test out initialization */
#endif
	/* test out forcing a cleanup with history minimums/maximum declarations */
//TODO:  put the error handler here at the bottom of the function
//   return SUCCESS;
//Flash_Persist_Test_ERROR_HANDLER:
// TODO: log error to error buffer log here
//	return trouble;  // will be posted to the context in the shell portion of the code.


}

/*FUNCTION*********************************************************************
 * Description   :
 *END**************************************************************************/


#define CMD_FLASH_COMMAND_LIST(_) \
/*  name	   		help	  																				function  	 num parameters*/ \
_( "persist_test",		"persist_test     : thorough persistence testing",								Flash_Persist_Test,  	0 ) \
_( "persist_clear",		"persist_clear    : clear persistence region",								 	Flash_Persist_Clear,  	0 ) \
_( "persist_gc",		"persist_gc       : force garbage collection to happen",						Flash_Persist_Test_Garbage_Collection,  	0 ) \
_(  NULL,     	NULL,	       																			NULL,			0 ) \




#if ( 1 == FSL_SHELL_USR_USE_PARAMETER_COUNT )
# define CMD_FLASH_EXTRACT_COMMANDS(cmd_string,help_string,callback,expected_no_params)  				{ cmd_string, help_string, callback, expected_no_params },
#else
# define CMD_FLASH_EXTRACT_COMMANDS(cmd_string,help_string,callback,expected_no_params)  				{ cmd_string, help_string, callback	 					},
#endif

/******************************************************************************
* VARIABLES
******************************************************************************/
static const shell_command_context_t  cmd_flash_command_list[] =
{
		CMD_FLASH_COMMAND_LIST( CMD_FLASH_EXTRACT_COMMANDS )
};


/*FUNCTION**********************************************************************
 * Description   :
 *END**************************************************************************/
void CMD_Flash_Shell_Bind( p_shell_context_t context )
{
#if ( 1 == FSL_SHELL_USR_DEFINE_COMMAND_LIST_TABLE )
	size_t index = 0;
	do /* we assume at least one entry!! */
	{
		++index;
	} while ( NULL != cmd_flash_command_list[index].pcCommand );

	Shell_Register_Command_List( context, index, cmd_flash_command_list );


#else
	int index = 0;
	while( NULL != cmd_flash_command_list[index].pcCommand )
	{
		Shell_RegisterCommand( context, &(cmd_flash_command_list[index]) );
		++index;
	}
#endif
}
