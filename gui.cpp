// Dear ImGui: standalone example application for DirectX 9
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#include "windows.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"
#include <d3d9.h>
#include <tchar.h>
#include <assert.h>
#include <vector>
#include <unordered_set>

#include "imnodes.h"
#include "imnodes_internal.h"
#include "imgui_memory_editor.h"

#pragma pack(push, 1)
#include "ghidra.h"
#pragma pack(pop)


//----------------------------------------------------------------------------


int cyrillic_remap[] = {
0x80, 0x0402,
0x81, 0x0403,
0x83, 0x0453,
0x8A, 0x0409,
0x8C, 0x040A,
0x8D, 0x040C,
0x8E, 0x040B,
0x8F, 0x040F,
0x90, 0x0452,
0x9A, 0x0459,
0x9C, 0x045A,
0x9D, 0x045C,
0x9E, 0x045B,
0x9F, 0x045F,
0xA1, 0x040E,
0xA2, 0x045E,
0xA3, 0x0408,
0xA5, 0x0490,
0xA8, 0x0401,
0xAA, 0x0404,
0xAF, 0x0407,
0xB2, 0x0406,
0xB3, 0x0456,
0xB4, 0x0491,
0xB8, 0x0451,
0xBA, 0x0454,
0xBC, 0x0458,
0xBD, 0x0405,
0xBE, 0x0455,
0xBF, 0x0457,
0xC0, 0x0410,
0xC1, 0x0411,
0xC2, 0x0412,
0xC3, 0x0413,
0xC4, 0x0414,
0xC5, 0x0415,
0xC6, 0x0416,
0xC7, 0x0417,
0xC8, 0x0418,
0xC9, 0x0419,
0xCA, 0x041A,
0xCB, 0x041B,
0xCC, 0x041C,
0xCD, 0x041D,
0xCE, 0x041E,
0xCF, 0x041F,
0xD0, 0x0420,
0xD1, 0x0421,
0xD2, 0x0422,
0xD3, 0x0423,
0xD4, 0x0424,
0xD5, 0x0425,
0xD6, 0x0426,
0xD7, 0x0427,
0xD8, 0x0428,
0xD9, 0x0429,
0xDA, 0x042A,
0xDB, 0x042B,
0xDC, 0x042C,
0xDD, 0x042D,
0xDE, 0x042E,
0xDF, 0x042F,
0xE0, 0x0430,
0xE1, 0x0431,
0xE2, 0x0432,
0xE3, 0x0433,
0xE4, 0x0434,
0xE5, 0x0435,
0xE6, 0x0436,
0xE7, 0x0437,
0xE8, 0x0438,
0xE9, 0x0439,
0xEA, 0x043A,
0xEB, 0x043B,
0xEC, 0x043C,
0xED, 0x043D,
0xEE, 0x043E,
0xEF, 0x043F,
0xF0, 0x0440,
0xF1, 0x0441,
0xF2, 0x0442,
0xF3, 0x0443,
0xF4, 0x0444,
0xF5, 0x0445,
0xF6, 0x0446,
0xF7, 0x0447,
0xF8, 0x0448,
0xF9, 0x0449,
0xFA, 0x044A,
0xFB, 0x044B,
0xFC, 0x044C,
0xFD, 0x044D,
0xFE, 0x044E,
0xFF, 0x044F
};


//----------------------------------------------------------------------------


struct pair_hash {
    template <class T1, class T2>
    std::size_t operator () (const std::pair<T1,T2> &p) const {
        auto h1 = std::hash<T1>{}(p.first);
        auto h2 = std::hash<T2>{}(p.second);

        // Mainly for demonstration purposes, i.e. works but is overly simple
        // In the real world, use sth. like boost.hash_combine
        return h1 ^ h2;  
    }
};

namespace ImGui {
bool InputShort(const char* label, unsigned short* v, unsigned short step = 1, unsigned short step_fast = 100, ImGuiInputTextFlags flags = 0)
{
    // Hexadecimal input provided as a convenience but the flag name is awkward. Typically you'd use InputText() to parse your own data, if you want to handle prefixes.
    const char* format = (flags & ImGuiInputTextFlags_CharsHexadecimal) ? "%04X" : "%d";
    return InputScalar(label, ImGuiDataType_U16, (void*)v, (void*)(step > 0 ? &step : NULL), (void*)(step_fast > 0 ? &step_fast : NULL), format, flags);
}

bool InputByte(const char* label, unsigned char* v, unsigned short step = 1, unsigned short step_fast = 10, ImGuiInputTextFlags flags = 0)
{
    // Hexadecimal input provided as a convenience but the flag name is awkward. Typically you'd use InputText() to parse your own data, if you want to handle prefixes.
    const char* format = (flags & ImGuiInputTextFlags_CharsHexadecimal) ? "%02X" : "%d";
    return InputScalar(label, ImGuiDataType_U8, (void*)v, (void*)(step > 0 ? &step : NULL), (void*)(step_fast > 0 ? &step_fast : NULL), format, flags);
}

bool SliderShort(const char* label, short* v, int v_min, int v_max, const char* format = "%d", ImGuiSliderFlags flags = 0)
{
    return SliderScalar(label, ImGuiDataType_S16, v, &v_min, &v_max, format, flags);
}

bool SliderByte(const char* label, char* v, int v_min, int v_max, const char* format = "%d", ImGuiSliderFlags flags = 0)
{
    return SliderScalar(label, ImGuiDataType_S8, v, &v_min, &v_max, format, flags);
}
bool SliderByte(const char* label, unsigned char* v, int v_min, int v_max, const char* format = "%d", ImGuiSliderFlags flags = 0)
{
    return SliderScalar(label, ImGuiDataType_U8, v, &v_min, &v_max, format, flags);
}
}


//----------------------------------------------------------------------------


typedef LinkedList *(*__FindFreeLinkedListNode)();
__FindFreeLinkedListNode FindFreeLinkedListNode = (__FindFreeLinkedListNode)0x0502e00;

typedef ProductionInfo *(*FN_get_production_info_005b1aa0)(BuildingInstance *);
FN_get_production_info_005b1aa0 GetProductionInfo = (FN_get_production_info_005b1aa0)0x05b1aa0;

// Returns: 0 - nothing found, 1 - building found, 2 - linked list found, 3 - character found
typedef int (*FN_find_by_object_id_005015e0)(BuildingInstance **param_1,LinkedList **param_2,Character **param_3,int object_id);
FN_find_by_object_id_005015e0 FindById = (FN_find_by_object_id_005015e0)0x05015e0;


//----------------------------------------------------------------------------


static int *g_objects_count = (int*)0x0699364;
static ObjectPrototype_65** g_objects_prototypes = (ObjectPrototype_65**)0x099EC10;
static Character* g_characters = (Character*)0x7f4ca0;
static BuildingInstance **g_market_ptr = (BuildingInstance**)0x0773D78;
static BuildingInstance** g_current_building_ptr = (BuildingInstance**)0x0774718;
static LinkedList** g_current_room_ptr = (LinkedList**)0x07746cc;


//----------------------------------------------------------------------------


bool viewDoesNotExist(Character *ptr) {
    return ImNodes::EditorContextGet().Nodes.IdMap.GetInt(static_cast<ImGuiID>(ptr->object_id), -1) < 0;
}
bool viewDoesNotExist(BuildingInstance *ptr) {
    return ImNodes::EditorContextGet().Nodes.IdMap.GetInt(static_cast<ImGuiID>(ptr->object_id), -1) < 0;
}
bool viewDoesNotExist(LinkedList *ptr) {
    return ImNodes::EditorContextGet().Nodes.IdMap.GetInt(static_cast<ImGuiID>(ptr->this_object_id), -1) < 0;
}
bool linkDoesNotExist(int id) {
    return ImNodes::EditorContextGet().Links.IdMap.GetInt(static_cast<ImGuiID>(id), -1) < 0;
}

//----------------------------------------------------------------------------

struct ViewBase;
struct ViewLinkedList;
struct ViewCharacter;
struct ViewBuildingInstance;
struct Connection;

struct NodesViewer {
    int widget_id;
    std::unordered_set<std::pair<int, int>, pair_hash> connections;
    ViewBase *root_view;
    float minimap_fraction = 0.15f;
    ImNodesMiniMapLocation minimap_location = ImNodesMiniMapLocation_BottomRight;
    bool draw_minimap = false;
};
static NodesViewer g_viewer;

ViewBase *CreateView(NodesViewer *viewer, Character* ptr);
ViewBase *CreateView(NodesViewer *viewer, LinkedList* ptr);
ViewBase *CreateView(NodesViewer *viewer, BuildingInstance* ptr);

struct Connection {
    int socket;
    int link;
    const char* name;

    ViewBase *view = nullptr;
    bool draw = false;

    ~Connection();

    void Cleanup(NodesViewer *viewer);

    template<class T>
    void DrawSocket(NodesViewer *viewer, T *node);
    void DrawLink(NodesViewer *viewer);
};

struct ViewBase {
    MemoryEditor mem_edit;
    std::vector<Connection> connections;
    int input_socket;
    void* data;
    size_t data_size;

    bool show_mem_edit = false;

    ViewBase(NodesViewer *viewer, void *d, size_t ds) {
        input_socket = viewer->widget_id--;
        data = d;
        data_size = ds;
        mem_edit.OptShowAscii = false;
    }

    virtual ~ViewBase() = default;
    virtual void Cleanup(NodesViewer *viewer) {
        for (auto& c : connections) {
            c.Cleanup(viewer);
        }
    };
    virtual int Id() = 0;
    virtual void DrawNode(NodesViewer *viewer) = 0;
    virtual void Draw(NodesViewer *viewer) {
        ImNodes::BeginNode(Id());
        {
            DrawNode(viewer);
            if (data && data_size) {
                ImGui::Checkbox("Memory editor", &show_mem_edit);
            }
        }
        ImNodes::EndNode();

        for (auto& c: connections) {
            c.DrawLink(viewer);
        }
    };
};

struct ViewInit : ViewBase {
    int id;

    ViewInit(NodesViewer *viewer, void* d) : ViewBase(viewer, d, 0) {
        id = viewer->widget_id--;
        connections.push_back({ viewer->widget_id--, viewer->widget_id--, "Market" });
        connections.push_back({ viewer->widget_id--, viewer->widget_id--, "Current Building" });
        connections.push_back({ viewer->widget_id--, viewer->widget_id--, "Current Room" });
        connections.push_back({ viewer->widget_id--, viewer->widget_id--, "Current Player" });
    }

    virtual ~ViewInit() = default;
    int Id() { return id; }
    void DrawNode(NodesViewer *viewer) {
        connections[0].DrawSocket(viewer, *g_market_ptr);
        connections[1].DrawSocket(viewer, *g_current_building_ptr);
        connections[2].DrawSocket(viewer, *g_current_room_ptr);
        connections[3].DrawSocket(viewer, g_characters + 1);
    }
};

struct ViewLinkedList : public ViewBase {
    enum {
        CONTENT,
        NEXT,
    };

    ViewLinkedList(NodesViewer *viewer, LinkedList* d) : ViewBase(viewer, d, sizeof(*d)) {
        connections.push_back({ viewer->widget_id--, viewer->widget_id--, "Content" });
        connections.push_back({ viewer->widget_id--, viewer->widget_id--, "Next" });
    }

    virtual ~ViewLinkedList() = default;
    int Id() { return ((LinkedList *)this->data)->this_object_id; }
    void DrawNode(NodesViewer *viewer);
};

struct ViewCharacter : public ViewBase {
    virtual ~ViewCharacter() = default;

    ViewCharacter(NodesViewer *viewer, Character* d) : ViewBase(viewer, d, sizeof(*d)) {
        connections.push_back({ viewer->widget_id--, viewer->widget_id--, "Inventory" });
        connections.push_back({ viewer->widget_id--, viewer->widget_id--, "Home" });
    }
    int Id() { return ((Character *)this->data)->object_id; }
    void DrawNode(NodesViewer *viewer);
};

struct ViewBuildingInstance : public ViewBase {
    ViewBuildingInstance(NodesViewer *viewer, BuildingInstance* d) : ViewBase(viewer, d, sizeof(*d))
    {
        connections.push_back({ viewer->widget_id--, viewer->widget_id--, "Content" });
        connections.push_back({ viewer->widget_id--, viewer->widget_id--, "Character #1" });
        connections.push_back({ viewer->widget_id--, viewer->widget_id--, "Character #2" });
    }

    virtual ~ViewBuildingInstance() = default;
    int Id() { return ((BuildingInstance *)this->data)->object_id; }
    void DrawNode(NodesViewer *viewer);
};


//----------------------------------------------------------------------------


Connection::~Connection() {
    if (view) {
        delete view;
        view = nullptr;
    }
}

void Connection::Cleanup(NodesViewer *viewer) {
    if (view) {
        auto pair = std::make_pair(socket, view->input_socket);
        viewer->connections.extract(pair);
        view->Cleanup(viewer);
    }
}

template<class T>
void Connection::DrawSocket(NodesViewer *viewer, T *node) {
    draw = false;

    if (node && (!view || view->data == node)) {
        if (!viewDoesNotExist(node) && linkDoesNotExist(link)) {
            // TODO: write something depending on the T* ???
            ImGui::Text("%s (%X) Already opened", name, node);
            return;
        }

        ImNodes::BeginOutputAttribute(socket);
        {
            if (!view) {
                view = CreateView(viewer, node);
            }

            auto pair = std::make_pair(socket, view->input_socket);

            ImGui::Text("%s (%X)", name, node);
            ImGui::SameLine();

            if (viewer->connections.contains(pair)) {
                if (ImGui::Button("Hide")) {
                    viewer->connections.extract(pair);
                    view->Cleanup(viewer);
                    draw = false;
                } else {
                    draw = true;
                }
            } else {
                draw = false;
                if (ImGui::Button("Show")) {
                    viewer->connections.insert(pair);
                }
            }
        }
        ImNodes::EndOutputAttribute();
    } else if (view) {
        auto pair = std::make_pair(socket, view->input_socket);
        viewer->connections.extract(pair);
        view->Cleanup(viewer);
        delete view;
        view = nullptr;
    }
}
void Connection::DrawLink(NodesViewer *viewer) {
    if (draw) {
        view->Draw(viewer);
        ImNodes::Link(link, socket, view->input_socket);
        draw = false;
    }
}

void ViewLinkedList::DrawNode(NodesViewer *viewer) {
    LinkedList *node = (LinkedList *)this->data;
    bool is_item = false;
    ObjectPrototype_65 *prototype = *g_objects_prototypes + node->object_prot_index;
    switch (prototype->type) {
    case 0:
    case 1:
    case 3:
    case 4:
    case 10:
    case 21:
    case 22:
    case 25:
    case 26:
    case 27:
    case 28:
        ImGui::Text("Extension: %s", prototype->name);
        break;
    case 2:
        ImGui::Text("Room: %s", prototype->name);
        break;
    case 5:
    case 17:
        ImGui::Text("Storage (%s)", prototype->name);
        break;
    case 7:
        ImGui::Text("Money storage (%s)", prototype->name);
        break;
    case 8:
        ImGui::Text("Book (%s)", prototype->name);
        break;
    case 9:
        ImGui::Text("Currency: %s", prototype->name);
        break;
    case 23:
    case 32:
    case 37:
    case 39:
        is_item = true;
        ImGui::Text("Item: %s", prototype->name);
        break;
    case 24:
        ImGui::Text("Production capabilities (%s)", prototype->name);
        break;
    case 29:
        ImGui::Text("Cart: %s", prototype->name);
        break;
    case 33:
        ImGui::Text("Skill improvement: %s", prototype->name);
        break;
    case 36:
        ImGui::Text("Action (%s)", prototype->name);
        break;
    default:
        ImGui::Text("Unknown object type: %i", (int)prototype->type);
        ImGui::Text("Prototype name: %s", prototype->name);
        break;
    }

    ImNodes::BeginInputAttribute(input_socket);
    ImGui::Text("Parent node");
    ImNodes::EndInputAttribute();

    if (show_mem_edit) {
        mem_edit.DrawContents(data, data_size);
    } else {
        ImGui::InputShort("Prototype index", &node->object_prot_index, 1);
        if (prototype->type == 9) {
            ImGui::InputInt("Count", &node->count, 32);
        } else if (is_item) {
            if (node->container_object_id == node->owner_object_id) {
                ImGui::SliderInt("Count", &node->count, 1, 3);
            } else {
                ImGui::SliderInt("Count", &node->count, 1, 100);
            }
        } else if (node->object_prot_index == 42 || node->object_prot_index == 278) {
            ImGui::SliderInt("Capacity Type", &node->count, 1, 3);
        } else {
            ImGui::InputInt("Count", &node->count, 1);
        }

        ImGui::Text("Object ID: %i", node->this_object_id);
        ImGui::Text("Container ID: %i", node->container_object_id);
        ImGui::Text("Owner ID: %i", node->owner_object_id);
        ImGui::Text("Magick byte: %i", (int)node->magick_byte);
        //ImGui::Text("Time: (%i, %i)", (int)node->time_1, (int)node->time_2);
        if (node->object_prot_index == 42) {
            ImGui::SliderByte("Raw material cells", &node->time_1, 1, 8);
            ImGui::SliderByte("Production cells", &node->time_2, 1, 8);
        } else {
            ImGui::InputShort("Time as 2 bytes)", (unsigned short *)&node->time_1, 1);
            ImGui::InputByte("Time byte #1", &node->time_1, 1);
            ImGui::InputByte("Time byte #2", &node->time_2, 1);
        }
        ImGui::Text("Unknown 4 bytes: %X", node->field14_0x1e);
        ImGui::Text("Unknown byte: %i", (int)node->field15_0x22);
    }

    if (node->content) {
        connections[CONTENT].DrawSocket(viewer, node->content);
    } else {
        ImGui::Text("Content");
        ImGui::SameLine();
        if (ImGui::Button("Copy self ->")) {
            LinkedList* tmp = FindFreeLinkedListNode();
            memcpy(tmp, node, sizeof(*node));
            node->content = tmp;
            tmp->this_object_id = (*g_objects_count)++;
        }
    }
    if (node->next) {
        connections[NEXT].DrawSocket(viewer, node->next);
    } else {
        ImGui::Text("Next");
        ImGui::SameLine();
        if (ImGui::Button("Copy self -->")) {
            LinkedList* tmp = FindFreeLinkedListNode();
            memcpy(tmp, node, sizeof(*node));
            node->next = tmp;
            tmp->this_object_id = (*g_objects_count)++;
        }
    }
}

void ViewCharacter::DrawNode(NodesViewer *viewer) {
    Character *character = (Character *)this->data;

    // Player
    if (character->playermode == 6) {
        ImGui::Text("Character type: Player");
    } else if (character->playermode == 5) {
        ImGui::Text("Character type: AI Opponent");
    } else if (character->playermode == 1) {
        ImGui::Text("Character type: Master");
    } else {
        ImGui::Text("Character type: Unknown");
        ImGui::Text("Playermode: %i", (int)character->playermode);
    }

    if (show_mem_edit) {
        mem_edit.DrawContents(data, data_size);
    } else {
        //ImGui::InputText("First Name", character->first_name, sizeof(character->first_name));
        //ImGui::InputText("Last Name", character->last_name, sizeof(character->last_name));

        ImGui::Text("First Name: %s", character->first_name);
        ImGui::Text("Last Name: %s", character->last_name);

        //ImGui::DebugTextEncoding(character->first_name);

        ImGui::Text("Index: %i", character->index);
        ImGui::SliderInt("Action Points", &character->action_points, 1, 50);
        ImGui::Text("Object ID: %i", character->object_id);
        ImGui::Text("Another object ID: %i", character->another_object_id);
        if (character->playermode == 1) {
            ImGui::Text("Master budget: %i", character->master_budget / 32);
        }
        /*
        ImGui::Text("Maybe flags #1: %X", character->flags_maybe);
        ImGui::Text("Maybe flags #2: %X", character->maybe_flags);
        ImGui::Text("Unknown byte #1: %i", (int)character->field6_0xd);
        ImGui::Text("Unknown byte #2: %i", (int)character->field419_0x215);
        ImGui::Text("Unknown byte #3: %i", (int)character->field420_0x216);
        ImGui::Text("Unknown byte #4: %i", (int)character->field421_0x217);
        ImGui::Text("Unknown byte #5: %i", (int)character->field422_0x218);
        ImGui::Text("Unknown int #1: %X (%i)", character->field314_0x190, character->field314_0x190);
        ImGui::Text("Unknown int #2: %X (%i)", character->field315_0x194, character->field315_0x194);
        ImGui::Text("Unknown int #3: %X (%i)", character->field379_0x1e4, character->field379_0x1e4);
        ImGui::Text("Unknown int #4: %X (%i)", character->field380_0x1e8, character->field380_0x1e8);
        ImGui::Text("Unknown int #5: %X (%i)", character->field413_0x20c, character->field413_0x20c);
        */
    }

    ImNodes::BeginInputAttribute(input_socket);
    ImGui::Text("Parent");
    ImNodes::EndInputAttribute();

    connections[0].DrawSocket(viewer, character->inventory);
    connections[1].DrawSocket(viewer, character->maybe_home);
}

void ViewBuildingInstance::DrawNode(NodesViewer *viewer) {
    BuildingInstance *node = (BuildingInstance *)this->data;

    ImNodesEditorContext& editor = ImNodes::EditorContextGet();
    auto id_map = editor.Nodes.IdMap;

    ImGui::Text("Building(idx: %i, id: %i): %s", (int)node->building_prot_index, (int)node->object_id, node->building_name);

    if (show_mem_edit) {
        mem_edit.DrawContents(data, data_size);
    } else {
        ImGui::Text("Object Prototype: %i", (int)node->object_prot_index);
        ImGui::Text("Character ID: %i", node->character_object_id);

        /*
        ImGui::Text("int #01: %X (%i)", node->field6_0x2b,  node->field6_0x2b);
        ImGui::Text("int #02: %X (%i)", node->field8_0x30,  node->field8_0x30);
        ImGui::Text("int #03: %X (%i)", node->field14_0x39, node->field14_0x39);
        ImGui::Text("int #04: %X (%i)", node->field15_0x3d, node->field15_0x3d);
        ImGui::Text("int #05: %X (%i)", node->field16_0x41, node->field16_0x41);
        ImGui::Text("int #06: %X (%i)", node->field17_0x45, node->field17_0x45);
        ImGui::Text("int #07: %X (%i)", node->field38_0x69, node->field38_0x69);
        ImGui::Text("int #08: %X (%i)", node->field39_0x6d, node->field39_0x6d);
        ImGui::Text("int #09: %X (%i)", node->field40_0x71, node->field40_0x71);
        ImGui::Text("int #10: %X (%i)", node->field72_0x95, node->field72_0x95);
        ImGui::Text("int #11: %X (%i)", node->field77_0x9d, node->field77_0x9d);
        ImGui::Text("int #12: %X (%i)", node->field78_0xa1, node->field78_0xa1);
        ImGui::Text("int #13: %X (%i)", node->field79_0xa5, node->field79_0xa5);
        ImGui::Text("float #01: %f", node->field18_0x49);
        */

        ImGui::Text("Characters: [%i, %i]", node->character_index_1, node->character_index_2);
    }

    ImNodes::BeginInputAttribute(input_socket);
    ImGui::Text("Parent");
    ImNodes::EndInputAttribute();

    connections[0].DrawSocket(viewer, node->building_content);
    connections[1].DrawSocket(viewer, g_characters + node->character_index_1);
    connections[2].DrawSocket(viewer, g_characters + node->character_index_2);
}


//----------------------------------------------------------------------------


ViewBase *CreateView(NodesViewer *viewer, Character* ptr) {
    return new ViewCharacter(viewer, ptr);
}
ViewBase *CreateView(NodesViewer *viewer, LinkedList* ptr) {
    return new ViewLinkedList(viewer, ptr);
}
ViewBase *CreateView(NodesViewer *viewer, BuildingInstance* ptr) {
    return new ViewBuildingInstance(viewer, ptr);
}


//----------------------------------------------------------------------------


void DrawNodesViewer(NodesViewer *viewer) {
    ImGui::Begin("Object Instance");
    ImGui::Text("Connections count: %i", viewer->connections.size());
    ImGui::SameLine();
    ImGui::Text("| Minimap [");
    ImGui::SameLine();
    ImGui::Checkbox("draw", &viewer->draw_minimap);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(100);
    ImGui::SliderFloat("fraction", &viewer->minimap_fraction, 0.1, 0.5);
    ImGui::SameLine();
    ImGui::Text("]");
    ImNodes::BeginNodeEditor();

    ImGui::PushItemWidth(150);

    // XXX
    if (!viewer->root_view) {
        viewer->widget_id = -1;
        viewer->root_view = new ViewInit(viewer, nullptr);
    }

    ImGui::SetCursorScreenPos(ImVec2(10, 10));

    viewer->root_view->Draw(viewer);

    ImGui::PopItemWidth();

    if (viewer->draw_minimap) {
        // Must be right before EndNodeEditor
        ImNodes::MiniMap(viewer->minimap_fraction, viewer->minimap_location);
    }
    ImNodes::EndNodeEditor();

    ImGui::End();
}


//----------------------------------------------------------------------------


// Data
static LPDIRECT3D9              g_pD3D = NULL;
static LPDIRECT3DDEVICE9        g_pd3dDevice = NULL;
static D3DPRESENT_PARAMETERS    g_d3dpp = {};

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void ResetDevice();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static bool __initialized = false;

// Main code
void gui_init()
{
    // Create application window
    //ImGui_ImplWin32_EnableDpiAwareness();
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("ImGui Example"), NULL };
    ::RegisterClassEx(&wc);
    HWND hwnd = ::CreateWindow(wc.lpszClassName, _T("Dear ImGui DirectX9 Example"), WS_OVERLAPPEDWINDOW, 0, 0, 600, 800, NULL, NULL, wc.hInstance, NULL);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClass(wc.lpszClassName, wc.hInstance);
        return;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImNodes::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX9_Init(g_pd3dDevice);

    io.MouseDrawCursor = true;

/*
    ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\Tahoma.ttf", 15.0f, NULL, io.Fonts->GetGlyphRangesCyrillic());
    io.Fonts->Build();
    for (int i = 0; i < sizeof(cyrillic_remap) / sizeof(*cyrillic_remap); i += 2) {
        font->AddRemapChar(cyrillic_remap[i], cyrillic_remap[i + 1]);
    }
    font->AddRemapChar(0xD2, 0x0422);
    io.FontDefault = font;

    char buf[64];
    snprintf(buf, 64, "%X", 'Υπεν');
    MessageBoxA(NULL, buf, "Hmm...", MB_OK);
*/

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

    __initialized = true;
}

void gui_deinit() {
    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImNodes::DestroyContext();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    // TODO
    //::DestroyWindow(hwnd);
    //::UnregisterClass(wc.lpszClassName, wc.hInstance);
}

void gui_loop_step() {
    if (!__initialized) gui_init();

    // Our state
    static bool show_demo_window = true;
    static bool show_another_window = false;
    static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    MSG msg;
    while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
    {
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
        if (msg.message == WM_QUIT)
            break;
    }

    // Start the Dear ImGui frame
    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    DrawNodesViewer(&g_viewer);
/*
    // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
    if (show_demo_window)
        ImGui::ShowDemoWindow(&show_demo_window);

    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
    {
        static float f = 0.0f;
        static int counter = 0;

        ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

        ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
        ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
        ImGui::Checkbox("Another Window", &show_another_window);

        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
        ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

        if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
            counter++;
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
    }

    // 3. Show another simple window.
    if (show_another_window)
    {
        ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        ImGui::Text("Hello from another window!");
        if (ImGui::Button("Close Me"))
            show_another_window = false;
        ImGui::End();
    }
*/
    // Rendering
    ImGui::EndFrame();
    g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
    g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
    D3DCOLOR clear_col_dx = D3DCOLOR_RGBA((int)(clear_color.x*clear_color.w*255.0f), (int)(clear_color.y*clear_color.w*255.0f), (int)(clear_color.z*clear_color.w*255.0f), (int)(clear_color.w*255.0f));
    g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);
    if (g_pd3dDevice->BeginScene() >= 0)
    {
        ImGui::Render();
        ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
        g_pd3dDevice->EndScene();
    }
    HRESULT result = g_pd3dDevice->Present(NULL, NULL, NULL, NULL);

    // Handle loss of D3D9 device
    if (result == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
        ResetDevice();
}

// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
    if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
        return false;

    // Create the D3DDevice
    ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
    g_d3dpp.Windowed = TRUE;
    g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN; // Need to use an explicit format with alpha if needing per-pixel alpha composition.
    g_d3dpp.EnableAutoDepthStencil = TRUE;
    g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;           // Present with vsync
    //g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;   // Present without vsync, maximum unthrottled framerate
    if (g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0)
        return false;

    return true;
}

void CleanupDeviceD3D()
{
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
    if (g_pD3D) { g_pD3D->Release(); g_pD3D = NULL; }
}

void ResetDevice()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
    HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
    if (hr == D3DERR_INVALIDCALL)
        IM_ASSERT(0);
    ImGui_ImplDX9_CreateDeviceObjects();
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
        {
            g_d3dpp.BackBufferWidth = LOWORD(lParam);
            g_d3dpp.BackBufferHeight = HIWORD(lParam);
            ResetDevice();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}
