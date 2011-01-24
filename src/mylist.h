#ifndef MYLIST_H_INCLUDED
#define MYLIST_H_INCLUDED

//node structure
struct MList_node
{
    MList_node  *next;      //pointer to next node
    void        *data;      //pointer to data
};

//List structure
struct MList
{
    MList_node  *CurNode;   //pointer to current node
    MList_node  *Head;      //pointer to first node
    MList_node  *Tail;      //pointer to last node
    unsigned int count;     //count of elements
};

//Linked-list functions
MList *CreateMList();
void AddToMList(MList *lst, void *item);
void StartMList(MList *lst);
bool NextSMList(MList *lst);
void NextMList(MList *lst);
bool ToIndxMList(MList *lst, unsigned int indx);
void *DataMList(MList *lst);
void DeleteMList(MList *lst);
bool eofMList(MList *lst);

#endif // MYLIST_H_INCLUDED
