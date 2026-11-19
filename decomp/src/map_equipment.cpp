// Showing the player's equipment and the house parts on the map model: every part has several versions (nodes
// "h03", "h03a", "h03b", ...); all are hidden first (node state bit 0x20 set, bit 4 cleared), then the version
// chosen by the part's script flag is shown. Version 2 parts also have alpha nodes ("Q0", "QQ0", "QQ0Alpha") that
// are listed for the alpha pass.

// A scene node (scene_node.cpp), only the calls used here.
struct Node
{
    Node *Find(const char *name, int recursive);     // 0x1019ece0
    void SetFlags(unsigned int bits);                // 0x1019b130
    void ClearFlags(unsigned int bits);              // 0x1019b1b0
};

extern "C" Node *DAT_10255f08;                       // the map model
extern "C" int DAT_10c62edc[0x1000];                 // script flags
extern "C" Node *DAT_1049ef90[];                     // alpha-pass nodes: the "Q" node of each listed part
extern "C" Node *DAT_102550c0[];                     //   its "QQ" node
extern "C" Node *DAT_1020aefc[];                     //   its "QQ...Alpha" node
extern "C" int DAT_1032d868;                         // number of listed parts
extern "C" int DAT_104bfe6c;
extern "C" int DAT_10219bb8;

// Finds node `name` in the map model and sets or clears node state bits on it (when it exists).
#define SET_NODE(name, bits)     node = DAT_10255f08->Find(name, 0);     if (node != 0)         node->SetFlags(bits)
#define CLEAR_NODE(name, bits)     node = DAT_10255f08->Find(name, 0);     if (node != 0)         node->ClearFlags(bits)

// 0x100135b0 (Map_ShowEquipment). Shows the versions of the four equipment parts chosen by flags 0x6d6-0x6d9, the
// object set of flag 0x6db (0 = "object", else "object2") and the house of the same flag (3 = the second house),
// then sets bit 4 on the first four listed alpha nodes. When DAT_104bfe6c is set the listed count moves to
// DAT_10219bb8 and the list is emptied. Returns 0.
extern "C" int FUN_100135b0(void)
{
    Node *node;
    // part h03: hide every version, then show the one chosen by flag 0x6d6
    SET_NODE("h03", 0x20);
    CLEAR_NODE("h03a", 4);
    SET_NODE("h03b", 0x20);
    CLEAR_NODE("h03ba", 4);
    SET_NODE("QQ0", 0x20);
    SET_NODE("QQ0Alpha", 0x20);
    CLEAR_NODE("h03ca", 4);
    CLEAR_NODE("Q0", 4);
    if (DAT_10c62edc[0x6d6] == 0)
    {
        CLEAR_NODE("h03", 0x20);
        SET_NODE("h03a", 4);
    }
    else if (DAT_10c62edc[0x6d6] == 1)
    {
        CLEAR_NODE("h03b", 0x20);
        SET_NODE("h03ba", 4);
    }
    else if (DAT_10c62edc[0x6d6] == 2)
    {
        CLEAR_NODE("h03b", 0x20);
        SET_NODE("h03ba", 4);
        CLEAR_NODE("QQ0", 0x20);
        CLEAR_NODE("QQ0Alpha", 0x20);
        SET_NODE("h03ca", 4);
        DAT_1049ef90[DAT_1032d868] = DAT_10255f08->Find("Q0", 0);
        DAT_102550c0[DAT_1032d868] = DAT_10255f08->Find("QQ0", 0);
        DAT_1020aefc[DAT_1032d868] = DAT_10255f08->Find("QQ0Alpha", 0);
        DAT_1032d868++;
    }
    // part h01: hide every version, then show the one chosen by flag 0x6d7
    SET_NODE("h01", 0x20);
    CLEAR_NODE("h01a", 4);
    SET_NODE("h01b", 0x20);
    CLEAR_NODE("h01ba", 4);
    SET_NODE("QQ1", 0x20);
    SET_NODE("QQ1Alpha", 0x20);
    CLEAR_NODE("h01ca", 4);
    CLEAR_NODE("Q1", 4);
    if (DAT_10c62edc[0x6d7] == 0)
    {
        CLEAR_NODE("h01", 0x20);
        SET_NODE("h01a", 4);
    }
    else if (DAT_10c62edc[0x6d7] == 1)
    {
        CLEAR_NODE("h01b", 0x20);
        SET_NODE("h01ba", 4);
    }
    else if (DAT_10c62edc[0x6d7] == 2)
    {
        CLEAR_NODE("h01b", 0x20);
        SET_NODE("h01ba", 4);
        CLEAR_NODE("QQ1", 0x20);
        CLEAR_NODE("QQ1Alpha", 0x20);
        SET_NODE("h01ca", 4);
        DAT_1049ef90[DAT_1032d868] = DAT_10255f08->Find("Q1", 0);
        DAT_102550c0[DAT_1032d868] = DAT_10255f08->Find("QQ1", 0);
        DAT_1020aefc[DAT_1032d868] = DAT_10255f08->Find("QQ1Alpha", 0);
        DAT_1032d868++;
    }
    // part h06: hide every version, then show the one chosen by flag 0x6d8
    SET_NODE("h06", 0x20);
    CLEAR_NODE("h06a", 4);
    SET_NODE("h06b", 0x20);
    CLEAR_NODE("h06ba", 4);
    SET_NODE("QQ2", 0x20);
    SET_NODE("QQ2Alpha", 0x20);
    CLEAR_NODE("h06ca", 4);
    CLEAR_NODE("Q2", 4);
    if (DAT_10c62edc[0x6d8] == 0)
    {
        CLEAR_NODE("h06", 0x20);
        SET_NODE("h06a", 4);
    }
    else if (DAT_10c62edc[0x6d8] == 1)
    {
        CLEAR_NODE("h06b", 0x20);
        SET_NODE("h06ba", 4);
    }
    else if (DAT_10c62edc[0x6d8] == 2)
    {
        CLEAR_NODE("h06b", 0x20);
        SET_NODE("h06ba", 4);
        CLEAR_NODE("QQ2", 0x20);
        CLEAR_NODE("QQ2Alpha", 0x20);
        SET_NODE("h06ca", 4);
        DAT_1049ef90[DAT_1032d868] = DAT_10255f08->Find("Q2", 0);
        DAT_102550c0[DAT_1032d868] = DAT_10255f08->Find("QQ2", 0);
        DAT_1020aefc[DAT_1032d868] = DAT_10255f08->Find("QQ2Alpha", 0);
        DAT_1032d868++;
    }
    // part h07: hide every version, then show the one chosen by flag 0x6d9
    SET_NODE("h07", 0x20);
    CLEAR_NODE("h07a", 4);
    SET_NODE("h07b", 0x20);
    CLEAR_NODE("h07ba", 4);
    SET_NODE("QQ3", 0x20);
    SET_NODE("QQ3Alpha", 0x20);
    CLEAR_NODE("h07ca", 4);
    CLEAR_NODE("Q3", 4);
    if (DAT_10c62edc[0x6d9] == 0)
    {
        CLEAR_NODE("h07", 0x20);
        SET_NODE("h07a", 4);
    }
    else if (DAT_10c62edc[0x6d9] == 1)
    {
        CLEAR_NODE("h07b", 0x20);
        SET_NODE("h07ba", 4);
    }
    else if (DAT_10c62edc[0x6d9] == 2)
    {
        CLEAR_NODE("h07b", 0x20);
        SET_NODE("h07ba", 4);
        CLEAR_NODE("QQ3", 0x20);
        CLEAR_NODE("QQ3Alpha", 0x20);
        SET_NODE("h07ca", 4);
        DAT_1049ef90[DAT_1032d868] = DAT_10255f08->Find("Q3", 0);
        DAT_102550c0[DAT_1032d868] = DAT_10255f08->Find("QQ3", 0);
        DAT_1020aefc[DAT_1032d868] = DAT_10255f08->Find("QQ3Alpha", 0);
        DAT_1032d868++;
    }

    SET_NODE("object", 0x20);
    CLEAR_NODE("objecta", 4);
    SET_NODE("object2", 0x20);
    CLEAR_NODE("object2a", 4);
    int room = DAT_10c62edc[0x6db];
    if (room == 0)
    {
        CLEAR_NODE("object", 0x20);
        SET_NODE("objecta", 4);
    }
    else
    {
        CLEAR_NODE("object2", 0x20);
        SET_NODE("object2a", 4);
    }
    SET_NODE("RSObject", 0x20);
    SET_NODE("hibiwall", 0x20);
    SET_NODE("RSObjectAlpha", 0x20);
    CLEAR_NODE("CA_house", 4);
    CLEAR_NODE("houseAyuka", 4);
    CLEAR_NODE("houseAkabe", 4);
    SET_NODE("RSObject2", 0x20);
    SET_NODE("wall", 0x20);
    SET_NODE("RSObject2Alpha", 0x20);
    CLEAR_NODE("CA_house2", 4);
    CLEAR_NODE("houseA2yuka", 4);
    CLEAR_NODE("houseA2kabe", 4);
    if (room == 3)
    {
        CLEAR_NODE("RSObject2", 0x20);
        CLEAR_NODE("wall", 0x20);
        CLEAR_NODE("RSObject2Alpha", 0x20);
        SET_NODE("CA_house2", 4);
        SET_NODE("houseA2yuka", 4);
        SET_NODE("houseA2kabe", 4);
    }
    else
    {
        CLEAR_NODE("RSObject", 0x20);
        CLEAR_NODE("hibiwall", 0x20);
        CLEAR_NODE("RSObjectAlpha", 0x20);
        SET_NODE("CA_house", 4);
        SET_NODE("houseAyuka", 4);
        SET_NODE("houseAkabe", 4);
    }
    if (DAT_1049ef90[0] != 0)
        DAT_1049ef90[0]->SetFlags(4);
    if (DAT_1049ef90[1] != 0)
        DAT_1049ef90[1]->SetFlags(4);
    if (DAT_1049ef90[2] != 0)
        DAT_1049ef90[2]->SetFlags(4);
    if (DAT_1049ef90[3] != 0)
        DAT_1049ef90[3]->SetFlags(4);
    if (DAT_104bfe6c != 0)
    {
        DAT_10219bb8 = DAT_1032d868;
        DAT_1032d868 = 0;
    }
    return 0;
}
