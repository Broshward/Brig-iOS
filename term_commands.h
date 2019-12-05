/*
 * =====================================================================================
 *
 *       Filename:  term_commands.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  11.04.2013 16:41:28
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *        Company:  
 *
 * =====================================================================================
 */

//

#define WRITE_ARRAY 0b11000000
#define  READ_ARRAY 0b01000000
#define WRITE_UINT8_T 10 //write uint8_t
// Example:
// dev_addr,WRITE_UINT8_T,num_bytes,(uint8_t[4])addr,(uint8_t)data[1], CS
//	    12 ,          10 ,       5 ,     0xXXXXXXXX ,            XX  , CS
//
#define WRITE_UINT16_T 11 //write uint16_t
// Example:
// dev_addr,WRITE_UINT8_T,num_bytes,(uint8_t[4])addr,(uint8_t)data[2], CS
//	    12 ,          11 ,       5 ,     0xXXXXXXXX ,          XX,XX , CS
//
#define WRITE_UINT32_T 13 //write uint32_t
#define WRITE_UINT8_T_ARRAY 14 //write array of uint32_t (32 to 32)
#define WRITE_UINT32_T_ARRAY 15 //write array of uint32_t (32 to 32)


#define READ_UINT8_T -10 //read uint8_t
// Example:
// dev_addr,READ_UINT8_T,num_bytes,(uint8_t[4])addr,CS
//      12 ,        -10 ,       4 ,     0xXXXXXXXX ,CS
//
#define READ_UINT16_T -11 //read uint16_t
// Example:
// dev_addr,READ_UINT8_T,num_bytes,(uint8_t[4])addr,CS
//      12 ,        -11 ,       4 ,     0xXXXXXXXX ,CS
//
#define READ_UINT32_T -13 //read uint32_t
#define READ_UINT8_T_ARRAY -14 //read array of uint8_t
#define READ_UINT32_T_ARRAY -15 //read array of uint8_t
