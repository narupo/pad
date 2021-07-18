#pragma once

// TODO: test

#include <stdint.h>

#include <pad/lib/memory.h>
#include <pad/lib/string.h>
#include <pad/lib/unicode.h>
#include <pad/lib/dict.h>
#include <pad/core/alias_info.h>
#include <pad/lang/types.h>
#include <pad/lang/object_dict.h>
#include <pad/lang/scope.h>
#include <pad/lang/gc.h>

typedef enum {
    PAD_CTX_TYPE__DEFAULT,
    PAD_CTX_TYPE__DEF_STRUCT,
    PAD_CTX_TYPE__OBJECT,
    PAD_CTX_TYPE__MODULE,
} PadCtxType;

struct PadCtx {
    PadCtxType type;

    // ref_prevにはコンテキストをつなげたい時に、親のコンテキストを設定する
    // contextはこのref_prevを使い親のコンテキストを辿れるようになっている
    // これによってルートのコンテキストや1つ前のコンテキストを辿れる
    PadCtx *ref_prev;  // reference to previous context

    PadGC *ref_gc;  // reference to gc (DO NOT DELETE)
    PadAliasInfo *alinfo;  // alias info for builtin alias module

    // ルートのcontextのstdout_buf, stderr_bufにputsなどの組み込み関数の出力が保存される
    // その他ref_blockやtext_blockなどの出力もルートのcontextに保存されるようになっている
    // 2020/10/06以前はコンテキストごとにputsの出力を保存していた
    PadStr *stdout_buf;  // stdout buffer in context
    PadStr *stderr_buf;  // stderr buffer in context

    // コンテキストはスコープを管理する
    // 関数などのブロックに入るとスコープがプッシュされ、関数のスコープになる
    // 関数から出るとこのスコープがポップされ、スコープから出る
    PadScope *scope;  // scope in context

    bool do_break;  // if do break from current context then store true
    bool do_continue;  // if do continue on current context then store
    bool do_return;
    bool is_use_buf;  // if true then context use stdout/stderr buffer
};

/**
 * destruct PadObj
 *
 * @param[in] *self pointer to PadCtx
 *
 * @return
 */
void
PadCtx_Del(PadCtx *self);

/**
 * destruct PadObj and escape global variable map from context
 *
 * @param[in] *self
 *
 * @return pointer to PadObjDict
 */
PadObjDict *
PadCtx_EscDelGlobalVarmap(PadCtx *self);

/**
 * construct PadObj
 *
 * @param[in|out] *gc reference to PadGC (DO NOT DELETE)
 * @return pointer to PadCtx dynamic allocate memory (do PadCtx_Del)
 */
PadCtx *
PadCtx_New(PadGC *ref_gc, PadCtxType type);

/**
 * clear state of context
 *
 * @param[in] *self pointer to PadCtx
 */
void
PadCtx_Clear(PadCtx *self);

/**
 * set alias value and description at element of key
 *
 * @param[in] *self  pointer to PadCtx
 * @param[in] *key   key value strings
 * @param[in] *value value strings
 * @param[in] *desc  description strings
 *
 * @return success to pointer to self
 * @return failed to pointer to NULL
 */
PadCtx *
PadCtx_SetAlias(PadCtx *self, const char *key, const char *value, const char *desc);

void
PadCtx_SetDefGlobalVars(PadCtx *self);

/**
 * get alias value of key
 *
 * @param[in] *self pointer to PadCtx
 * @param[in] *key  key of alias
 *
 * @return found to pointer to value strings
 * @return not found to pointer to NULL
 */
const char *
PadCtx_GetAliasValue(PadCtx *self, const char *key);

/**
 * get alias description value of key
 *
 * @param[in] *self pointer to PadCtx
 * @param[in] *key  key of alias
 *
 * @return found to pointer to description strings
 * @return not found to pointer to NULL
 */
const char *
PadCtx_GetAliasDesc(PadCtx *self, const char *key);

/**
 * push back strings at stdout buffer in context
 *
 * @param[in] *self pointer to PadCtx
 * @param[in] *str  pointer to strings
 *
 * @return success to pointer to PadCtx
 * @return failed to pointer to NULL
 */
PadCtx *
PadCtx_PushBackStdoutBuf(PadCtx *self, const char *str);

/**
 * push back strings at stdout buffer in context
 *
 * @param[in] *self pointer to PadCtx
 * @param[in] *str  pointer to strings
 *
 * @return success to pointer to PadCtx
 * @return failed to pointer to NULL
 */
PadCtx *
PadCtx_PushBackStderrBuf(PadCtx *self, const char *str);

/**
 * get stdout buffer read-only
 *
 * @param[in] *self pointer to PadCtx
 *
 * @return pointer to buffer
 */
const char *
PadCtx_GetcStdoutBuf(const PadCtx *self);

/**
 * get stderr buffer read-only
 *
 * @param[in] *self pointer to PadCtx
 *
 * @return pointer to buffer
 */
const char *
PadCtx_GetcStderrBuf(const PadCtx *self);

/**
 * get alinfo read-only
 *
 * @param[in] *self pointer to PadCtx
 *
 * @return pointer to PadAliasInfo read-only
 */
const PadAliasInfo *
PadCtx_GetcAliasInfo(const PadCtx *self);

/**
 * get variables map as PadObjDict from current scope
 *
 * @param[in] *self pointer to PadCtx
 *
 * @return pointer to PadObjDict
 */
PadObjDict *
PadCtx_GetVarmapAtCurScope(PadCtx *self);

PadObjDict *
PadCtx_GetVarmapAtGlobal(PadCtx *self);

/**
 * get variables map as PadObjDict from global scope
 *
 * @param[in] *self pointer to PadCtx
 *
 * @return pointer to PadObjDict
 */
PadObjDict *
PadCtx_GetVarmapAtHeadScope(PadCtx *self);

/**
 * get do-break flag
 *
 * @param[in] *self pointer to PadCtx
 *
 * @return true or false
 */
bool
PadCtx_GetDoBreak(const PadCtx *self);

/**
 * set do-break flag
 *
 * @param[in] *self    pointer to PadCtx
 * @param[in] do_break value of flag
 */
void
PadCtx_SetDoBreak(PadCtx *self, bool do_break);

/**
 * get do-continue flag
 *
 * @param[in] *self pointer to PadCtx
 *
 * @return true or false
 */
bool
PadCtx_GetDoContinue(const PadCtx *self);

/**
 * set do-continue flag
 *
 * @param[in] *self       pointer to PadCtx
 * @param[in] do_continue value of flag
 */
void
PadCtx_SetDoContinue(PadCtx *self, bool do_continue);

/**
 * get do-return flag
 *
 * @param[in] *self pointer to PadCtx
 *
 * @return true or false
 */
bool
PadCtx_GetDoReturn(const PadCtx *self);

/**
 * set do-return flag
 *
 * @param[in] *self     pointer to PadCtx
 * @param[in] do_return value of flag
 */
void
PadCtx_SetDoReturn(PadCtx *self, bool do_return);

/**
 * clear do-break, do-continue, do-return flag
 *
 * @param[in] *self pointer to PadCtx
 */
void
PadCtx_ClearJumpFlags(PadCtx *self);

/**
 * push back scope at tail of scope chain
 *
 * @param[in] *self pointer to PadCtx
 */
void
PadCtx_PushBackScope(PadCtx *self);

/**
 * pop back scope from tail of scope chain
 *
 * @param[in] *self pointer to PadCtx
 */
void
PadCtx_PopBackScope(PadCtx *self);

bool
PadCtx_CurScopeHasGlobalName(PadCtx *self, const char *key);

/**
 * find variable from varmap of scope at tail to head in scope chain
 *
 * @param[in] *self pointer to PadCtx
 * @param[in] *key  key strings
 *
 * @return found to poitner to PadObj
 * @return not found to pointer to NULL
 */
PadObj *
PadCtx_FindVarRef(PadCtx *self, const char *key);

PadObj *
PadCtx_FindVarRefAtGlobal(PadCtx *self, const char *key);

/**
 * find variable from varmap of scope at tail to head in scope chain
 * traverse previous context on find
 *
 * @param[in] *self pointer to PadCtx
 * @param[in] *key  key strings
 *
 * @return found to poitner to PadObj
 * @return not found to pointer to NULL
 */
PadObj *
PadCtx_FindVarRefAll(PadCtx *self, const char *key);

PadObj *
PadCtx_FindVarRefAllIgnoreStructHead(PadCtx *self, const char *key);

/**
 * get reference of PadGC in context
 *
 * @param[in] *self pointer to PadCtx
 *
 * @return reference to PadGC (this is reference, DO NOT DELETE)
 */
PadGC *
PadCtx_GetGC(PadCtx *self);

PadScope *
PadCtx_GetCurScope(PadCtx *self);

/**
 * clear stdout buffer
 *
 * @param[in] *self pointer to PadCtx
 */
void
PadCtx_ClearStdoutBuf(PadCtx *self);

/**
 * clear stderr buffer
 *
 * @param[in] *self pointer to PadCtx
 */
void
PadCtx_ClearStderrBuf(PadCtx *self);

/**
 * swap stdout buffer
 *
 * @param[in] *self
 * @param[in] *stdout_buf pointer to set stdout buffer
 *
 * @return pointer to stdout buffer of swapped
 */
PadStr *
PadCtx_SwapStdoutBuf(PadCtx *self, PadStr *stdout_buf);

/**
 * swap stderr buffer
 *
 * @param[in] *self
 * @param[in] *stderr_buf pointer to set stdout buffer
 *
 * @return pointer to stderr buffer of swapped
 */
PadStr *
PadCtx_SwapStderrBuf(PadCtx *self, PadStr *stderr_buf);

/**
 * dump PadCtx at stream
 *
 * @param[in] *self
 * @param[in] *fout stream
 */
void
PadCtx_Dump(const PadCtx *self, FILE *fout);

/**
 * if idn in current scope then return true else return false
 *
 * @param[in] *self
 *
 * @return true or false
 */
bool
PadCtx_VarInCurScope(const PadCtx *self, const char *idn);

/**
 * get reference of varmap from current scope
 *
 * @param[in] *self
 *
 * @return reference to varmap
 */
PadObjDict *
PadCtx_GetRefVarmapCurScope(const PadCtx *self);

/**
 * pop last newline of stdout buf
 * 
 * @return 
 */
void
PadCtx_PopNewlineOfStdoutBuf(PadCtx *self);

void
PadCtx_SetRefPrev(PadCtx *self, PadCtx *ref_prev);

PadCtx *
PadCtx_GetRefPrev(const PadCtx *self);

PadCtx *
PadCtx_FindMostPrev(PadCtx *self);

PadCtx *
PadCtx_DeepCopy(const PadCtx *other);

PadCtx *
PadCtx_ShallowCopy(const PadCtx *other);

PadCtx *
PadCtx_UnpackObjAryToCurScope(PadCtx *self, PadObjAry *arr);

void
PadCtx_SetUseBuf(PadCtx *self, bool is_use_buf);

bool
PadCtx_GetIsUseBuf(const PadCtx *self);

void
PadCtx_SetType(PadCtx *self, PadCtxType type);

PadCtxType
PadCtx_GetType(const PadCtx *self);
