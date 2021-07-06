#pragma once
#include <stdint.h>

struct PadObjDict;
typedef struct PadObjDict PadObjDict;

struct PadObjAry;
typedef struct PadObjAry PadObjAry;

struct PadObj;
typedef struct PadObj PadObj;

struct PadFuncObj;
typedef struct PadFuncObj PadFuncObj;

struct PadIndexObj;
typedef struct PadIndexObj PadIndexObj;

struct PadModObj;
typedef struct PadModObj PadModObj;

struct PadIdentObj;
typedef struct PadIdentObj PadIdentObj;

struct PadDefStructObj;
typedef struct PadDefStructObj PadDefStructObj;

struct PadObjObj;
typedef struct PadObjObj PadObjObj;

struct PadRingObj;
typedef struct PadRingObj PadRingObj;

struct PadOwnsMethodObj;
typedef struct PadOwnsMethodObj PadOwnsMethodObj;

struct PadTypeObj;
typedef struct PadTypeObj PadTypeObj;

struct PadBltFuncObj;
typedef struct PadBltFuncObj PadBltFuncObj;

struct PadNodeAry;
typedef struct PadNodeAry PadNodeAry;

struct PadNodeDict;
typedef struct PadNodeDict PadNodeDict;

struct PadTkr;
typedef struct PadTkr PadTkr;

struct PadCtx;
typedef struct PadCtx PadCtx;

struct PadScope;
typedef struct PadScope PadScope;

struct PadAST;
typedef struct PadAST PadAST;

struct PadCcArgs;
typedef struct PadCcArgs PadCcArgs;

struct PadTrvArgs;
typedef struct PadTrvArgs PadTrvArgs;

struct PadBltFuncArgs;
typedef struct PadBltFuncArgs PadBltFuncArgs;

typedef PadObj *(*PadBltFunc)(PadBltFuncArgs *args);

typedef struct PadBltFuncInfo PadBltFuncInfo;

struct PadGC;
typedef struct PadGC PadGC;

struct PadGCItem;
typedef struct PadGCItem PadGCItem;

typedef long PadIntObj;
typedef double PadFloatObj;

// number of depth type for function recursion
typedef int32_t PadDepth;

struct PadChainNode;
typedef struct PadChainNode PadChainNode;

struct PadChainNodes;
typedef struct PadChainNodes PadChainNodes;

struct PadChainObj;
typedef struct PadChainObj PadChainObj;

struct PadChainObjs;
typedef struct PadChainObjs PadChainObjs;

struct PadNode;
typedef struct PadNode PadNode;

struct PadKit;
typedef struct PadKit PadKit;

struct PadImporter;
typedef struct PadImporter PadImporter;

typedef char *(* PadImporterFixPathFunc)(PadImporter *, char *, int32_t, const char *);
