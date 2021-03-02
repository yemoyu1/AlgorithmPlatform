#pragma once

#include <vector>

using namespace std;

typedef vector<CString> vecN;
typedef vector<float> vecf;
// CSensorAlgorithmDialog 对话框

class CSensorAlgorithmDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CSensorAlgorithmDialog)

public:
	CSensorAlgorithmDialog(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CSensorAlgorithmDialog();
	bool SaveCSV(vecN _data[], vecf _data1[]);
// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SENSOR_ALGORITHM_DIALOG };
#endif
public:
	vecf data1[12];
	vecf data2[12];
	vecN dataname[12];
	double m_timecost;
protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnLoad();
	afx_msg void OnBnClickedBtnCalculation();
	CListCtrl m_list;
};
