

#ifndef _FLASH_USER_COMMANDS_H_
#define _FLASH_USER_COMMANDS_H_  1

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
 *
 * *****************************************************************************/


/*MACRO**********************************************************************
 * Description   : The Subversion (SVN) time/date marker which is updated
 * during commit of the source file to the repository.
 *END**************************************************************************/
#define FLASH_USER_COMMANDS_H_SVN_REV  "$Date: 2017-04-21 16:15:27 -0500 (Fri, 21 Apr 2017) $"


/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @name Command API
 * @{
 */


#if 0
/*!
 * @brief
 *
 */
uint32_t FLASH_CFG_DMP( p_shell_context_t context, int32_t argc, char **argv );

/*!
 * @brief
 *
 */
uint32_t FLS_TEST_WRITE(p_shell_context_t context, int32_t argc, char ** argv);

/*!
 * @brief
 *
 */
uint32_t FLS_PERSIST_WR(p_shell_context_t context, int32_t argc, char ** argv);

/*!
 * @brief
 *
 */
uint32_t FLS_PERSIST_TEST(p_shell_context_t context, int32_t argc, char ** argv);

#else

/*!
 * @brief
 *
 */
int32_t Flash_Configuration_Dump( p_shell_context_t context, int32_t argc, char **argv );

/*!
 * @brief
 *
 */
int32_t Flash_Test_Write(p_shell_context_t context, int32_t argc, char ** argv);

/*!
 * @brief
 *
 */
int32_t Flash_Persist_Write(p_shell_context_t context, int32_t argc, char ** argv);

/*!
 * @brief
 *
 */
int32_t Flash_Persist_Test(p_shell_context_t context, int32_t argc, char ** argv);

/*!
 * @brief
 *
 */
int32_t Flash_Persist_Clear( p_shell_context_t context, int32_t argc, char ** argv );

/*!
 * @brief
 *
 */
void CMD_Flash_Shell_Bind( p_shell_context_t context );

#endif


/*! @}*/



#if defined(__cplusplus)
}
#endif



#endif /* _FLASH_USER_COMMANDS_H_ */
