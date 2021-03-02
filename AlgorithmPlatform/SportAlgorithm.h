#pragma once

#include <vector>

using namespace std;

typedef vector<CString> vecN;
typedef vector<double> vecd;

// CSportAlgorithm 对话框

class CSportAlgorithm : public CDialogEx
{
	DECLARE_DYNAMIC(CSportAlgorithm)

public:
	CSportAlgorithm(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CSportAlgorithm();
	bool SaveData2CSV(vecN _data[], vecd _data1[]);
	bool SaveParCSV(vecd _data[]);
	bool SavePedCSV(vecd _data);
	bool SaveDatFCSV(vecd _data[]);
// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SPORT_ALGORITHM_DIALOG };
#endif
public:
	vecd data1[13];//初始导入数据
	vecd data2[13];//帧赛选侯的数据
	vecN dataname[13];//字段头
	vecd m_Par[17]; //总的特性计算
	vecd m_Ped;  //判断是否是同一步 
	vecd m_DatF[5];  //每一步运动参数
	double m_timecost;
public:
	int m_column_count;
	bool isWildValue();//野值判断处理
	void ResultToList();
protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnLoad();
	afx_msg void OnBnClickedBtnCalculation();
	CListCtrl m_list;
	double sport_e1;//Timespan 
	double sport_e2;//thres
	int sport_e3;//a_min
	int sport_e4;//a_max
	int sport_e5;//ω_min
	int sport_e6;//ω_max
	double sport_e7;//delta
	afx_msg void OnEnChangeSportEdit();
	afx_msg void OnBnClickedBtnCalculation2();
	afx_msg void OnEnChangeSportEdit7();
};
