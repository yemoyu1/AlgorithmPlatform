// SportAlgorithm.cpp: 实现文件
//
//

#include "pch.h"
#include "AlgorithmPlatform.h"
#include "SportAlgorithm.h"
#include "afxdialogex.h"
#include "CSheet.h"
#include <time.h>
#include<thread>
#include <future>
#include <ATLComTime.h>

#include "stdio.h"
#include "stdlib.h"
#include <math.h> 


using namespace std;



//速度积分函数
double f(double t, double a,double ak_1,double _tk_1,double delta)
{
    double v;    
    v = (a - ak_1)* t / delta + ((delta * a) - (_tk_1 * a) + (_tk_1 * ak_1)) / delta;
    return (v);
}
//位移积分函数
double g(double t, double v, double _vk_1,double _tk_1, double delta)
{
    double s;                                  
    s = (v - _vk_1) * t / delta + ((delta * v) - (_tk_1 * v) + (_tk_1 * _vk_1)) / delta;
    return (s);
}
//东北天 X
double get_enu_x(double &a11,double &a12,double &a13,double _x, double _y, double _z)
{
    a11 = cos(_y) * cos(_z);
    a12 = cos(_y) * sin(_x);
    a13 = -sin(_y);
    double e_x1 = a11 *_x;
    double e_x2 = a12  * _y;
    double e_x3= a13 * _z;
    return e_x1+ e_x2+ e_x3;
}
//东北天 Y
double get_enu_y(double& a21, double& a22, double& a23, double _x, double _y, double _z)
{
    a21 = sin(_z) * sin(_y) * cos(_x) - cos(_z) * sin(_x);
    a22 = sin(_z) * sin(_y) * sin(_x) + cos(_z) * cos(_x);
    a23 = sin(_z) * cos(_y);
    double e_y1 = a21 * _x;
    double e_y2 = a22 * _y;
    double e_y3 = a23 * _z;
    return e_y1 + e_y2 + e_y3;
}
//东北天 Z
double get_enu_z(double& a31, double& a32, double& a33, double _x, double _y, double _z)
{
    a31 = cos(_z) * sin(_y) * cos(_x) + sin(_z) * sin(_x);
    a32 = cos(_z) * sin(_z) * sin(_x) - sin(_z) * cos(_x);
    a33 = cos(_z) * cos(_y);
    double e_z1 = a31 * _x;
    double e_z2 = a32 * _y;
    double e_z3 = a33 * _z;
    return e_z1 + e_z2 + e_z3;
}


//行列式 求绝对A
double get_A(double a11, double a12, double a13, 
    double a21, double a22, double a23,
    double a31, double a32, double a33)
{
    double A = (a11* a22* a33)+(a12*a23*a31)+(a13*a21*a32) -
        (a11*a23*a32)-(a12*a21*a33)-(a13*a22*a31);
    return A;
}

//矩阵转换伴随矩阵 a转换A  
void setMatrixConversionAdjoint(double a11, double a12, double a13, double a21, double a22, double a23,double a31, double a32, double a33,
    double& A11, double& A12, double& A13,double& A21, double& A22, double& A23,double& A31, double& A32, double& A33,
    double _A)
{
    //求伴随矩阵
    A11 = (a22 * a33) - (a23 * a32);
    A12 = (-a21 * a33) + (a23 * a31);
    A13 = (a21 * a32) - (a22 * a31);
    A21 = (-a12 * a33) + (a13 * a32);
    A22 = (a12 * a33) - (a13 * a31);
    A23 = (-a11 * a32) + (a12 * a31);
    A31 = (a12 * a23) - (a13 * a22);
    A32 = (-a11 * a23) + (a13 * a21);
    A33 = (a11 * a22) - (a12 * a21);

    //逆矩阵
    A11 /= _A;
    A12 /= _A;
    A13 /= _A;
    A21 /= _A;
    A22 /= _A;
    A23 /= _A;
    A31 /= _A;
    A32 /= _A;
    A33 /= _A;


}
//东北天坐标系 逆矩阵存放到par本体里
double get_enu_inverse_x(double A11, double A12, double A13, 
    double _x, double _y, double _z)
{
    double e_x1 = A11 * _x;
    double e_x2 = A12 * _y;
    double e_x3 = A13 * _z;
    return e_x1 + e_x2 + e_x3;
}

double get_enu_inverse_y(double A21, double A22, double A23,
    double _x, double _y, double _z )
{
    double e_y1 = A21 * _x;
    double e_y2 = A22 * _y;
    double e_y3 = A23 * _z;
    return e_y1 + e_y2 + e_y3;
}

double get_enu_inverse_z(double A31, double A32, double A33,
    double _x, double _y, double _z )
{
    double e_z1 = A31 * _x;
    double e_z2 = A32 * _y;
    double e_z3 = A33 * _z;
    return e_z1 + e_z2 + e_z3;
}

void Take_V(double _t, vecd _data2[], vecd _Par[])
{
    double K1, K2, K3, K4;
    for (size_t i = 0; i < 17; i++){_Par[i].clear(); }
    int _pedcount = 0;
    int _datfcount = 0;
    for (size_t k = 0; k < _data2[0].size(); k++)
    {
        //-----------------天秒时----------------------
        _Par[0].push_back(_data2[0][k]); 

        //----------------计算本体坐标系下加速度矢量长度-------------------
        double _a_length = sqrt(pow(_data2[1][k], 2.0) + pow(_data2[2][k], 2.0) + pow(_data2[3][k], 2.0));
        _Par[1].push_back(_a_length);
        //TRACE("_a_length=%f _data2_1=%f  _data2_2=%f _data2_3=%f \n", _a_length, pow(_data2[1][k], 2.0), pow(_data2[2][k], 2.0), pow(_data2[3][k], 2.0));
        
        //------------------从本体坐标系转换到东北天--------------------
        double a11, a12, a13, a21, a22, a23, a31, a32, a33;
        _Par[2].push_back(get_enu_x(a11, a12, a13,_data2[7][k], _data2[8][k], _data2[9][k]));
        _Par[3].push_back(get_enu_y(a21, a22, a23, _data2[7][k], _data2[8][k], _data2[9][k])-1); //直接补偿重力加速度
        _Par[4].push_back(get_enu_z(a31, a32, a33,_data2[7][k], _data2[8][k], _data2[9][k]));

        if (_Par[0].size() == 1)
        {
            for (size_t i = 5; i < 17; i++)
            {
                _Par[i].push_back(0.0f);

            }
            continue;
        }
        //------------------库塔法----------------------
        //-----------------速度积分 --------------等待算法确认-------
        for (size_t i = 1; i < 4; i++)
        {   //_data2[0][k] - _data2[0][0]
            K1 = _t * f(_data2[0][k], _data2[i][k], _data2[i][k - 1], _data2[0][k - 1],_t);
            K2 = _t * f(_data2[0][k] + 0.5 * _t, _data2[i][k] + 0.5 * K1, _data2[i][k - 1], _data2[0][k - 1],_t);
            K3 = _t * f(_data2[0][k] + 0.5 * _t, _data2[i][k] + K2, _data2[i][k - 1], _data2[0][k - 1],_t);
            K4 = _t * f(_data2[0][k] + _t, _data2[i][k] + K3, _data2[i][k - 1], _data2[0][k - 1],_t);
            double  _v = _data2[i][k] + (K1 + 2.0 * K2 + 2.0 * K3 + K4) / 6.0;
            _Par[4+i].push_back(_v);
        }        
        
        
        //--------------------位移积分 ------------使用_Par 的值 等待算法确认-------------
        for (size_t i = 5; i < 8; i++)
        {
            K1 = _t * g(_data2[0][k], _data2[i][k], _data2[i][k-1], _data2[0][k - 1],_t);
            K2 = _t * g(_data2[0][k] + 0.5 * _t, _data2[i][k] + 0.5 * K1, _data2[i][k - 1], _data2[0][k - 1],_t);
            K3 = _t * g(_data2[0][k] + 0.5 * _t, _data2[i][k] + K2, _data2[i][k - 1], _data2[0][k - 1],_t);
            K4 = _t * g(_data2[0][k] + _t, _data2[i][k] + K3, _data2[i][k - 1], _data2[0][k - 1],_t);
            double  _v = _data2[i][k] + (K1 + 2.0 * K2 + 2.0 * K3 + K4) / 6.0;
            _Par[3 + i].push_back(_v);
        }
        
        //---------将东北天坐标系下各坐标轴方向上的速度和位移转化到本地坐标系--------
        //三阶行列式 求绝对值 A
        double _A=get_A(a11, a12, a13, a21, a22, a23, a31, a32, a33);

        //伴随矩阵 a 转换到A
        double A11, A12, A13, A21, A22, A23, A31, A32, A33;
        setMatrixConversionAdjoint(a11, a12, a13, a21, a22, a23, a31, a32, a33,
            A11, A12, A13, A21, A22, A23, A31, A32, A33, _A);

        //求逆矩阵
        //速度
        _Par[11].push_back(get_enu_inverse_x(A11, A12, A13, _Par[5][k], _Par[6][k], _Par[7][k]));
        _Par[12].push_back(get_enu_inverse_y(A21, A22, A23, _Par[5][k], _Par[6][k], _Par[7][k]));
        _Par[13].push_back(get_enu_inverse_z(A31, A32, A33, _Par[5][k], _Par[6][k], _Par[7][k]));

        //位移
        _Par[14].push_back(get_enu_inverse_x(A11, A12, A13, _Par[8][k], _Par[9][k], _Par[10][k]));
        _Par[15].push_back(get_enu_inverse_y(A21, A22, A23, _Par[8][k], _Par[9][k], _Par[10][k]));
        _Par[16].push_back(get_enu_inverse_z(A31, A32, A33, _Par[8][k], _Par[9][k], _Par[10][k]));

        //-----------------------------------------------------------------        
    }
}

//计算步骤 
void CalculationSteps(vecd _Par[], vecd& _Ped, double _thres, double _Timespan)
{
    // data2 同屬一步的判斷
    for (size_t k = 0; k < _Par[0].size()-1; k++)
    {
        //跳过第一帧
        if (k == 0)
        {
            continue;
        }

        double _tk = 0.0f;
        if (_Ped.size() == 0)
        {
            _tk = _Par[0][0];
        }
        else
        {
            int idx=_Ped.size()-1;
            assert(idx< _Par[0].size());
            _tk = _Par[0][idx + 1];
        }
        if (_Par[1][k] - _Par[1][k - 1] < 0 && 
            _Par[1][k + 1] - _Par[1][k] > 0 &&
            _Par[1][k] <= _thres &&
            _Par[0][k] - _tk > _Timespan)
        {            
            _Ped.push_back(k);
        }
    }    
}
//求解每一步的运动参数
void getDatFData(vecd _Par[],vecd _Ped, vecd _DatF[])
{
    for (size_t j = 0; j < _Ped.size(); j++)
    {
        int _idx = _Ped[j];
        int i = 0;
        if (j > 0)
        {
            i = _Ped[j-1]+1;
        }
        double d1 = _Par[0][j];
        _DatF[0].push_back(d1);
        double d5 = sqrt(pow(_Par[14][i], 2.0) + pow(_Par[15][i], 2.0) + pow(_Par[16][i], 2.0));

        _DatF[4].push_back(d5);
        double d2 = 0.0,d3 = 0.0, d4 = 0.0;
        for (; i <= _idx; i++)
        {
            if(d2< _Par[14][i])
                d2=_Par[14][i];
            if (d3 < _Par[15][i])
                d3 =_Par[15][i];
            if (d4 < _Par[16][i])
                d4= _Par[16][i];
        }
        _DatF[1].push_back(d2);
        _DatF[2].push_back(d3);
        _DatF[3].push_back(d4);
    }
}


CString StringToDateTime_S(CString _cstr)
{
    int count = _cstr.Find('.');
    CString _datetime = _cstr.Left(count);
    //_cstr = _cstr.Left(count);
    _cstr.Delete(0, count);
    COleDateTime date_time;//sAi
    date_time.ParseDateTime(_datetime);

    int datetime = date_time.GetHour() * 3600;
    datetime += date_time.GetMinute() * 60;
    datetime += date_time.GetSecond();
    _datetime.Format(_T("%d%s"), datetime, _cstr);

    return _datetime;
}

double StringToDateTime_F(CString _cstr)
{
    int count = _cstr.Find('.');    
    CString _datetime= _cstr.Left(count);
    //_cstr = _cstr.Left(count);
    _cstr.Delete(0, count);
    COleDateTime date_time;//sAi
    date_time.ParseDateTime(_datetime);

    int datetime = date_time.GetHour() * 3600;
    datetime += date_time.GetMinute() * 60;
    datetime += date_time.GetSecond();
    _datetime.Format(_T("%d%s"), datetime, _cstr);

    return _tstof(_datetime);;
}

// CSportAlgorithm 对话框
IMPLEMENT_DYNAMIC(CSportAlgorithm, CDialogEx)

CSportAlgorithm::CSportAlgorithm(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SPORT_ALGORITHM_DIALOG, pParent)
    , sport_e1(0.2)
    , sport_e2(1.2)
    , sport_e3(-16)
    , sport_e4(16)
    , sport_e5(-2000)
    , sport_e6(2000)
    , sport_e7(0.05)
{

}

CSportAlgorithm::~CSportAlgorithm()
{
}


BOOL CSportAlgorithm::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    DWORD dwStyle = m_list.GetExtendedStyle();   //listcontrol部分  
    dwStyle |= LVS_EX_FULLROWSELECT;
    dwStyle |= LVS_EX_GRIDLINES;
    m_list.SetExtendedStyle(dwStyle);
    m_timecost = 0;
    ((CButton*)GetDlgItem(IDC_RADIO1))->SetCheck(BST_CHECKED);
    m_column_count = 10; //导出的列数 data2

    return true;
}

void CSportAlgorithm::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST1, m_list);
    DDX_Text(pDX, IDC_SPORT_EDIT1, sport_e1);
    DDX_Text(pDX, IDC_SPORT_EDIT2, sport_e2);
    DDX_Text(pDX, IDC_SPORT_EDIT3, sport_e3);
    DDX_Text(pDX, IDC_SPORT_EDIT4, sport_e4);
    DDX_Text(pDX, IDC_SPORT_EDIT5, sport_e5);
    DDX_Text(pDX, IDC_SPORT_EDIT6, sport_e6);
    DDX_Text(pDX, IDC_SPORT_EDIT7, sport_e7);
}


BEGIN_MESSAGE_MAP(CSportAlgorithm, CDialogEx)
	ON_BN_CLICKED(ID_BTN_LOAD, &CSportAlgorithm::OnBnClickedBtnLoad)
	ON_BN_CLICKED(ID_BTN_CALCULATION, &CSportAlgorithm::OnBnClickedBtnCalculation)
    ON_EN_CHANGE(IDC_SPORT_EDIT1, &CSportAlgorithm::OnEnChangeSportEdit)
    ON_EN_CHANGE(IDC_SPORT_EDIT2, &CSportAlgorithm::OnEnChangeSportEdit)
    ON_EN_CHANGE(IDC_SPORT_EDIT3, &CSportAlgorithm::OnEnChangeSportEdit)
    ON_EN_CHANGE(IDC_SPORT_EDIT4, &CSportAlgorithm::OnEnChangeSportEdit)
    ON_EN_CHANGE(IDC_SPORT_EDIT5, &CSportAlgorithm::OnEnChangeSportEdit)
    ON_EN_CHANGE(IDC_SPORT_EDIT6, &CSportAlgorithm::OnEnChangeSportEdit)
    ON_EN_CHANGE(IDC_SPORT_EDIT7, &CSportAlgorithm::OnEnChangeSportEdit)
    ON_BN_CLICKED(ID_BTN_CALCULATION2, &CSportAlgorithm::OnBnClickedBtnCalculation2)
    
END_MESSAGE_MAP()


// CSportAlgorithm 消息处理程序
//算法1 
int proc(CSportAlgorithm* _csad, CString szFileName,int _column_count)
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
    for (int i = 0; i < _csad->m_column_count; i++)
    {
        _csad->data1[i].clear();

    }
    for (int i = 0; i < sheet.getRowCount(); i++)
    {
        CRow row = sheet.getRow(i);
        if (i > 0)
        {
            _csad->m_list.InsertItem(i - 1, row.getColumn(0), LVCFMT_CENTER);
        }
        int _data_j = 0;
        int _list_j = 0;
        for (int j = 0; j < row.getColumnCount(); j++)
        {
          
            /*CString s;
            s.Format(_T("%s \n"), row.getColumn(j));*/

            if (i == 0)
            {               
                  _csad->dataname[_data_j].push_back(row.getColumn(j));    
                  _data_j++;
            }
            else
            {
                if (j == 1)
                {
                    _csad->data1[_data_j].push_back(StringToDateTime_F(row.getColumn(j)));
                    _data_j++;
                }
                else if((j!=0 && j!=  row.getColumnCount()))
                {
                    _csad->data1[_data_j].push_back(_tstof(row.getColumn(j)));
                    _data_j++;
                }
                
            }
       
            if (i == 0)
            {
                if (j == 0)
                {
                    _csad->m_list.InsertColumn(_list_j, _T("NO"), LVCFMT_CENTER, 120);
                    _list_j++;
                }
                _csad->m_list.InsertColumn(_list_j, row.getColumn(j), LVCFMT_CENTER, 120);
            }
            else
            {
                if (j == 0)
                {
                    CString _str;
                    _str.Format(_T("%d"), i);
                    _csad->m_list.SetItemText(i - 1, _list_j, _str);
                    _list_j++;
                }
                if (j==1)
                {    //_T("2021-02-18 11:32:24")
                    _csad->m_list.SetItemText(i - 1, _list_j, StringToDateTime_S(row.getColumn(j)));
                }
                else
                {
                    _csad->m_list.SetItemText(i - 1, _list_j, row.getColumn(j));
                }                
            }            
            _list_j++;
        }
    }
    //SaveCSV(szFileName,data1, dataname);

    time_t time_seconds2 = time(0);
    localtime_s(&now_time, &time_seconds2);
    TRACE(_T("%d-%d-%d %d:%d:%d /n"), now_time.tm_year + 1900, now_time.tm_mon + 1, now_time.tm_mday, now_time.tm_hour, now_time.tm_min, now_time.tm_sec);

    _csad->m_timecost = difftime(time_seconds2, time_seconds1);
    //TRACE(_T("用时%f秒"), _csad->m_timecost);
    _csad->MessageBox(_T("导入成功"), _T("提示"));
    return 1;
}


//读取文件
void CSportAlgorithm::OnBnClickedBtnLoad()
{

   /* connect();

    while (true)
    {
        TRACE("MAIN  THREAD! \n");
        Sleep(11);
    }*/
    CFileDialog dlg(true, _T("*.csv"), _T("UranusData.csv"), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T("打开为(*.csv)|*.csv|所有文件(*.*)|*.*||"));

    if (dlg.DoModal() == IDOK)
    {
        CString szFileName = dlg.GetPathName();    //文件名  
        szFileName.ReleaseBuffer();
        SetDlgItemText(IDC_FILE_PATH, szFileName);
        //thread th2(proc, this, szFileName);     
        //th2.detach();
        //th2.join();
        std::future<int> fu = std::async(std::launch::deferred, proc, this, szFileName, m_column_count);
        std::chrono::milliseconds span(1000);
        //while (fu.wait_for(span) != std::future_status::ready)
        //    TRACE(_T(". \n"));
        ////std::cout << std::endl;
        //TRACE(_T("&d"),fu.get());

        fu.wait();
    }

}

//开始计算
void CSportAlgorithm::OnBnClickedBtnCalculation()
{
    CString _str;
    _str.Format(_T("本次处理的数据总数%d个，帧数%d帧，计算次数总数%d次，计算时长%.02f秒"), data1[0].size() * 12, data1[0].size(), data1[0].size(), m_timecost);
    //int ret = MessageBox(_str, _T("提示"));
    //if (ret == IDOK)
    {
        isWildValue();
        ResultToList();
        
    }
}


bool CSportAlgorithm::SaveData2CSV(vecN _data[], vecd _data1[])
{
    time_t time_seconds = time(0);
    CString _newName;
    _newName.Format(_T("WITData2_%d.csv"), time_seconds);
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
        //setlocale(LC_CTYPE, ("chs"));
        CString strText;
        /*
        for (size_t i = 0; i < m_column_count; i++)
        {
            CString s;        
            if (i == 0)
                s.Format(_T("%s"), _data[i][0]);
            else
                s.Format(_T("	%s"), _data[i][0]);
            strText += s;
        }
        clsFile.WriteString(strText);
        strText = _T("\n");
        clsFile.WriteString(strText);        
        strText = _T("");
        */
        for (size_t k = 0; k < _data1[0].size(); k++)
        {
            for (int i = 0; i < m_column_count; i++)
            {
                CString s;               
                if (i == 0)
                {
                    //s.Format(_T("%s"), m_list.GetItemText(k,2));
                    s.Format(_T("%.03f"), _data1[i][k]);
                }
                else
                {
                    s.Format(_T("	,%.03f"), _data1[i][k]);
                }
                strText += s;
            }
            clsFile.WriteString(strText);
            strText = _T("\n");
            clsFile.WriteString(strText);
            strText = _T("");
        }

        clsFile.Close();
        //TRACE(_T("导出到csv文件成功!"));
       // MessageBox(_T("导出到csv文件成功!"), _T("提示"));
    }


    return true;
}



bool CSportAlgorithm::SaveParCSV(vecd _data[])
{
    time_t time_seconds = time(0);
    CString _newName;
    _newName.Format(_T("WITPar_%d.csv"), time_seconds);
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
        //setlocale(LC_CTYPE, ("chs"));
        CString strText;
        
        for (size_t k = 0; k < _data[0].size(); k++)
        {
            for (int i = 0; i < 17; i++)
            {
                CString s;
                if (i == 0)
                {
                    //s.Format(_T("%s"), m_list.GetItemText(k,2));
                    s.Format(_T("%.03f"), _data[i][k]);
                }
                else
                {
                    s.Format(_T("	,%.03f"), _data[i][k]);
                }
                strText += s;
            }
            clsFile.WriteString(strText);
            strText = _T("\n");
            clsFile.WriteString(strText);
            strText = _T("");
        }

        clsFile.Close();
        //MessageBox(_T("导出到csv文件成功!"), _T("提示"));
    }


    return true;
}



bool CSportAlgorithm::SavePedCSV(vecd _data)
{
    time_t time_seconds = time(0);
    CString _newName;
    _newName.Format(_T("WITPed_%d.csv"), time_seconds);
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
        //setlocale(LC_CTYPE, ("chs"));
        for (size_t k = 0; k < _data.size(); k++)
        {
            CString strText;
            strText.Format(_T("%.03f"), _data[k]);
            clsFile.WriteString(strText);
            strText = _T("\n");
            clsFile.WriteString(strText);
        }
        clsFile.Close();
        //MessageBox(_T("导出到csv文件成功!"), _T("提示"));
    }


    return true;
}

bool CSportAlgorithm::SaveDatFCSV(vecd _data[])
{
    time_t time_seconds = time(0);
    CString _newName;
    _newName.Format(_T("WITDatF_%d.csv"), time_seconds);
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
        //setlocale(LC_CTYPE, ("chs"));
        CString strText;
        
        for (size_t k = 0; k < _data[0].size(); k++)
        {
            for (int i = 0; i < 5; i++)
            {
                CString s;
                if (i == 0)
                {
                    //s.Format(_T("%s"), m_list.GetItemText(k,2));
                    s.Format(_T("%.03f"), _data[i][k]);
                }
                else
                {
                    s.Format(_T("	,%.03f"), _data[i][k]);
                }
                strText += s;
            }
            clsFile.WriteString(strText);
            strText = _T("\n");
            clsFile.WriteString(strText);
            strText = _T("");
        }

        clsFile.Close();
        MessageBox(_T("导出到csv文件成功!"), _T("提示"));
    }


    return true;
}


void CSportAlgorithm::OnEnChangeSportEdit()
{

    //UINT ID = LOWORD(GetCurrentMessage()->wParam);
    //switch (ID)
    //{
    //case IDC_SPORT_EDIT1:
    //    break;
    //default:
    //    break;
    //}
    UpdateData();
}


//野值判断处理
bool CSportAlgorithm::isWildValue()
{
    
    for (int i = 0; i < m_column_count; i++)
    {
        data2[i].clear();

    }
    int _count=0;
    for (size_t k = 0; k < data1[0].size(); k++)
    {
        CString s;
        if (data1[0][k] < 0.0 || data1[0][k] > 84600.0)
        {
            continue;
        }
        if (data1[1][k] < sport_e3 || data1[1][k] > sport_e4 ||
            data1[2][k] < sport_e3 || data1[2][k] > sport_e4 ||
            data1[3][k] < sport_e3 || data1[3][k] > sport_e4)
        {
            continue;
        }

        if (data1[4][k] < sport_e5 || data1[4][k] > sport_e6 ||
            data1[5][k] < sport_e5 || data1[5][k] > sport_e6 ||
            data1[6][k] < sport_e5 || data1[6][k] > sport_e6)
        {
            continue;
        }

        for (int i = 0; i < m_column_count; i++)
        {
            data2[i].push_back(data1[i][k]);

        }
        _count++;
    }
    
    return false;
}

void CSportAlgorithm::OnBnClickedBtnCalculation2()
{
    SaveData2CSV(dataname, data2);
    SaveParCSV(m_Par);
    SavePedCSV(m_Ped);
    SaveDatFCSV(m_DatF);


}

void CSportAlgorithm::ResultToList()
{
    //m_list.DeleteAllItems();
    while (m_list.DeleteColumn(0));

    for (int j = 0; j < m_column_count; j++)
    {
        m_list.InsertColumn(j, dataname[j+1][0], LVCFMT_CENTER, 120);
    }
    
    for (size_t i = 0; i < data2[0].size(); i++)
    {       
        CString _str;
        _str.Format(_T("%.03f"), data2[0][i]); //秒时间  
        m_list.InsertItem(i, _str, LVCFMT_CENTER);        
        for (int j = 0; j < m_column_count; j++)
        {
            _str.Format(_T("%.03f"), data2[j][i]);
            m_list.SetItemText(i, j, _str);
        }
    }

    Take_V(sport_e7, data2, m_Par); //获得本地坐标系
    CalculationSteps(m_Par, m_Ped, sport_e2, sport_e1);//计算步数
    getDatFData(m_Par, m_Ped, m_DatF);//获取运动特性
    MessageBox(_T("验证计算完成!"), _T("提示"));
}

