#include "Buffer/StaticBuffer.h"
#include "Cache/OpenRelTable.h"
#include "Disk_Class/Disk.h"
#include "FrontendInterface/FrontendInterface.h"
#include <iostream>
#include <cstring>

void stage1()
{
	unsigned char buffer[BLOCK_SIZE];
	Disk::readBlock(buffer, 7000);
	char message[] = "hello";
	memcpy(buffer + 20, message, 6);
	Disk::writeBlock(buffer, 7000);

	unsigned char buffer1[BLOCK_SIZE];
	Disk::readBlock(buffer1, 6000);
	char message1[] = "world";
	memcpy(buffer1 + 20, message1, 6);
	Disk::writeBlock(buffer1, 6000);

	unsigned char buffer2[BLOCK_SIZE];
	char message2[6];
	// Disk::readBlock(buffer2, 6000);
	Disk::readBlock(buffer2, 7000);
	memcpy(message2, buffer2 + 20, 6);
	std::cout << message2 << '\n';
}
void stage1ex()
{
	unsigned char buffer[BLOCK_SIZE];
	Disk::readBlock(buffer, 0);
	for (int i = 0; i < 9; i++)
		std::cout << (int)*(buffer + i) << ',';
	std::cout << (int)buffer[10] << '\n';
}

void stage2()
{
	RecBuffer relCatBuffer(RELCAT_BLOCK);
	RecBuffer attrCatBuffer(ATTRCAT_BLOCK);

	HeadInfo relCatHeader;
	HeadInfo attrCatHeader;

	relCatBuffer.getHeader(&relCatHeader);
	attrCatBuffer.getHeader(&attrCatHeader);

	for (int i = 0; i < relCatHeader.numEntries; i++)
	{
		Attribute relCatRecord[RELCAT_NO_ATTRS];
		relCatBuffer.getRecord(relCatRecord, i);
		printf("Relation: %s\n", relCatRecord[RELCAT_REL_NAME_INDEX].sVal);

		for (int j = 0; j < attrCatHeader.numEntries; j++)
		{
			Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
			attrCatBuffer.getRecord(attrCatRecord, j);
			if (strcmp(attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal, relCatRecord[RELCAT_REL_NAME_INDEX].sVal) == 0)
			{
				const char *attrType = attrCatRecord[ATTRCAT_ATTR_TYPE_INDEX].nVal == NUMBER ? "NUM" : "STR";
				printf(" %s: %s\n", attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, attrType);
			}
		}
		printf("\n");
	}
}
void stage2ex1()
{
	RecBuffer relCatBuffer(RELCAT_BLOCK);
	RecBuffer attrCatBuffer(ATTRCAT_BLOCK);

	HeadInfo relCatHeader;
	HeadInfo attrCatHeader;

	relCatBuffer.getHeader(&relCatHeader);
	attrCatBuffer.getHeader(&attrCatHeader);

	for (int i = 0; i < relCatHeader.numEntries; i++)
	{
		Attribute relCatRecord[RELCAT_NO_ATTRS];
		relCatBuffer.getRecord(relCatRecord, i);
		printf("Relation: %s\n", relCatRecord[RELCAT_REL_NAME_INDEX].sVal);

		int currBlock = ATTRCAT_BLOCK;
		while (currBlock != -1)
		{
			RecBuffer attrCatBuffer(currBlock);
			attrCatBuffer.getHeader(&attrCatHeader);
			for (int j = 0; j < attrCatHeader.numEntries; j++)
			{
				Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
				attrCatBuffer.getRecord(attrCatRecord, j);
				if (strcmp(attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal, relCatRecord[RELCAT_REL_NAME_INDEX].sVal) == 0)
				{
					const char *attrType = attrCatRecord[ATTRCAT_ATTR_TYPE_INDEX].nVal == NUMBER ? "NUM" : "STR";
					printf(" %s: %s\n", attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, attrType);
				}
			}
			currBlock = attrCatHeader.rblock;
		}
		printf("\n");
	}
}
void stage2ex2()
{
	RecBuffer attrCatBuffer(ATTRCAT_BLOCK);
	HeadInfo attrCatHeader;

	int currBlock = ATTRCAT_BLOCK;
	int replaced = 0;
	while (currBlock != -1)
	{
		RecBuffer attrCatBuffer(currBlock);
		attrCatBuffer.getHeader(&attrCatHeader);
		for (int i = 0; i < attrCatHeader.numEntries; i++)
		{
			Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
			attrCatBuffer.getRecord(attrCatRecord, i);
			if (strcmp(attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal, "Students") == 0 && strcmp(attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, "Class") == 0)
			{
				strcpy(attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, "Batch");
				attrCatBuffer.setRecord(attrCatRecord, i);
				replaced = 1;
				break;
			}
		}
		if (replaced)
			break;
		currBlock = attrCatHeader.rblock;
	}

	currBlock = ATTRCAT_BLOCK;
	printf("Relation: Students\n");
	while (currBlock != -1)
	{
		RecBuffer attrCatBuffer(currBlock);
		attrCatBuffer.getHeader(&attrCatHeader);
		for (int i = 0; i < attrCatHeader.numEntries; i++)
		{
			Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
			attrCatBuffer.getRecord(attrCatRecord, i);
			if (strcmp(attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal, "Students") == 0)
			{
				const char *attrType = attrCatRecord[ATTRCAT_ATTR_TYPE_INDEX].nVal == NUMBER ? "NUM" : "STR";
				printf(" %s: %s\n", attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, attrType);
			}
		}
		currBlock = attrCatHeader.rblock;
	}
}

void stage3()
{
	for (int i = RELCAT_RELID; i <= ATTRCAT_RELID; i++) // i = 0 and i = 1 (i.e RELCAT_RELID and ATTRCAT_RELID)
	{
		// get the relation catalog entry using RelCacheTable::getRelCatEntry()
		RelCatEntry relCatBuf;
		RelCacheTable::getRelCatEntry(i, &relCatBuf);
		printf("Relation: %s\n", relCatBuf.relName);

		for (int j = 0; j < relCatBuf.numAttrs; j++)
		{
			// get the attribute catalog entry for (rel-id i, attribute offset j) in attrCatEntry using AttrCacheTable::getAttrCatEntry()
			AttrCatEntry attrCatBuf;
			AttrCacheTable::getAttrCatEntry(i, j, &attrCatBuf);
			const char *attrType = attrCatBuf.attrType == NUMBER ? "NUM" : "STR";
			printf("  %s: %s\n", attrCatBuf.attrName, attrType);
		}
		printf("\n");
	}
}
void stage3ex()
{

	for (int i = 0; i < MAX_OPEN; i++)
	{
		RelCatEntry relCatBuf;
		RelCacheTable::getRelCatEntry(i, &relCatBuf);
		if (strcmp(relCatBuf.relName, "Students") == 0)
		{
			printf("Relation: %s\n", relCatBuf.relName);
			for (int j = 0; j < relCatBuf.numAttrs; j++)
			{
				AttrCatEntry attrCatBuf;
				AttrCacheTable::getAttrCatEntry(i, j, &attrCatBuf);
				const char *attrType = attrCatBuf.attrType == NUMBER ? "NUM" : "STR";
				printf("  %s: %s\n", attrCatBuf.attrName, attrType);
			}
			printf("\n");
			break;
		}
	}
}

void stage4()
{
	/* Run the following commands:
	SELECT * FROM ATTRIBUTECAT INTO null WHERE RelName=RELATIONCAT;
	SELECT * FROM RELATIONCAT INTO null WHERE #Records>10;
	*/
}
void stage4ex1()
{
	/* After Inserting Records into Students Relation, run the following commands:
	SELECT * FROM Students INTO null WHERE Batch=J;
	SELECT * FROM Students INTO null WHERE Batch!=J;
	SELECT * FROM Students INTO null WHERE Marks>=90;
	*/
}
void stage4ex2()
{
	/* Run the following commands and ensure that you get the corresponding output as shown:
	SELECT * FROM RELATIONCAT INTO null WHERE #Records > five;
	SELECT * FROM RELATIONCAT INTO null WHERE Name = Students;
	SELECT * FROM Participants INTO null WHERE regNo>0;

	# Error: Mismatch in attribute type
	# Error: Attribute does not exist
	# Error: Relation is not open
	*/
}

void stage5ex1()
{
	/*
	OPEN TABLE Students;
	SELECT * FROM Students INTO null WHERE Batch=J;
	CLOSE TABLE Students;
	*/
}
void stage5ex2()
{
	/*
	SELECT * FROM Events INTO null WHERE id>0;
	SELECT * FROM Locations INTO null WHERE name!=none;
	SELECT * FROM Participants INTO null WHERE regNo>0;
	*/
}
void stage5ex3()
{
	/*
	open table x;
	open table a;
	open table b;
	open table c;
	open table d;
	open table e;
	open table f;
	open table g;
	open table h;
	open table i;
	open table j;
	open table k;
	close table k;
	close table j;
	open table k;

	# Error: Relation does not exist
	# Relation a opened successfully
	# Relation b opened successfully
	# Relation c opened successfully
	# Relation d opened successfully
	# Relation e opened successfully
	# Relation f opened successfully
	# Relation g opened successfully
	# Relation h opened successfully
	# Relation i opened successfully
	# Relation j opened successfully
	# Error: Cache is full
	# Error: Relation is not open
	# Relation j closed successfully
	# Relation k opened successfully
	*/
}

void stage6ex1()
{
	/*
	ALTER TABLE RENAME Books TO LibraryBooks;
	ALTER TABLE RENAME LibraryBooks COLUMN bookOwner TO lender;
	OPEN TABLE LibraryBooks;
	SELECT * FROM LibraryBooks INTO null WHERE price>0;
	exit
	*/
}
void stage6ex2()
{
	/*
	alter table rename LibBooks to Books;
	alter table rename LibraryBooks to Students;
	alter table rename LibraryBooks to RELATIONCAT;
	open table LibraryBooks;
	alter table rename LibraryBooks to LibBooks;

	# Error: Relation does not exist
	# Error: Relation already exists
	# Error: This operation is not permitted
	# Relation LibraryBooks opened successfully
	# Error: Relation is open
	*/
}

void stage7ex1()
{
	/*
	OPEN TABLE Locations;

	INSERT INTO Locations VALUES (elhc, 300);
	INSERT INTO Locations VALUES (nlhc, 300);
	INSERT INTO Locations VALUES (eclc, 500);
	INSERT INTO Locations VALUES (pits, 150);
	INSERT INTO Locations VALUES (oat, 950);
	INSERT INTO Locations VALUES (audi, 1000);

	SELECT * FROM Locations INTO null WHERE capacity>0;
	CLOSE TABLE Locations;
	SELECT * FROM RELATIONCAT INTO null WHERE RelName=Locations;

	CLOSE TABLE -> # required to update the relation catalog
	*/
}
void stage7ex2()
{
	/*
	OPEN TABLE Events;
	INSERT INTO Events VALUES FROM events.csv;
	SELECT * FROM Events INTO null WHERE id>0;
	CLOSE TABLE Events;
	*/
}
void stage7ex3()
{
	/*
	insert into Participants values (43, Ragam, ELHC);
	insert into Participants values (four, Ragam);
	insert into RELATIONCAT values (test);

	# Error: Mismatch in number of attributes
	# Error: Mismatch in attribute type
	# Error: This operation is not permitted
	*/
}

void stage8ex1()
{
	/*
	run s8text.txt;
	*/
}
void stage8ex2()
{
	/*
	dump relcat
	dump attrcat
	print table Stores
	*/
}
void stage8ex3()
{
	/*
	create table Stores(name STR);
	create table People(name NUM, name STR);
	open table Products;
	open table Stores;
	drop table Stores;
	drop table RELATIONCAT;
	close table Stores;
	drop table Stores;

	# Error: Relation already exists
	# Error: Duplicate attributes found
	# Error: Relation does not exist
	# Relation Stores opened successfully
	# Error: Relation is open
	# Error: This operation is not permitted
	# Relation Stores closed successfully
	# Relation Stores deleted successfully
	*/
}

int main(int argc, char *argv[])
{
	/* Initialize the Run Copy of Disk */
	Disk disk_run;
	StaticBuffer buffer;
	OpenRelTable cache;

	// stage1();     /* Saving data in disk and Loading data from disk */
	// stage1ex();   /* Print values in Block_Allocation_Map */

	// stage2();     /* Printing all the relations and their attributes (first block of ATTRCAT) */
	// stage2ex1();  /* Printing all the relations and their attributes (multiple blocks of ATTRCAT) */
	// stage2ex2();  /* Changing the attribute name in a relation */

	// stage3();     /* Printing RelCat and AttrCat relation attributes from Cache */
	// stage3ex();   /* Printing Students relation attributes from Cache */

	// stage4();     /* Printing all Attrs from <tablename> where <condition> */
	// stage4ex1();  /* Printing all Attrs from <tablename> where <condition> */
	// stage4ex2();  /* Checking Error Conditions */

	// stage5ex1();  /* Testing OPEN, SELECT & CLOSE */
	// stage5ex2();  /* OPENing multiple relations */
	// stage5ex3();  /* Filling the Cache */

	// stage6ex1();  /* Testing ALTER TABLE RENAME and RENAME COLUMN name */
	// stage6ex2();  /* Checking Error Conditions */

	// stage7ex1();  /* Testing INSERT INTO table VALUES */
	// stage7ex2();  /* Testing INSERT INTO table VALUES FROM file */
	// stage7ex3();  /* Checking Error Conditions */

	// stage8ex1();  /* Testing CREATE TABLE and DROP TABLE from file using RUN */
	// stage8ex2();	 /* DUMPing RELCAT and ATTRCAT */
	// stage8ex3();  /* Checking Error Conditions */

	return FrontendInterface::handleFrontend(argc, argv);
	// return 0;
}
