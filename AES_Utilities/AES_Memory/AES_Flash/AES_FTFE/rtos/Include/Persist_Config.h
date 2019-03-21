/*
 * Persist_Config.h
 *
 *  Created on: Nov 28, 2018
 *      Author: AES_Local
 */

#ifndef INCLUDE_PERSIST_CONFIG_H_
#define INCLUDE_PERSIST_CONFIG_H_

/***********************************************************************************************************************************************************
 *                                                             Queue Sizing Definitions
 *
 *
 *
 *
 ************************************************************************************************************************************************************/
#define PERSIST_WRITE_QUEUE_BUFFER_SIZE     1024 /* bytes */
#define PERSIST_COMMIT_QUEUE_BUFFER_SIZE    512  /* bytes */
#define PERSIST_BANK_THRESHOLD_RATIO        .75  /* percent full before garbage collection takes place */


#endif /* INCLUDE_PERSIST_CONFIG_H_ */
