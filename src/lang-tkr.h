/**
 * Description.
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2017
 */
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

/******
* tok *
******/

struct cap_ltkrtok;

/**
 * 
 *
 * @param[in]  *self 
 */
void 
cap_ltkrtokdel(struct cap_ltkrtok *self);

/**
 * 
 *
 * @param[in]  capa 
 *
 * @return 
 */
struct cap_ltkrtok * 
cap_ltkrtoknewcapa(int32_t capa);

/**
 * 
 *
 * @return 
 */
struct cap_ltkrtok * 
cap_ltkrtoknew(void);

/**
 * 
 *
 * @param[in]  *str 
 *
 * @return 
 */
struct cap_ltkrtok * 
cap_ltkrtoknewstr(const uint8_t *str);

struct cap_ltkrtok *
cap_ltkrtokresize(struct cap_ltkrtok *self, int32_t recapa);

struct cap_ltkrtok *
cap_ltkrtokpush(struct cap_ltkrtok *self, uint8_t c);

const uint8_t *
cap_ltkrtokgetc(const struct cap_ltkrtok *self);

void
cap_ltkrtokclear(struct cap_ltkrtok *self);

/*******
* toks *
*******/

struct cap_ltkrtoks;

/**
 * 
 *
 * @param[in]  *self 
 */
void 
cap_ltkrtoksdel(struct cap_ltkrtoks *self);

/**
 * 
 *
 * @param[in]  capa 
 *
 * @return 
 */
struct cap_ltkrtoks * 
cap_ltkrtoksnewcapa(int32_t capa);

/**
 * 
 *
 * @return 
 */
struct cap_ltkrtoks * 
cap_ltkrtoksnew(void);

/**
 * 
 *
 * @param[in]  *self  
 * @param[in]  recapa 
 *
 * @return 
 */
struct cap_ltkrtoks * 
cap_ltkrtoksrecapa(struct cap_ltkrtoks *self, int32_t recapa);

/**
 * 
 *
 * @param[in]  *self 
 * @param[in]  *tok  
 *
 * @return 
 */
struct cap_ltkrtoks * 
cap_ltkrtoksmove(struct cap_ltkrtoks *self, struct cap_ltkrtok *tok);

/**
 * 
 *
 * @param[in]  *self 
 * @param[in]  idx   
 *
 * @return 
 */
const struct cap_ltkrtok * 
cap_ltkrtoksgetc(const struct cap_ltkrtoks *self, int32_t idx);

int32_t
cap_ltkrtokslen(const struct cap_ltkrtoks *self);

/******
* tkr *
******/

struct cap_ltkr;

/**
 * 
 *
 * @param[in]  *self 
 */
void 
cap_ltkrdel(struct cap_ltkr *self);

/**
 * 
 *
 * @return 
 */
struct cap_ltkr * 
cap_ltkrnew(void);

/**
 * 
 *
 * @param[in]  *self 
 * @param[in]  *fin  
 *
 * @return 
 */
struct cap_ltkr * 
cap_ltkrparsestream(struct cap_ltkr *self, FILE *fin);

