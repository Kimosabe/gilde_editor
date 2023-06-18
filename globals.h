#pragma once

#include <assert.h>

#pragma pack(push, 1)
#include "ghidra.h"
#pragma pack(pop)

enum LANG {
    RU,
    DE,

    LANG_SIZE // Must be the last
};

enum GLOBALS {
    OPEN_ENGINE_ADDR,
    CALL_TO_OPEN_ENGINE_INSIDE_MAIN,
    WIN_MESSAGE_LOOOP_ADDR,
    CALL_TO_WIN_MESSAGE_LOOP_INSIDE_MASTER,
    FIND_FREE_LINKED_LIST_NODE,
    GET_PRODUCTION_INFO,
    FIND_BY_OBJECT_ID,
    G_OBJECTS_COUNT,
    G_OBJECTS_PROTOTYPES,
    G_CHARACTERS,
    G_MARKET_PTR,
    G_CURRENT_BUILDING_PTR,
    G_CURRENT_BUILDING_PTR_2,
    G_CURRENT_ROOM_PTR,
    G_CHAR_580_1,
    G_CHAR_580_2,

    GLOBALS_SIZE // Must be the last
};

extern size_t g_globals[GLOBALS_SIZE][LANG_SIZE];

extern LANG g_lang;
#define _G(key) g_globals[(key)][g_lang]

//----------------------------------------------------------------------------


typedef LinkedList *(*__FindFreeLinkedListNode)();
extern __FindFreeLinkedListNode FindFreeLinkedListNode;

typedef ProductionInfo *(*FN_get_production_info_005b1aa0)(BuildingInstance *);
extern FN_get_production_info_005b1aa0 GetProductionInfo;

// Returns: 0 - nothing found, 1 - building found, 2 - linked list found, 3 - character found
typedef int (*FN_find_by_object_id_005015e0)(BuildingInstance **param_1,LinkedList **param_2,Character **param_3,int object_id);
extern FN_find_by_object_id_005015e0 FindById;


//----------------------------------------------------------------------------


extern int *g_objects_count;
extern ObjectPrototype_65** g_objects_prototypes;
extern Character* g_characters;
extern BuildingInstance **g_market_ptr;
extern BuildingInstance** g_current_building_ptr;
extern BuildingInstance** g_current_building_ptr_2;
extern LinkedList** g_current_room_ptr;
extern Character_580* g_char_580_1;
extern Character_580* g_char_580_2;


//----------------------------------------------------------------------------


void InitGlobals();

