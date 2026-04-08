#include "Algebra.h"

#include <cstring>
#include <cstdlib>
#include <cstdio>
using namespace std;

long long Algebra::ncmps = 0;

int Algebra::select(char srcRel[ATTR_SIZE], char targetRel[ATTR_SIZE], char attr[ATTR_SIZE], int op, char strVal[ATTR_SIZE])
{
    int srcRelId = OpenRelTable::getRelId(srcRel);
    if (srcRelId == E_RELNOTOPEN)
    {
        return E_RELNOTOPEN;
    }

    AttrCatEntry attrCatEntry;
    int attrCat = AttrCacheTable::getAttrCatEntry(srcRelId, attr, &attrCatEntry);
    if (attrCat == E_ATTRNOTEXIST)
    {
        return E_ATTRNOTEXIST;
    }

    /*** Convert strVal to an attribute of data type NUMBER or STRING ***/

    Attribute attrVal;
    int type = attrCatEntry.attrType;
    if (type == NUMBER)
    {
        bool isNumber(char *);
        if (isNumber(strVal))
        {
            attrVal.nVal = atof(strVal);
        }
        else
        {
            return E_ATTRTYPEMISMATCH;
        }
    }
    else if (type == STRING)
    {
        strcpy(attrVal.sVal, strVal);
    }

    RelCatEntry relCatEntry;
    RelCacheTable::getRelCatEntry(srcRelId, &relCatEntry);
    int src_nAttrs = relCatEntry.numAttrs;

    char attr_names[src_nAttrs][ATTR_SIZE];
    int attr_types[src_nAttrs];

    for (int i = 0; i < src_nAttrs; i++)
    {
        AttrCatEntry attrCatEntry;
        AttrCacheTable::getAttrCatEntry(srcRelId, i, &attrCatEntry);
        strcpy(attr_names[i], attrCatEntry.attrName);
        attr_types[i] = attrCatEntry.attrType;
    }

    int ret = Schema::createRel(targetRel, src_nAttrs, attr_names, attr_types);
    if (ret < 0)
    {
        return ret;
    }

    int targetRelId = OpenRelTable::openRel(targetRel);
    if (targetRelId < 0)
    {
        Schema::deleteRel(targetRel);
        return targetRelId;
    }

    Attribute record[src_nAttrs];
    RelCacheTable::resetSearchIndex(srcRelId);
    AttrCacheTable::resetSearchIndex(srcRelId, attr);
    Algebra::ncmps = 0;
    int nrecs = 0;
    while (BlockAccess::search(srcRelId, record, attr, attrVal, op) == SUCCESS)
    {
        ret = BlockAccess::insert(targetRelId, record);
        nrecs++;
        if (ret != SUCCESS)
        {
            Schema::closeRel(targetRel);
            Schema::deleteRel(targetRel);
            return ret;
        }
    }
    Schema::closeRel(targetRel);
    printf("Number of comparisions: %lld\n", Algebra::ncmps);
    printf("Number of records selected : %d\n", nrecs);

    return SUCCESS;
}

int Algebra::project(char srcRel[ATTR_SIZE], char targetRel[ATTR_SIZE])
{
    int srcRelId = OpenRelTable::getRelId(srcRel);
    if (srcRelId == E_RELNOTOPEN)
    {
        return E_RELNOTOPEN;
    }

    RelCatEntry relCatEntry;
    RelCacheTable::getRelCatEntry(srcRelId, &relCatEntry);
    int nAttrs = relCatEntry.numAttrs;

    char attrNames[nAttrs][ATTR_SIZE];
    int attrTypes[nAttrs];
    for (int i = 0; i < nAttrs; i++)
    {
        AttrCatEntry attrCatEntry;
        AttrCacheTable::getAttrCatEntry(srcRelId, i, &attrCatEntry);
        strcpy(attrNames[i], attrCatEntry.attrName);
        attrTypes[i] = attrCatEntry.attrType;
    }

    int ret = Schema::createRel(targetRel, nAttrs, attrNames, attrTypes);
    if (ret < 0)
    {
        return ret;
    }
    int targetRelId = OpenRelTable::openRel(targetRel);
    if (targetRelId < 0)
    {
        Schema::deleteRel(targetRel);
        return targetRelId;
    }

    RelCacheTable::resetSearchIndex(srcRelId);
    Attribute record[nAttrs];

    while (BlockAccess::project(srcRelId, record) == SUCCESS)
    {
        ret = BlockAccess::insert(targetRelId, record);
        if (ret != SUCCESS)
        {
            Schema::closeRel(targetRel);
            Schema::deleteRel(targetRel);
            return ret;
        }
    }
    Schema::closeRel(targetRel);

    return SUCCESS;
}

int Algebra::project(char srcRel[ATTR_SIZE], char targetRel[ATTR_SIZE], int tar_nAttrs, char tar_Attrs[][ATTR_SIZE])
{

    int srcRelId = OpenRelTable::getRelId(srcRel);
    if (srcRelId == E_RELNOTOPEN)
    {
        return E_RELNOTOPEN;
    }

    RelCatEntry relCatEntry;
    RelCacheTable::getRelCatEntry(srcRelId, &relCatEntry);
    int src_nAttrs = relCatEntry.numAttrs;

    int attr_offset[tar_nAttrs];
    int attr_types[tar_nAttrs];

    for (int i = 0; i < tar_nAttrs; i++)
    {
        AttrCatEntry attrCatEntry;
        if (AttrCacheTable::getAttrCatEntry(srcRelId, tar_Attrs[i], &attrCatEntry) != SUCCESS)
        {
            return E_ATTRNOTEXIST;
        }
        attr_offset[i] = attrCatEntry.offset;
        attr_types[i] = attrCatEntry.attrType;
    }

    int ret = Schema::createRel(targetRel, tar_nAttrs, tar_Attrs, attr_types);
    if (ret < 0)
    {
        return ret;
    }
    int targetRelId = OpenRelTable::openRel(targetRel);
    if (targetRelId < 0)
    {
        Schema::deleteRel(targetRel);
        return targetRelId;
    }

    RelCacheTable::resetSearchIndex(srcRelId);
    Attribute record[src_nAttrs];

    while (BlockAccess::project(srcRelId, record) == SUCCESS)
    {
        Attribute proj_record[tar_nAttrs];
        for (int i = 0; i < tar_nAttrs; i++)
        {
            proj_record[i] = record[attr_offset[i]];
        }

        ret = BlockAccess::insert(targetRelId, proj_record);
        if (ret != SUCCESS)
        {
            Schema::closeRel(targetRel);
            Schema::deleteRel(targetRel);
            return ret;
        }
    }
    Schema::closeRel(targetRel);

    return SUCCESS;
}

#if 0
int Algebra::print(char srcRel[ATTR_SIZE])
{
    int srcRelId = OpenRelTable::getRelId(srcRel);
    if (srcRelId == E_RELNOTOPEN)
    {
        return E_RELNOTOPEN;
    }

    AttrCatEntry attrCatEntry;
    int attrCat = AttrCacheTable::getAttrCatEntry(srcRelId, attr, &attrCatEntry);
    if (attrCat == E_ATTRNOTEXIST)
    {
        return E_ATTRNOTEXIST;
    }

    int type = attrCatEntry.attrType;
    Attribute attrVal;
    if (type == NUMBER)
    {
        bool isNumber(char *);
        if (isNumber(strVal))
        {
            attrVal.nVal = atof(strVal);
        }
        else
        {
            return E_ATTRTYPEMISMATCH;
        }
    }
    else if (type == STRING)
    {
        strcpy(attrVal.sVal, strVal);
    }

    /*** Selecting records from the source relation ***/

    RelCacheTable::resetSearchIndex(srcRelId);
    RelCatEntry relCatEntry;
    RelCacheTable::getRelCatEntry(srcRelId, &relCatEntry);

    int attrtype[relCatEntry.numAttrs];
    printf("|");
    for (int i = 0; i < relCatEntry.numAttrs; ++i)
    {
        AttrCatEntry attrCatEntry;
        AttrCacheTable::getAttrCatEntry(srcRelId, i, &attrCatEntry);
        attrtype[i] = attrCatEntry.attrType;
        printf(" %s |", attrCatEntry.attrName);
    }
    printf("\n");

    while (true)
    {
        RecId searchRes = BlockAccess::linearSearch(srcRelId, attr, attrVal, op);
        if (searchRes.block != -1 && searchRes.slot != -1)
        {
            RecBuffer BlockBuffer(searchRes.block);
            Attribute rec[relCatEntry.numAttrs];
            BlockBuffer.getRecord(rec, searchRes.slot);

            printf("|");
            for (int i = 0; i < relCatEntry.numAttrs; i++)
            {
                if (attrtype[i] == NUMBER)
                    printf(" %.2f |", rec[i].nVal);
                else
                    printf(" %s |", rec[i].sVal);
            }
            printf("\n");
        }
        else
        {
            break;
        }
    }

    return SUCCESS;
}
#endif

// will return if a string can be parsed as a floating point number
bool isNumber(char *str)
{
    int len;
    float ignore;
    /*****************************************************************************
      sscanf returns the number of elements read, so if there is no float matching
      the first %f, ret will be 0, else it'll be 1

      %n gets the number of characters read. this scanf sequence will read the
      first float ignoring all the whitespace before and after. and the number of
      characters read that far will be stored in len. if len == strlen(str), then
      the string only contains a float with/without whitespace. else, there's other
      characters.
    *****************************************************************************/
    int ret = sscanf(str, "%f %n", &ignore, &len);
    return ret == 1 && len == strlen(str);
}

int Algebra::insert(char relName[ATTR_SIZE], int nAttrs, char record[][ATTR_SIZE])
{
    if (strcmp(relName, RELCAT_RELNAME) == 0 || strcmp(relName, ATTRCAT_RELNAME) == 0)
        return E_NOTPERMITTED;

    int relId = OpenRelTable::getRelId(relName);
    if (relId == E_RELNOTOPEN)
        return E_RELNOTOPEN;

    RelCatEntry relCatEntry;
    RelCacheTable::getRelCatEntry(relId, &relCatEntry);

    if (relCatEntry.numAttrs != nAttrs)
        return E_NATTRMISMATCH;

    union Attribute recordValues[nAttrs];

    for (int i = 0; i < nAttrs; ++i)
    {
        AttrCatEntry attrCatEntry;
        AttrCacheTable::getAttrCatEntry(relId, i, &attrCatEntry);
        int type = attrCatEntry.attrType;

        if (type == NUMBER)
        {
            if (isNumber(record[i]))
            {
                recordValues[i].nVal = atof(record[i]);
            }
            else
            {
                return E_ATTRTYPEMISMATCH;
            }
        }
        else if (type == STRING)
        {
            strcpy(recordValues[i].sVal, record[i]);
        }
    }

    int retVal = BlockAccess::insert(relId, recordValues);

    return retVal;
}

int Algebra::join(char srcRelation1[ATTR_SIZE], char srcRelation2[ATTR_SIZE], char targetRelation[ATTR_SIZE], char attribute1[ATTR_SIZE], char attribute2[ATTR_SIZE])
{
    int srcRelId1 = OpenRelTable::getRelId(srcRelation1);
    int srcRelId2 = OpenRelTable::getRelId(srcRelation2);
    if (srcRelId1 == E_RELNOTOPEN || srcRelId2 == E_RELNOTOPEN)
        return E_RELNOTOPEN;

    AttrCatEntry attrCatEntry1, attrCatEntry2;
    int ret1 = AttrCacheTable::getAttrCatEntry(srcRelId1, attribute1, &attrCatEntry1);
    int ret2 = AttrCacheTable::getAttrCatEntry(srcRelId2, attribute2, &attrCatEntry2);
    if (ret1 == E_ATTRNOTEXIST || ret2 == E_ATTRNOTEXIST)
        return E_ATTRNOTEXIST;
    if (attrCatEntry1.attrType != attrCatEntry2.attrType)
        return E_ATTRTYPEMISMATCH;

    RelCatEntry relCatEntry1, relCatEntry2;
    RelCacheTable::getRelCatEntry(srcRelId1, &relCatEntry1);
    RelCacheTable::getRelCatEntry(srcRelId2, &relCatEntry2);
    int numOfAttributes1 = relCatEntry1.numAttrs;
    int numOfAttributes2 = relCatEntry2.numAttrs;

    for (int i = 0; i < numOfAttributes1; i++)
    {
        AttrCatEntry entry1, entry2;
        AttrCacheTable::getAttrCatEntry(srcRelId1, i, &entry1);
        if (AttrCacheTable::getAttrCatEntry(srcRelId2, entry1.attrName, &entry2) == SUCCESS)
        {
            if (!(entry1.offset == attrCatEntry1.offset && entry2.offset == attrCatEntry2.offset))
                return E_DUPLICATEATTR;
        }
    }

    if (attrCatEntry2.rootBlock == -1)
    {
        int ret = BPlusTree::bPlusCreate(srcRelId2, attribute2);
        if (ret != SUCCESS)
            return ret;
    }

    int numOfAttributesInTarget = numOfAttributes1 + numOfAttributes2 - 1;

    char targetRelAttrNames[numOfAttributesInTarget][ATTR_SIZE];
    int targetRelAttrTypes[numOfAttributesInTarget];

    int k = 0;
    for (int i = 0; i < numOfAttributes1; i++)
    {
        AttrCatEntry attrCatEntry;
        AttrCacheTable::getAttrCatEntry(srcRelId1, i, &attrCatEntry);
        strcpy(targetRelAttrNames[k], attrCatEntry.attrName);
        targetRelAttrTypes[k] = attrCatEntry.attrType;
        k++;
    }
    for (int i = 0; i < numOfAttributes2; i++)
    {
        AttrCatEntry attrCatEntry;
        AttrCacheTable::getAttrCatEntry(srcRelId2, i, &attrCatEntry);
        if (i != attrCatEntry2.offset)
        {
            strcpy(targetRelAttrNames[k], attrCatEntry.attrName);
            targetRelAttrTypes[k] = attrCatEntry.attrType;
            k++;
        }
    }

    int ret = Schema::createRel(targetRelation, numOfAttributesInTarget, targetRelAttrNames, targetRelAttrTypes);
    if (ret != SUCCESS)
        return ret;

    int targetRelId = OpenRelTable::openRel(targetRelation);
    if (targetRelId < 0)
    {
        Schema::deleteRel(targetRelation);
        return targetRelId;
    }

    Attribute record1[numOfAttributes1];
    Attribute record2[numOfAttributes2];
    Attribute targetRecord[numOfAttributesInTarget];

    RelCacheTable::resetSearchIndex(srcRelId1);
    while (BlockAccess::project(srcRelId1, record1) == SUCCESS)
    {
        RelCacheTable::resetSearchIndex(srcRelId2);
        AttrCacheTable::resetSearchIndex(srcRelId2, attribute2);

        int k1 = 0;
        for (int i = 0; i < numOfAttributes1; i++)
        {
            targetRecord[k1++] = record1[i];
        }
        while (BlockAccess::search(srcRelId2, record2, attribute2, record1[attrCatEntry1.offset], EQ) == SUCCESS)
        {
            int k2 = k1;
            for (int i = 0; i < numOfAttributes2; i++)
            {
                if (i != attrCatEntry2.offset)
                {
                    targetRecord[k2++] = record2[i];
                }
            }

            int ret = BlockAccess::insert(targetRelId, targetRecord);
            if (ret != SUCCESS)
            {
                OpenRelTable::closeRel(targetRelId);
                Schema::deleteRel(targetRelation);
                return E_DISKFULL;
            }
        }
    }

    OpenRelTable::closeRel(targetRelId);
    return SUCCESS;
}