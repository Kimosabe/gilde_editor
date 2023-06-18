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

#include "globals.h"


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


struct ViewBase;
struct ViewLinkedList;
struct ViewCharacter;
struct ViewBuildingInstance;
struct ViewSomethingAboutBuilding;
struct ViewCharacter580;
struct ViewMemoryEdit;
struct ConnectionV2;

struct NodesViewer {
    int widget_id;
    std::unordered_set<std::pair<int, int>, pair_hash> connections;
    ViewBase *root_view;
    float minimap_fraction = 0.15f;
    ImNodesMiniMapLocation minimap_location = ImNodesMiniMapLocation_BottomRight;
    bool draw_minimap = false;
};
static NodesViewer g_viewer;

struct ConnectionV2 {
    typedef int (*GetID)(ConnectionV2*, void*);
    typedef bool (*GetPtr)(ConnectionV2*, void**);
    typedef ViewBase *(*CreateView)(ConnectionV2*, NodesViewer*, void*);

    int values[3];
    int socket;
    int link;
    const char *name;
    ViewBase *view;
    bool draw;

    GetID get_id;
    GetPtr get_ptr;
    CreateView create_view;

    ~ConnectionV2() {
        if (view) {
            delete view;
            view = nullptr;
        }
    }
};

int GetCharacterID(ConnectionV2 *c, void* ptr);
int GetBuildingInstanceID(ConnectionV2 *c, void* ptr);
int GetLinkedListID(ConnectionV2 *c, void* ptr);
int PtrAsID(ConnectionV2 *c, void* ptr);
int GetIDByID(ConnectionV2 *c, void* ptr);

bool viewDoesNotExist(ConnectionV2 *c, void *ptr);

void Cleanup(ConnectionV2* c, NodesViewer *viewer);
void DrawSocket(ConnectionV2* c, NodesViewer *viewer);
void DrawLink(ConnectionV2* c, NodesViewer *viewer);

bool GetCharacterPtrByIndex(ConnectionV2* c, void** ptr);
bool GetCharacterPtr(ConnectionV2* c, void** ptr);
bool GetBuildingInstancePtr(ConnectionV2* c, void** ptr);
bool GetLinkedListPtr(ConnectionV2* c, void** ptr);
bool GetSomethingAboutBuildingPtr(ConnectionV2* c, void** ptr);
bool GetIDPtr(ConnectionV2* c, void** ptr);
bool GetRawPtr(ConnectionV2* c, void** ptr);

ViewBase *CreateCharacterView(ConnectionV2*, NodesViewer *viewer, void* ptr);
ViewBase *CreateBuildingInstanceView(ConnectionV2*, NodesViewer* viewer, void* ptr);
ViewBase *CreateLinkedListView(ConnectionV2*, NodesViewer* viewer, void* ptr);
ViewBase *CreateSomethingAboutBuildingView(ConnectionV2*, NodesViewer* viewer, void* ptr);
ViewBase *CreateIDView(ConnectionV2*, NodesViewer* viewer, void* ptr);
ViewBase *CreateMemoryEditView(ConnectionV2*, NodesViewer* viewer, void* ptr);
ViewBase *CreateCharacter580View(ConnectionV2*, NodesViewer* viewer, void* ptr);


struct ViewBase {
    MemoryEditor mem_edit;
    std::vector<ConnectionV2> connections_v2;
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
        for (auto &c : connections_v2) {
            ::Cleanup(&c, viewer);
        }
    };
    virtual int Id() = 0;
    virtual void DrawNode(NodesViewer *viewer) {};
    virtual void Draw(NodesViewer *viewer) {
        ImNodes::BeginNode(Id());
        {
            DrawNode(viewer);

            ImNodes::BeginInputAttribute(input_socket);
            ImGui::Text("Parent");
            ImNodes::EndInputAttribute();

            for (auto &c : connections_v2) {
                DrawSocket(&c, viewer);
            }

            if (data && data_size) {
                ImGui::Checkbox("Memory editor", &show_mem_edit);
            }
        }
        ImNodes::EndNode();

        for (auto &c : connections_v2) {
            DrawLink(&c, viewer);
        }
    };
};

#define CHARACTER_INDEX_CONNECTION(index, name) \
connections_v2.push_back({ {(int)(index), (int)(g_characters), (int)(g_characters + 768)}, viewer->widget_id--, viewer->widget_id--, name, nullptr, false, GetCharacterID, GetCharacterPtrByIndex, CreateCharacterView });
#define CHARACTER_CONNECTION(ptr, name) \
connections_v2.push_back({ {(int)ptr, 0, 0}, viewer->widget_id--, viewer->widget_id--, name, nullptr, false, GetCharacterID, GetCharacterPtr, CreateCharacterView });
#define BUILDING_INSTANCE_CONNECTION(ptr, name) \
connections_v2.push_back({ {(int)ptr, 0, 0}, viewer->widget_id--, viewer->widget_id--, name, nullptr, false, GetBuildingInstanceID, GetBuildingInstancePtr, CreateBuildingInstanceView });
#define LINKED_LIST_CONNECTION(ptr, name) \
connections_v2.push_back({ {(int)ptr, 0, 0}, viewer->widget_id--, viewer->widget_id--, name, nullptr, false, GetLinkedListID, GetLinkedListPtr, CreateLinkedListView });
#define SAB_CONNECTION(ptr, name) \
connections_v2.push_back({ {(int)ptr, 0, 0}, viewer->widget_id--, viewer->widget_id--, name, nullptr, false, PtrAsID, GetSomethingAboutBuildingPtr, CreateSomethingAboutBuildingView });
#define ID_CONNECTION(id, name) \
connections_v2.push_back({ {(int)id, 0, 0}, viewer->widget_id--, viewer->widget_id--, name, nullptr, false, GetIDByID, GetIDPtr, CreateIDView });
#define MEMORY_EDIT_CONNECTION(ptr, size, name) \
connections_v2.push_back({ {(int)ptr, (int)size, 0}, viewer->widget_id--, viewer->widget_id--, name, nullptr, false, PtrAsID, GetRawPtr, CreateMemoryEditView });
#define CHARACTER_580_CONNECTION(ptr, name) \
connections_v2.push_back({ {(int)ptr, 0, 0}, viewer->widget_id--, viewer->widget_id--, name, nullptr, false, PtrAsID, GetRawPtr, CreateCharacter580View });


struct ViewInit : ViewBase {
    int id;

    short current_player_index = 1;

    ViewInit(NodesViewer *viewer, void* d) : ViewBase(viewer, d, 0) {
        id = viewer->widget_id--;
        BUILDING_INSTANCE_CONNECTION(g_market_ptr, "Market");
        BUILDING_INSTANCE_CONNECTION(g_current_building_ptr, "Current Building #1");
        BUILDING_INSTANCE_CONNECTION(g_current_building_ptr_2, "Current Building #2");
        LINKED_LIST_CONNECTION(g_current_room_ptr, "Current Room");
        CHARACTER_INDEX_CONNECTION(&current_player_index, "Current Player");
        CHARACTER_580_CONNECTION(g_char_580_1, "g_char_580_1");
        CHARACTER_580_CONNECTION(g_char_580_2, "g_char_580_2");
    }

    virtual ~ViewInit() = default;
    int Id() { return id; }
};

struct ViewMemoryEdit : ViewBase {
    ViewMemoryEdit(NodesViewer *viewer, void* d, size_t size)
    : ViewBase(viewer, d, size) {
        show_mem_edit = true;
    }
    int Id() { return (int)data; }
    void DrawNode(NodesViewer *viewer) {
        if (show_mem_edit) {
            mem_edit.DrawContents(data, data_size);
        }
    };
};

struct ViewCharacter580 : ViewBase {
    ViewCharacter580(NodesViewer *viewer, Character_580* d) : ViewBase(viewer, d, sizeof(*d)) {
        SAB_CONNECTION(&d->ptr_0, "SAB #0");
        SAB_CONNECTION(&d->ptr_1, "SAB #1");
        SAB_CONNECTION(d->ptr_2/*it's a pointer to a pointer*/, "SAB #2");
        SAB_CONNECTION(&d->ptr_3, "SAB #3");

        if (d->character_ptr >= g_characters && d->character_ptr < (g_characters + 768)) {
            CHARACTER_CONNECTION(&d->character_ptr, "character");
        } else {
            LINKED_LIST_CONNECTION(&d->character_ptr, "object");
        }
    }
    int Id() { return (int)data; }
    void DrawNode(NodesViewer *viewer) {
        Character_580 *ptr = (Character_580 *)data;
        ImGui::Text("Char580: %s", ptr->name);
        if (show_mem_edit) {
            mem_edit.DrawContents(data, data_size);
        } else {
            ImGui::Text("Int [0]: %i (%08X)", ptr->field0_0x0, ptr->field0_0x0);
            ImGui::Text("Int [1]: %i (%08X)", ptr->field1_0x4, ptr->field1_0x4);
            ImGui::Text("Int [2]: %i (%08X)", ptr->field3_0x28, ptr->field3_0x28);
            ImGui::Text("Int [3]: %i (%08X)", ptr->field4_0x2c, ptr->field4_0x2c);
            ImGui::Text("Int [4]: %i (%08X)", ptr->field5_0x30, ptr->field5_0x30);
            ImGui::InputFloat("Unknown float", &ptr->field358_0x1a0);
        }
    };
};

struct ViewLinkedList : public ViewBase {
    ViewLinkedList(NodesViewer *viewer, LinkedList* d) : ViewBase(viewer, d, sizeof(*d)) {
        ID_CONNECTION(&d->container_object_id, "Container");
        ID_CONNECTION(&d->owner_object_id, "Owner");
        LINKED_LIST_CONNECTION(&d->content, "Content");
        SAB_CONNECTION(&d->something_about_building_ptr, "SAB ptr");
        MEMORY_EDIT_CONNECTION(&d->ptr_to_64_bytes_linked_list, sizeof(UnknownLinkedList64), "LinkedList64");
        CHARACTER_580_CONNECTION(&d->character_580_ptr, "Char580");
        LINKED_LIST_CONNECTION(&d->next, "Next");
    }

    virtual ~ViewLinkedList() = default;
    int Id() { return ((LinkedList *)this->data)->this_object_id; }
    void DrawNode(NodesViewer *viewer);
};

struct ViewCharacter : public ViewBase {
    virtual ~ViewCharacter() = default;

    ViewCharacter(NodesViewer *viewer, Character* d) : ViewBase(viewer, d, sizeof(*d)) {
        ID_CONNECTION(&d->object_id, "Some object");
        ID_CONNECTION(&d->another_object_id, "Another object");
        LINKED_LIST_CONNECTION(&d->inventory, "Inventory");
        BUILDING_INSTANCE_CONNECTION(&d->maybe_home, "Home");
    }
    int Id() { return ((Character *)this->data)->object_id; }
    void DrawNode(NodesViewer *viewer);
};

struct ViewBuildingInstance : public ViewBase {
    ViewBuildingInstance(NodesViewer *viewer, BuildingInstance* d);

    virtual ~ViewBuildingInstance() = default;
    int Id() { return ((BuildingInstance *)this->data)->object_id; }
    void DrawNode(NodesViewer *viewer);
};

struct ViewSomethingAboutBuilding : public ViewBase {
    ViewSomethingAboutBuilding(NodesViewer *viewer, SomethingAboutBuilding* d);

    virtual ~ViewSomethingAboutBuilding() = default;
    int Id() { return (int)data; }
    void DrawNode(NodesViewer *viewer);
};


//----------------------------------------------------------------------------


int GetCharacterID(ConnectionV2 *c, void* ptr) {
    return ((Character *)ptr)->object_id;
}
int GetBuildingInstanceID(ConnectionV2 *c, void* ptr) {
    return ((BuildingInstance *)ptr)->object_id;
}
int GetLinkedListID(ConnectionV2 *c, void* ptr) {
    return ((LinkedList *)ptr)->this_object_id;
}
int PtrAsID(ConnectionV2 *c, void* ptr) {
    return (int)ptr;
}
int GetIDByID(ConnectionV2 *c, void* ptr) {
    int *id_ptr = (int *)c->values[0];

    if (id_ptr) return *id_ptr;

    assert(false && "Can't get ID by ID");
    return (int)ptr;
}

bool viewDoesNotExist(ConnectionV2 *c, void *ptr) {
    return ImNodes::EditorContextGet().Nodes.IdMap.GetInt(static_cast<ImGuiID>(c->get_id(c, ptr)), -1) < 0;
}
bool linkDoesNotExist(int id) {
    return ImNodes::EditorContextGet().Links.IdMap.GetInt(static_cast<ImGuiID>(id), -1) < 0;
}


void Cleanup(ConnectionV2* c, NodesViewer *viewer) {
    if (c->view) {
        auto pair = std::make_pair(c->socket, c->view->input_socket);
        viewer->connections.extract(pair);
        c->view->Cleanup(viewer);
    }
}
void DrawSocket(ConnectionV2* c, NodesViewer *viewer) {
    c->draw = false;
    void *ptr;

    if (c->get_ptr(c, &ptr)) {
        if (!viewDoesNotExist(c, ptr) && linkDoesNotExist(c->link)) {
            // TODO: write something depending on the c ???
            ImGui::Text("%s (%X) Already opened", c->name, ptr);
            return;
        }

        ImNodes::BeginOutputAttribute(c->socket);
        {
            if (!c->view) {
                c->view = c->create_view(c, viewer, ptr);
            }

            auto pair = std::make_pair(c->socket, c->view->input_socket);

            ImGui::Text("%s (%X)", c->name, ptr);
            ImGui::SameLine();

            if (viewer->connections.contains(pair)) {
                if (ImGui::Button("Hide")) {
                    viewer->connections.extract(pair);
                    c->view->Cleanup(viewer);
                    c->draw = false;
                } else {
                    c->draw = true;
                }
            } else {
                c->draw = false;
                if (ImGui::Button("Show")) {
                    viewer->connections.insert(pair);
                }
            }
        }
        ImNodes::EndOutputAttribute();
    } else if (c->view) {
        auto pair = std::make_pair(c->socket, c->view->input_socket);
        viewer->connections.extract(pair);
        c->view->Cleanup(viewer);
        delete c->view;
        c->view = nullptr;
    }
}
void DrawLink(ConnectionV2* c, NodesViewer *viewer) {
    if (c->draw) {
        c->view->Draw(viewer);
        ImNodes::Link(c->link, c->socket, c->view->input_socket);
        c->draw = false;
    }
}

bool GetCharacterPtrByIndex(ConnectionV2* c, void** ptr) {
    short index = *(short*)c->values[0];
    Character *min_ptr = (Character *)c->values[1];
    Character *max_ptr = (Character *)c->values[2];
    *ptr = min_ptr + index;

    return index > 0 && min_ptr && *ptr >= min_ptr && *ptr < max_ptr && (!c->view || *ptr == c->view->data) && index == ((Character *)*ptr)->index;
}
bool GetCharacterPtr(ConnectionV2* c, void** ptr) {
    if (!c->values[0]) return false;

    Character *character = *(Character**)c->values[0];
    *ptr = character;

    return *ptr && (!c->view || *ptr == c->view->data) && character->index > 0;
}
bool GetBuildingInstancePtr(ConnectionV2* c, void** ptr) {
    if (!c->values[0]) return false;

    BuildingInstance *building = *(BuildingInstance**)c->values[0];
    *ptr = building;

    return *ptr && (!c->view || *ptr == c->view->data) && building->object_id > 0;
}
bool GetLinkedListPtr(ConnectionV2* c, void** ptr) {
    if (!c->values[0]) return false;

    LinkedList* object = *(LinkedList**)c->values[0];
    *ptr = object;

    return *ptr && (!c->view || *ptr == c->view->data) && object->this_object_id > 0;
}
bool GetSomethingAboutBuildingPtr(ConnectionV2* c, void** ptr) {
    if (!c->values[0]) return false;

    SomethingAboutBuilding* sab = *(SomethingAboutBuilding**)c->values[0];
    *ptr = sab;

    // XXX: maybe it's just another structure
    bool bad_sab = *ptr >= (void*)0x1040640 && *ptr <= *(void**)0x067a370;

    return *ptr && (!c->view || *ptr == c->view->data) && !bad_sab;
}
bool GetIDPtr(ConnectionV2* c, void** ptr) {
    if (!c->values[0]) return false;

    int id = *(int*)c->values[0];
    void *exiting_ptr = (void *)c->values[2];
    int existing_id = 0;

    if (!exiting_ptr) {
        Character *character_ptr;
        LinkedList *object_ptr;
        BuildingInstance *building_ptr;

        c->values[1] = FindById(&building_ptr, &object_ptr, &character_ptr, id);
        switch (c->values[1]) {
        case 1:
            *ptr = building_ptr;
            existing_id = building_ptr->object_id;
            break;
        case 2:
            *ptr = object_ptr;
            existing_id = object_ptr->this_object_id;
            break;
        case 3:
            *ptr = character_ptr;
            existing_id = character_ptr->object_id;
            break;
        case 0:
        default:
            *ptr = nullptr;
            break;
        }
        c->values[2] = (int)*ptr;
    } else {
        *ptr = exiting_ptr;
        switch (c->values[1]) {
        case 1:
            existing_id = ((BuildingInstance *)exiting_ptr)->object_id;
            break;
        case 2:
            existing_id = ((LinkedList *)exiting_ptr)->this_object_id;
            break;
        case 3:
            existing_id = ((Character *)exiting_ptr)->object_id;
            break;
        }
    }

    return id > 0 && id == existing_id && *ptr && (!c->view || *ptr == c->view->data);
}
bool GetRawPtr(ConnectionV2* c, void** ptr) {
    if (!c->values[0]) return false;

    void *current_ptr = *(void**)c->values[0];
    *ptr = current_ptr;

    return *ptr && (!c->view || *ptr == c->view->data);
}
ViewBase *CreateCharacterView(ConnectionV2*, NodesViewer *viewer, void* ptr) {
    return new ViewCharacter(viewer, (Character *)ptr);
}
ViewBase* CreateBuildingInstanceView(ConnectionV2*, NodesViewer* viewer, void* ptr) {
    return new ViewBuildingInstance(viewer, (BuildingInstance*)ptr);
}
ViewBase* CreateLinkedListView(ConnectionV2*, NodesViewer* viewer, void* ptr) {
    return new ViewLinkedList(viewer, (LinkedList*)ptr);
}
ViewBase* CreateSomethingAboutBuildingView(ConnectionV2*, NodesViewer* viewer, void* ptr) {
    return new ViewSomethingAboutBuilding(viewer, (SomethingAboutBuilding*)ptr);
}
ViewBase* CreateIDView(ConnectionV2* c, NodesViewer* viewer, void* ptr) {
    switch (c->values[1]) {
    case 1:
        return CreateBuildingInstanceView(c, viewer, ptr);
    case 2:
        return CreateLinkedListView(c, viewer, ptr);
    case 3:
        return CreateCharacterView(c, viewer, ptr);
    }

    assert(false && "Can't create view by ID");
    return nullptr;
}
ViewBase* CreateMemoryEditView(ConnectionV2* c, NodesViewer* viewer, void* ptr) {
    return new ViewMemoryEdit(viewer, ptr, (size_t)c->values[1]);
}
ViewBase* CreateCharacter580View(ConnectionV2*, NodesViewer* viewer, void* ptr) {
    return new ViewCharacter580(viewer, (Character_580*)ptr);
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

        if (prototype->type == 36) {
            ImGui::Text("Action active: %i", (int)node->action_in_progress_flag);
            ImGui::Text("Character ID for action: %i", node->character_id_for_action);
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
    }

    ImGui::Text("Unknown pointer: %X", node->ptr_to_64_bytes_linked_list);

    if (!node->content) {
        ImGui::Text("Content");
        ImGui::SameLine();
        if (ImGui::Button("Copy self ->")) {
            LinkedList* tmp = FindFreeLinkedListNode();
            memcpy(tmp, node, sizeof(*node));
            node->content = tmp;
            tmp->this_object_id = (*g_objects_count)++;
        }
    }
    if (!node->next) {
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
}

ViewBuildingInstance::ViewBuildingInstance(NodesViewer *viewer, BuildingInstance* d) : ViewBase(viewer, d, sizeof(*d))
{
    ID_CONNECTION(&d->character_object_id, "Some character");
    LINKED_LIST_CONNECTION(&d->building_content, "Content");
    CHARACTER_INDEX_CONNECTION(&d->character_index_1, "Character #1");
    CHARACTER_INDEX_CONNECTION(&d->character_index_2, "Character #2");
    SAB_CONNECTION(&d->building_something, "SAB pointer");
}

void ViewBuildingInstance::DrawNode(NodesViewer *viewer) {
    BuildingInstance *node = (BuildingInstance *)this->data;

    ImNodesEditorContext& editor = ImNodes::EditorContextGet();
    auto id_map = editor.Nodes.IdMap;

    ImGui::Text("Building(idx: %i, id: %i): %s", (int)node->building_prot_index, (int)node->object_id, node->building_name);

    if (show_mem_edit) {
        mem_edit.DrawContents(data, data_size);
    } else {
        ImGui::Text("Current Room Prototype: %i", (int)node->current_room_prot_index);
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
}

ViewSomethingAboutBuilding::ViewSomethingAboutBuilding(NodesViewer *viewer, SomethingAboutBuilding* d)
: ViewBase(viewer, d, sizeof(*d))
{
    SAB_CONNECTION(&d->something_about_building_ptr_1, "Ptr #1");
    SAB_CONNECTION(&d->something_about_building_ptr_2, "Ptr #2");
    SAB_CONNECTION(&d->ptr_1, "Ptr #3");
    SAB_CONNECTION(&d->ptr_2, "Ptr #4");
    SAB_CONNECTION(&d->ptr_3, "Ptr #5");
    SAB_CONNECTION(&d->ptr_4, "Ptr #6");
    if (*d->name == 'o') {
        LINKED_LIST_CONNECTION(&d->instance_of_any_type_ptr, "object");
    } else if (*d->name == 'g') {
        BUILDING_INSTANCE_CONNECTION(&d->instance_of_any_type_ptr, "building");
    } else if ((Character *)d->instance_of_any_type_ptr >= g_characters && (Character *)d->instance_of_any_type_ptr < (g_characters + 768)) {
        CHARACTER_CONNECTION(&d->instance_of_any_type_ptr, "character");
    } else {
        LINKED_LIST_CONNECTION(&d->instance_of_any_type_ptr, "object");
    }
    SAB_CONNECTION(&d->yet_another_ptr, "Ptr #7");
    MEMORY_EDIT_CONNECTION(&d->light_info, sizeof(LightInfo), "Light Info");
    MEMORY_EDIT_CONNECTION(&d->draw_data, sizeof(ObjectDrawData), "Draw Data");
    MEMORY_EDIT_CONNECTION(&d->animation, sizeof(ObjectAnimation), "Animation");
}

void ViewSomethingAboutBuilding::DrawNode(NodesViewer *viewer) {
    SomethingAboutBuilding* ptr = (SomethingAboutBuilding*)data;

    ImGui::Text("Name: %s", ptr->name);

    if (show_mem_edit) {
        mem_edit.DrawContents(data, data_size);
    } else {
        ImGui::Text("Animation Ptr: %X", ptr->animation);
        ImGui::Text("LightInfo Ptr: %X", ptr->light_info);
        ImGui::Text("Draw Data Ptr: %X", ptr->draw_data);
        ImGui::Text("Maybe flags: %X", ptr->some_flags);

        ImGui::PushItemWidth(220);
        ImGui::InputFloat3("Position", ptr->position);
        ImGui::PopItemWidth();
        ImGui::PushItemWidth(240);
        ImGui::InputFloat4("Matrix [0]", ptr->matrix_4x4_maybe + 0);
        ImGui::InputFloat4("Matrix [1]", ptr->matrix_4x4_maybe + 4);
        ImGui::InputFloat4("Matrix [2]", ptr->matrix_4x4_maybe + 8);
        ImGui::InputFloat4("Matrix [3]", ptr->matrix_4x4_maybe + 12);
        ImGui::PopItemWidth();
    }
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
