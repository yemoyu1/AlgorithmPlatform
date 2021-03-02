// SensorAlgorithmDialog.cpp: 实现文件
//

#include "pch.h"
#include "AlgorithmPlatform.h"
#include "SensorAlgorithmDialog.h"
#include "afxdialogex.h"
#include "CSheet.h"
#include <time.h>
#include <stdio.h>
#include <windows.h>
#include<thread>

// CSensorAlgorithmDialog 对话框

IMPLEMENT_DYNAMIC(CSensorAlgorithmDialog, CDialogEx)

CSensorAlgorithmDialog::CSensorAlgorithmDialog(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SENSOR_ALGORITHM_DIALOG, pParent)
{

}

CSensorAlgorithmDialog::~CSensorAlgorithmDialog()
{
}

BOOL CSensorAlgorithmDialog::OnInitDialog() 
{
    CDialogEx::OnInitDialog();

    DWORD dwStyle = m_list.GetExtendedStyle();   //listcontrol部分  
    dwStyle |= LVS_EX_FULLROWSELECT;
    dwStyle |= LVS_EX_GRIDLINES;
    m_list.SetExtendedStyle(dwStyle);
    m_timecost = 0;
    ((CButton*)GetDlgItem(IDC_RADIO1))->SetCheck(BST_CHECKED);
    return true;
}
void CSensorAlgorithmDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST1, m_list);
}


BEGIN_MESSAGE_MAP(CSensorAlgorithmDialog, CDialogEx)
	ON_BN_CLICKED(ID_BTN_LOAD, &CSensorAlgorithmDialog::OnBnClickedBtnLoad)
	ON_BN_CLICKED(ID_BTN_CALCULATION, &CSensorAlgorithmDialog::OnBnClickedBtnCalculation)
END_MESSAGE_MAP()


// CSensorAlgorithmDialog 消息处理程序
void proc(CSensorAlgorithmDialog* _csad,CString szFileName)
{
    
        //TRACE(szFileName);  
        time_t time_seconds1 = time(0);
        struct tm now_time;
        localtime_s(&now_time, &time_seconds1);
        TRACE(_T("%d-%d-%d %d:%d:%d/n"), now_time.tm_year + 1900, now_time.tm_mon + 1, now_time.tm_mday, now_time.tm_hour, now_time.tm_min, now_time.tm_sec);

        CSheet sheet;

        ifstream in(szFileName);
        sheet.loadFrom(in);
         _csad->m_list.DeleteAllItems();
        while (_csad->m_list.DeleteColumn(0));
        for (int i = 0; i < sheet.getRowCount(); i++)
        {
            //_tprintf(_T("[%02d] "), i);
            //std::cout << i << " : ";
            CRow row = sheet.getRow(i);

            if (i > 0)
            {
                _csad->m_list.InsertItem(i - 1, row.getColumn(0), LVCFMT_CENTER);
            }
            for (int j = 0; j < row.getColumnCount(); j++)
            {

                /*CString s;
                s.Format(_T("%s \n"), row.getColumn(j));*/
                if (i > 0 && j > 1 && j < 14)
                {
                    _csad->data1[j - 2].push_back(_tstof(row.getColumn(j)));
                    //m_list.SetItemText(j-2, i, row.getColumn(j));
                }
                else if (i == 0 && j > 1 && j < 14)
                {
                    _csad->dataname[j - 2].push_back(row.getColumn(j));
                    //m_list.SetItemText(j-2, i, row.getColumn(j));
                }
                if (i == 0)
                {
                    _csad->m_list.InsertColumn(j, row.getColumn(j), LVCFMT_CENTER, 120);
                    if (j == row.getColumnCount() - 1)
                    {
                        _csad->m_list.InsertColumn(j + 1, _T("NO"), LVCFMT_CENTER, 120);
                    }
                }
                else
                {
                    _csad->m_list.SetItemText(i - 1, j, row.getColumn(j));
                    if (j == row.getColumnCount() - 1) {
                        CString _str;
                        _str.Format(_T("%d"), i);
                        _csad->m_list.SetItemText(i - 1, j + 1, _str);
                    }
                }
            }
        }
        //SaveCSV(szFileName,data1, dataname);

        time_t time_seconds2 = time(0);
        localtime_s(&now_time, &time_seconds2);
        TRACE(_T("%d-%d-%d %d:%d:%d /n"), now_time.tm_year + 1900, now_time.tm_mon + 1, now_time.tm_mday, now_time.tm_hour, now_time.tm_min, now_time.tm_sec);

        _csad->m_timecost = difftime(time_seconds2, time_seconds1);
        TRACE(_T("用时%f秒"), _csad->m_timecost);
    
}


//读取文件
void CSensorAlgorithmDialog::OnBnClickedBtnLoad()
{

    CFileDialog dlg(true, _T("*.csv"), _T("UranusData.csv"), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T("打开为(*.csv)|*.csv|所有文件(*.*)|*.*||"));

    if (dlg.DoModal() == IDOK)
    {
        CString szFileName = dlg.GetPathName();    //文件名  
        szFileName.ReleaseBuffer();
        SetDlgItemText(IDC_FILE_PATH, szFileName);
        thread th2(proc, this, szFileName);
        th2.detach();
    }
   
	
}

//开始计算
void CSensorAlgorithmDialog::OnBnClickedBtnCalculation()
{
    CString _str;
    _str.Format(_T("本次处理的数据总数%d个，帧数%d帧，计算次数总数%d次，计算时长%.02f秒"), data1[0].size()*12,data1[0].size(), data1[0].size(), m_timecost);
    int ret=MessageBox(_str, _T("提示"));
    if (ret == IDOK)
    {
        SaveCSV(dataname,data1);
    }
}



bool CSensorAlgorithmDialog::SaveCSV(vector<CString> _data[], vector < float> _data1[])
{   


    time_t time_seconds = time(0);
    CString _newName;
    _newName.Format(_T("UranusData_%d.csv"), time_seconds);
    
    CFileDialog dlg(false, _T("*.csv"), _newName, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T("保存为(*.csv)|*.csv|所有文件(*.*)|*.*||"));

    if (dlg.DoModal() == IDOK)
    {
        CString szFileName = dlg.GetPathName();    //文件名  
        szFileName.ReleaseBuffer();
        SetDlgItemText(IDC_FILE_PATH, szFileName);

        //判断文件是否存在
        if (PathFileExists(szFileName))//文件已经存在
        {
            ::DeleteFile(szFileName);
        }
        CStdioFile  clsFile;
        if (!clsFile.Open(szFileName, CFile::modeCreate | CFile::modeWrite))
        {

            return false;
        }
        CString strText;
        for (size_t i = 0; i < 12; i++)
        {
            CString s;
            if (i == 11)
            {
                s.Format(_T("%s"), _data[i][0]);
            }
            else
            {
                s.Format(_T("%s,"), _data[i][0]);
            }

            strText += s;
        }
        clsFile.WriteString(strText);
        strText = _T("\n");
        clsFile.WriteString(strText);
        strText = _T("");
        for (int k = 0; k < data1[0].size(); k++)
        {
            for (size_t i = 0; i < 12; i++)
            {
                CString s;
                if (i == 11)
                {
                    s.Format(_T("%f"), _data1[i][k]);
                }
                else
                {
                    s.Format(_T("%f,"), _data1[i][k]);
                }

                strText += s;
            }
            clsFile.WriteString(strText);
            strText = _T("\n");
            clsFile.WriteString(strText);
            strText = _T("");
        }
        
        clsFile.Close();
        TRACE(_T("导出到csv文件成功!"));
    }
    

    return true;
}