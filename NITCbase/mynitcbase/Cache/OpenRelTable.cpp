#include "OpenRelTable.h"

#include <cstring>
#include <cstdlib>
#include <cstdio>

OpenRelTableMetaInfo OpenRelTable::tableMetaInfo[MAX_OPEN];

OpenRelTable::OpenRelTable()
{
    for (int i = 0; i < MAX_OPEN; ++i)
    {
        RelCacheTable::relCache[i] = nullptr;
        AttrCacheTable::attrCache[i] = nullptr;
        tableMetaInfo[i].free = true;
    }

    RecBuffer relCatBlock(RELCAT_BLOCK);

    Attribute relCatRecord[RELCAT_NO_ATTRS];
    relCatBlock.getRecord(relCatRecord, RELCAT_SLOTNUM_FOR_RELCAT);

    struct RelCacheEntry relCacheEntry;
    RelCacheTable::recordToRelCatEntry(relCatRecord, &relCacheEntry.relCatEntry);
    relCacheEntry.recId.block = RELCAT_BLOCK;
    relCacheEntry.recId.slot = RELCAT_SLOTNUM_FOR_RELCAT;

    RelCacheTable::relCache[RELCAT_RELID] = (struct RelCacheEntry *)malloc(sizeof(RelCacheEntry));
    *(RelCacheTable::relCache[RELCAT_RELID]) = relCacheEntry;

    relCatBlock.getRecord(relCatRecord, RELCAT_SLOTNUM_FOR_ATTRCAT);
    RelCacheTable::recordToRelCatEntry(relCatRecord, &relCacheEntry.relCatEntry);
    relCacheEntry.recId.block = RELCAT_BLOCK;
    relCacheEntry.recId.slot = RELCAT_SLOTNUM_FOR_ATTRCAT;

    RelCacheTable::relCache[ATTRCAT_RELID] = (struct RelCacheEntry *)malloc(sizeof(RelCacheEntry));
    *(RelCacheTable::relCache[ATTRCAT_RELID]) = relCacheEntry;

    RecBuffer attrCatBlock(ATTRCAT_BLOCK);

    Attribute attrCatRecord[ATTRCAT_NO_ATTRS];

    struct AttrCacheEntry *Head = nullptr, *Tail = nullptr;
    for (int i = 0; i <= 5; i++)
    {
        attrCatBlock.getRecord(attrCatRecord, i);

        struct AttrCacheEntry attrCacheEntry;
        AttrCacheTable::recordToAttrCatEntry(attrCatRecord, &attrCacheEntry.attrCatEntry);
        attrCacheEntry.recId.block = ATTRCAT_BLOCK;
        attrCacheEntry.recId.slot = i;

        struct AttrCacheEntry *new_entry = (struct AttrCacheEntry *)malloc(sizeof(AttrCacheEntry));
        *new_entry = attrCacheEntry;
        if (Head == nullptr)
        {
            Head = new_entry;
            Tail = new_entry;
        }
        else
        {
            Tail->next = new_entry;
            Tail = new_entry;
        }
        Tail->next = nullptr;
    }

    AttrCacheTable::attrCache[RELCAT_RELID] = Head;

    Head = nullptr;
    Tail = nullptr;
    for (int i = 6; i <= 11; i++)
    {

        attrCatBlock.getRecord(attrCatRecord, i);

        struct AttrCacheEntry attrCacheEntry;
        AttrCacheTable::recordToAttrCatEntry(attrCatRecord, &attrCacheEntry.attrCatEntry);
        attrCacheEntry.recId.block = ATTRCAT_BLOCK;
        attrCacheEntry.recId.slot = i;

        struct AttrCacheEntry *new_entry = (struct AttrCacheEntry *)malloc(sizeof(AttrCacheEntry));
        *new_entry = attrCacheEntry;
        if (Head == nullptr)
        {
            Head = new_entry;
            Tail = new_entry;
        }
        else
        {
            Tail->next = new_entry;
            Tail = new_entry;
        }
        Tail->next = nullptr;
    }

    AttrCacheTable::attrCache[ATTRCAT_RELID] = Head;

    tableMetaInfo[RELCAT_RELID].free = false;
    tableMetaInfo[ATTRCAT_RELID].free = false;

    strcpy(tableMetaInfo[RELCAT_RELID].relName, RELCAT_RELNAME);
    strcpy(tableMetaInfo[ATTRCAT_RELID].relName, ATTRCAT_RELNAME);
}

OpenRelTable::~OpenRelTable()
{
    for (int i = 2; i < MAX_OPEN; ++i)
    {
        if (tableMetaInfo[i].free == false)
        {
            OpenRelTable::closeRel(i);
        }
    }

    /**** Closing the catalog relations in the relation cache ****/

    // releasing the relation cache entry of the attribute catalog

    if (RelCacheTable::relCache[ATTRCAT_RELID]->dirty == true)
    {

        /* Get the Relation Catalog entry from RelCacheTable::relCache
        Then convert it to a record using RelCacheTable::relCatEntryToRecord(). */
        Attribute record[RELCAT_NO_ATTRS];
        RelCacheTable::relCatEntryToRecord(&RelCacheTable::relCache[ATTRCAT_RELID]->relCatEntry, record);
        // declaring an object of RecBuffer class to write back to the buffer
        RecId recId = {RELCAT_BLOCK, RELCAT_SLOTNUM_FOR_ATTRCAT};
        RecBuffer relCatBlock(recId.block);
        relCatBlock.setRecord(record, recId.slot);
        // Write back to the buffer using relCatBlock.setRecord() with recId.slot
    }
    // free the memory dynamically allocated to this RelCacheEntry
    free(RelCacheTable::relCache[ATTRCAT_RELID]);

    // releasing the relation cache entry of the relation catalog

    if (RelCacheTable::relCache[RELCAT_RELID]->dirty == true)
    {

        /* Get the Relation Catalog entry from RelCacheTable::relCache
        Then convert it to a record using RelCacheTable::relCatEntryToRecord(). */
        Attribute record[RELCAT_NO_ATTRS];
        RelCacheTable::relCatEntryToRecord(&RelCacheTable::relCache[RELCAT_RELID]->relCatEntry, record);

        // declaring an object of RecBuffer class to write back to the buffer
        RecId recId = {RELCAT_BLOCK, RELCAT_SLOTNUM_FOR_RELCAT};
        RecBuffer relCatBlock(recId.block);
        relCatBlock.setRecord(record, recId.slot);
        // Write back to the buffer using relCatBlock.setRecord() with recId.slot
    }
    // free the memory dynamically allocated for this RelCacheEntry
    free(RelCacheTable::relCache[RELCAT_RELID]);

    for (int i = RELCAT_RELID; i <= ATTRCAT_RELID; i++)
    {
        struct AttrCacheEntry *Head = AttrCacheTable::attrCache[i];
        while (Head != nullptr)
        {
            struct AttrCacheEntry *entry = Head;
            Head = Head->next;
            free(entry);
        }
    }
    // free the memory allocated for the attribute cache entries of the
    // relation catalog and the attribute catalog
}

int OpenRelTable::getFreeOpenRelTableEntry()
{
    for (int i = 2; i < MAX_OPEN; i++)
    {
        if (tableMetaInfo[i].free == true)
        {
            return i;
        }
    }

    return E_CACHEFULL;
}

int OpenRelTable::getRelId(char relName[ATTR_SIZE])
{
    for (int i = 0; i < MAX_OPEN; i++)
    {
        if (tableMetaInfo[i].free == false && strcmp(tableMetaInfo[i].relName, relName) == 0)
        {
            return i;
        }
    }

    return E_RELNOTOPEN;
}

int OpenRelTable::openRel(char relName[ATTR_SIZE])
{
    int relId = getRelId(relName);
    if (relId >= 0)
    {
        return relId;
    }

    relId = getFreeOpenRelTableEntry();
    if (relId == E_CACHEFULL)
    {
        return E_CACHEFULL;
    }

    RelCacheTable::resetSearchIndex(RELCAT_RELID);

    Attribute relname;
    strcpy(relname.sVal, relName);
    RecId relcatRecId = BlockAccess::linearSearch(RELCAT_RELID, RELCAT_ATTR_RELNAME, relname, EQ);

    if (relcatRecId.block == -1 && relcatRecId.slot == -1)
    {
        return E_RELNOTEXIST;
    }

    RecBuffer relCatBuffer(relcatRecId.block);
    Attribute rec[RELCAT_NO_ATTRS];
    relCatBuffer.getRecord(rec, relcatRecId.slot);

    RelCacheEntry relCacheEntry;
    RelCacheTable::recordToRelCatEntry(rec, &relCacheEntry.relCatEntry);
    relCacheEntry.recId = relcatRecId;

    RelCacheTable::relCache[relId] = (struct RelCacheEntry *)malloc(sizeof(RelCacheEntry));
    *(RelCacheTable::relCache[relId]) = relCacheEntry;

    struct AttrCacheEntry *listHead = nullptr, *listTail = nullptr;

    RelCacheTable::resetSearchIndex(ATTRCAT_RELID);
    for (int i = 0; i < relCacheEntry.relCatEntry.numAttrs; i++)
    {
        RecId attrcatRecId = BlockAccess::linearSearch(ATTRCAT_RELID, ATTRCAT_ATTR_RELNAME, relname, EQ);

        RecBuffer attrCatBuffer(attrcatRecId.block);
        Attribute attrCatRec[ATTRCAT_NO_ATTRS];
        attrCatBuffer.getRecord(attrCatRec, attrcatRecId.slot);

        struct AttrCacheEntry attrCacheEntry;
        AttrCacheTable::recordToAttrCatEntry(attrCatRec, &attrCacheEntry.attrCatEntry);
        attrCacheEntry.recId = attrcatRecId;

        struct AttrCacheEntry *new_entry = (struct AttrCacheEntry *)malloc(sizeof(AttrCacheEntry));
        *new_entry = attrCacheEntry;
        if (listHead == nullptr)
        {
            listHead = new_entry;
            listTail = new_entry;
        }
        else
        {
            listTail->next = new_entry;
            listTail = new_entry;
        }
        listTail->next = nullptr;
    }

    AttrCacheTable::attrCache[relId] = listHead;

    tableMetaInfo[relId].free = false;
    strcpy(tableMetaInfo[relId].relName, relName);

    return relId;
}

int OpenRelTable::closeRel(int relId)
{
    if (relId == RELCAT_RELID || relId == ATTRCAT_RELID)
        return E_NOTPERMITTED;

    if (relId < 0 || relId >= MAX_OPEN)
        return E_OUTOFBOUND;

    if (tableMetaInfo[relId].free == true)
        return E_RELNOTOPEN;

    if (RelCacheTable::relCache[relId]->dirty == 1)
    {
        Attribute record[RELCAT_NO_ATTRS];
        RelCacheTable::relCatEntryToRecord(&RelCacheTable::relCache[relId]->relCatEntry, record);
        // RelCacheTable::resetSearchIndex(RELCAT_RELID);
        // RecId recId = BlockAccess::linearSearch(RELCAT_RELID, RELCAT_ATTR_RELNAME, record[RELCAT_REL_NAME_INDEX], EQ);
        RecId recId = RelCacheTable::relCache[relId]->recId;

        RecBuffer relCatBlock(recId.block);
        relCatBlock.setRecord(record, recId.slot);
    }
    free(RelCacheTable::relCache[relId]);

    AttrCacheEntry *listHead = AttrCacheTable::attrCache[relId];
    while (listHead)
    {
        AttrCacheEntry *entry = listHead;
        listHead = listHead->next;
        free(entry);
    }

    tableMetaInfo[relId].free = true;
    RelCacheTable::relCache[relId] = nullptr;
    AttrCacheTable::attrCache[relId] = nullptr;

    return SUCCESS;
}
