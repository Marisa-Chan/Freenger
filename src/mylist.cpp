//Simple single linked-list procedures and structures
//It's provide only basic functions:
//Create lists, add items(only to the end)
//Get list elements by index, or listing manualy
//Delete list (only full list, but it's easy to modify it's to any way)

#include <stdlib.h>

#include "mylist.h"




//Creates single linked-list object
MList *CreateMList()
{
    MList *tmp;
    tmp = new (MList);

    tmp->CurNode    = NULL;
    tmp->Head       = NULL;
    tmp->Tail       = NULL;
    tmp->count      = 0;
    return tmp;
}

//Adds item to linked-list
void AddToMList(MList *lst, void *item)
{
    MList_node *tmp = new (MList_node);
    tmp->data = item;
    tmp->next = NULL;
    if (lst->count == 0)
    {
        lst->Head=tmp;
        lst->CurNode=tmp;
        lst->Tail=tmp;

    }
    else
    {
        lst->Tail->next=tmp;
        lst->Tail=tmp;
    }

    lst->count++;
}

//Go to the first linked-list item
void StartMList(MList *lst)
{
    lst->CurNode = lst->Head;
}

//Go to next linked-list item
//returns false if next item not exist
bool NextSMList(MList *lst)
{
    if (lst->CurNode->next)
    {
        lst->CurNode=lst->CurNode->next;
        return true;
    }
    else
        return false;
}

//Go to next linked-list item without checking of item exist's
void NextMList(MList *lst)
{
    if (lst->CurNode)
    lst->CurNode=lst->CurNode->next;
}

//Go to element index
bool ToIndxMList(MList *lst, unsigned int indx)
{
    if (lst->count > indx)
    {
        lst->CurNode = lst->Head;
        for (unsigned int iter=0; iter<indx;iter++)
            lst->CurNode = lst->CurNode->next;

        return true;
    }
    else
        return false;
}

//Get data of element
void *DataMList(MList *lst)
{
    return lst->CurNode->data;
}

//Delete list object and delete all nodes assigned to list
void DeleteMList(MList *lst)
{
    MList_node *nxt = lst->Head->next;
    lst->CurNode = lst->Head;
    while (lst->CurNode)
        {
            nxt = lst->CurNode->next;
            delete lst->CurNode;
            lst->CurNode = nxt;
        }
    delete lst;
}

//Return true on EOF of list
bool eofMList(MList *lst)
{
    return (lst->CurNode == NULL);
}

