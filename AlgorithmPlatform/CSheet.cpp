#include "pch.h"
#include "CSheet.h"


CRow::CRow(CStringArray* row)
{
    _row = row;
}

int CRow::getColumnCount(void)
{
    return _row->GetCount();
}

CString CRow::getColumn(int i)
{
    return _row->GetAt(i);
}



CSheet::CSheet()
{
}

CSheet::~CSheet()
{
    for (int i = 0; i < _rows.GetCount(); i++)
    {
        delete _rows.GetAt(i);
    }
}

int CSheet::loadFrom(ifstream& in)
{
    int lines = 0;

    while (!in.eof())
    {
        //读取其中的一行  
        char line[256] = { 0 };
        in.getline(line, 255);

        CString s(line);

        //空白行，跳过  
        if (s.IsEmpty())
            continue;

        //#为注释标记，跳过  
        if (s[0] == '#')
            continue;

        CStringArray* pRow = new CStringArray();
        int i = 0;
        CString token = s.Tokenize(_T(",\t"), i);
        while (token != _T(""))
        {
            pRow->Add(token);
            token = s.Tokenize(_T(",\t"), i);
        }

        _rows.Add(pRow);
        lines++;
    }

    return lines;
}

int CSheet::getRowCount(void)
{
    return _rows.GetCount();
}

CRow CSheet::getRow(int i)
{
    return CRow(_rows.GetAt(i));
}
