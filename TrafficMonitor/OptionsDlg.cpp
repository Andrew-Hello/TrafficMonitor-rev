// OptionsDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "TrafficMonitor.h"
#include "OptionsDlg.h"
#include "afxdialogex.h"
#include "TaskbarVolumeControl.h"


// COptionsDlg 对话框

IMPLEMENT_DYNAMIC(COptionsDlg, CBaseDialog)

COptionsDlg::COptionsDlg(int tab, CWnd* pParent /*=NULL*/)
    : CBaseDialog(IDD_OPTIONS_DIALOG, pParent), m_tab_selected{ tab }
{

}

COptionsDlg::~COptionsDlg()
{
}

CString COptionsDlg::GetDialogName() const
{
    return OPTION_DLG_NAME;
}

void COptionsDlg::DoDataExchange(CDataExchange* pDX)
{
    CBaseDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TAB1, m_tab);
}

void COptionsDlg::CreateTaskbarVolumeWheelCheck()
{
    CWnd* anchor = m_tab2_dlg.GetDlgItem(IDC_WIN11_SETTINGS_BUTTON);
    if (anchor == nullptr)
        return;

    CRect anchor_rect;
    anchor->GetWindowRect(&anchor_rect);
    m_tab2_dlg.ScreenToClient(&anchor_rect);

    CRect client_rect;
    m_tab2_dlg.GetClientRect(&client_rect);

    const int margin = theApp.DPI(6);
    CRect check_rect(anchor_rect.right + margin,
                     anchor_rect.top,
                     client_rect.right - theApp.DPI(12),
                     anchor_rect.bottom);

    CString text;
    const std::wstring& language = theApp.m_str_table.GetLanguageInfo().bcp_47;
    if (language == L"zh-CN")
        text = L"鼠标滚轮调节系统音量";
    else if (language == L"zh-TW")
        text = L"滑鼠滾輪調整系統音量";
    else
        text = L"Mouse wheel controls system volume";

    if (m_taskbar_volume_wheel_check.Create(text,
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
        check_rect,
        &m_tab2_dlg,
        IDC_TASKBAR_VOLUME_WHEEL_DYNAMIC))
    {
        m_taskbar_volume_wheel_check.SetFont(m_tab2_dlg.GetFont());
        m_taskbar_volume_wheel_check.SetCheck(CTaskbarVolumeControl::IsEnabled() ? BST_CHECKED : BST_UNCHECKED);
    }
}

void COptionsDlg::SaveTaskbarVolumeWheelSetting()
{
    if (::IsWindow(m_taskbar_volume_wheel_check.GetSafeHwnd()))
        CTaskbarVolumeControl::SetEnabled(m_taskbar_volume_wheel_check.GetCheck() == BST_CHECKED);
}


BEGIN_MESSAGE_MAP(COptionsDlg, CBaseDialog)
    ON_WM_SIZE()
    ON_BN_CLICKED(IDC_APPLY_BUTTON, &COptionsDlg::OnBnClickedApplyButton)
END_MESSAGE_MAP()


// COptionsDlg 消息处理程序


BOOL COptionsDlg::OnInitDialog()
{
    CBaseDialog::OnInitDialog();

    // TODO:  在此添加额外的初始化
    SetIcon(theApp.GetMenuIcon(IDI_SETTINGS), FALSE);       // 设置小图标

    //创建子对话框
    m_tab1_dlg.Create(IDD_MAIN_WND_SETTINGS_DIALOG, &m_tab);
    m_tab2_dlg.Create(IDD_TASKBAR_SETTINGS_DIALOG, &m_tab);
    m_tab3_dlg.Create(IDD_GENERAL_SETTINGS_DIALOG, &m_tab);
    CreateTaskbarVolumeWheelCheck();

    //保存子对话框
    m_tab_vect.push_back(&m_tab1_dlg);
    m_tab_vect.push_back(&m_tab2_dlg);
    m_tab_vect.push_back(&m_tab3_dlg);

    //获取子对话框的初始高度
    for (const auto* pDlg : m_tab_vect)
    {
        CRect rect;
        pDlg->GetWindowRect(rect);
        m_tab_height.push_back(rect.Height());
    }

    //添加对话框
    m_tab.AddWindow(&m_tab1_dlg, CCommon::LoadText(IDS_MAIN_WINDOW_SETTINGS));
    m_tab.AddWindow(&m_tab2_dlg, CCommon::LoadText(IDS_TASKBAR_WINDOW_SETTINGS));
    m_tab.AddWindow(&m_tab3_dlg, CCommon::LoadText(IDS_GENERAL_SETTINGS));

    //为每个标签添加图标
    CImageList ImageList;
    ImageList.Create(theApp.DPI(16), theApp.DPI(16), ILC_COLOR32 | ILC_MASK, 2, 2);
    ImageList.Add(theApp.GetMenuIcon(IDI_MAIN_WINDOW));
    ImageList.Add(theApp.GetMenuIcon(IDI_TASKBAR_WINDOW));
    ImageList.Add(theApp.GetMenuIcon(IDI_SETTINGS));
    m_tab.SetImageList(&ImageList);
    ImageList.Detach();

    m_tab.SetItemSize(CSize(theApp.DPI(60), theApp.DPI(24)));
    m_tab.AdjustTabWindowSize();

    //为每个子窗口设置滚动信息
    for (size_t i = 0; i < m_tab_vect.size(); i++)
    {
        m_tab_vect[i]->SetScrollbarInfo(m_tab.m_tab_rect.Height(), m_tab_height[i]);
    }

    //设置默认选中的标签
    if (m_tab_selected < 0 || m_tab_selected >= m_tab.GetItemCount())
        m_tab_selected = 0;
    m_tab.SetCurTab(m_tab_selected);

    return TRUE;  // return TRUE unless you set the focus to a control
                  // 异常: OCX 属性页应返回 FALSE
}


void COptionsDlg::OnOK()
{
    // TODO: 在此添加专用代码和/或调用基类
    SaveTaskbarVolumeWheelSetting();
    m_tab1_dlg.OnOK();
    m_tab2_dlg.OnOK();
    m_tab3_dlg.OnOK();

    CBaseDialog::OnOK();
}


void COptionsDlg::OnSize(UINT nType, int cx, int cy)
{
    CBaseDialog::OnSize(nType, cx, cy);

    // TODO: 在此处添加消息处理程序代码
    if (nType != SIZE_MINIMIZED)
    {
        //为每个子窗口设置滚动信息
        for (size_t i = 0; i < m_tab_vect.size(); i++)
        {
            m_tab_vect[i]->SetScrollbarInfo(m_tab.m_tab_rect.Height(), m_tab_height[i]);
        }
    }
}


void COptionsDlg::OnCancel()
{
    // TODO: 在此添加专用代码和/或调用基类
    m_tab3_dlg.OnCancel();

    CBaseDialog::OnCancel();
}


void COptionsDlg::OnBnClickedApplyButton()
{
    SaveTaskbarVolumeWheelSetting();
    m_tab2_dlg.SaveColorSettingToDefaultStyle();
    ::SendMessage(theApp.m_pMainWnd->GetSafeHwnd(), WM_SETTINGS_APPLIED, (WPARAM)this, 0);
    for (size_t i = 0; i < m_tab_vect.size(); i++)
    {
        m_tab_vect[i]->OnSettingsApplied();
    }
}
