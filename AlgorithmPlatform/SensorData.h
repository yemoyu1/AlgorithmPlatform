#pragma once
#include <vector>

using namespace std;
typedef vector<CString> vecN;
typedef vector<float> vecf;
// CSensorData 对话框

class CSensorData : public CDialogEx
{
	DECLARE_DYNAMIC(CSensorData)

public:
	CSensorData(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CSensorData();
	bool SaveCSV(vecN _data[], vecf _data1[]);
// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SENSOR_DATA_DIALOG };
#endif
private:
	vecf data1[12];
	vecf data2[12];
	vecN dataname[12];
	double m_timecost;
protected:
	BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnLoad();
	afx_msg void OnBnClickedBtnCalculation();
	CListCtrl m_list;
};
