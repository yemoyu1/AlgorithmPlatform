#pragma once
#include <afxtempl.h>
#include <iostream>
#include <stdlib.h>
#include <fstream>

using namespace std;

class CRow
{
private:
    CStringArray* _row;
public:
    CRow(CStringArray* row);
    int getColumnCount(void);
    CString getColumn(int i);
};

class CSheet
{
private:
    CTypedPtrArray<CPtrArray, CStringArray*> _rows;
public:
    CSheet(void);
    ~CSheet(void);
    int loadFrom(ifstream& in);
    int getRowCount(void);
    CRow getRow(int i);
};

