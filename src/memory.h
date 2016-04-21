#ifndef MEMORY_H
#define MEMORY_H

#include "define.h"
#include "util.h"

/**
 * Wrapper of free(3)
 * 
 * @param[in] ptr 
 *                
 * @return        
 */
void
mem_free(void* ptr);

/**
 * Wrapper of malloc(3)
 * 
 * @param[in] size 
 *                 
 * @return         
 */
void*
mem_malloc(size_t size);

/**
 * Wrapper of calloc(3)
 * 
 * @param[in] nmemb 
 * @param[in] size  
 *                  
 * @return          
 */
void*
mem_calloc(size_t nmemb, size_t size);

/**
 * Wrapper of realloc(3)
 * 
 * @param[in] ptr  
 * @param[in] size 
 *                 
 * @return         
 */
void*
mem_realloc(void* ptr, size_t size);

/**
 * Wrapper of malloc(3)
 * If error then die
 * 
 * @param[in] size 
 *                 
 * @return         
 */
void*
mem_emalloc(size_t size);

/**
 * Wrapper of calloc(3)
 * If error then die
 * 
 * @param[in] nmemb 
 * @param[in] size  
 *                  
 * @return          
 */
void*
mem_ecalloc(size_t nmemb, size_t size);

/**
 * Wrapper of realloc(3)
 * If error then die
 * 
 * @param[in] ptr  
 * @param[in] size 
 *                 
 * @return         
 */
void*
mem_erealloc(void* ptr, size_t size);

#endif
