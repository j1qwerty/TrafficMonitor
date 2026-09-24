#include "stdafx.h"
#include "TrafficMonitorController.h"
#include "TrafficMonitor.h"
#include "afxdialogex.h"
#include "BaseDialog.h"
#include "TaskBarDlg.h"
#include "ClassicalTaskbarDlg.h"
#include "Win11TaskbarDlg.h"
#include "WineTaskbarDlg.h"
#include "TaskbarHelper.h"
#include "SupportedRenderEnums.h"
#include "OptionsDlg.h"
#include "NetworkInfoDlg.h"
#include "AboutDlg.h"
#include "IconSelectDlg.h"
#include "HistoryTrafficDlg.h"
#include "PluginManagerDlg.h"
#include "PluginInfoDlg.h"
#include "SetItemOrderDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

unsigned int CTrafficMonitorController::m_WM_TASKBARCREATED{ ::RegisterWindowMessage(_T("TaskbarCreated")) };

BEGIN_MESSAGE_MAP(CTrafficMonitorController, CDialog)
    ON_WM_TIMER()
    ON_COMMAND(ID_NETWORK_INFO, &CTrafficMonitorController::OnNetworkInfo)
    ON_COMMAND(ID_SHOW_NOTIFY_ICON, &CTrafficMonitorController::OnShowNotifyIcon)
    ON_COMMAND(ID_SHOW_CPU_MEMORY2, &CTrafficMonitorController::OnShowCpuMemory2)
    ON_COMMAND(ID_APP_ABOUT, &CTrafficMonitorController::OnAppAbout)
    ON_COMMAND(ID_TRAFFIC_HISTORY, &CTrafficMonitorController::OnTrafficHistory)
    ON_COMMAND(ID_OPTIONS2, &CTrafficMonitorController::OnOptions2)
    ON_COMMAND(ID_CHANGE_NOTIFY_ICON, &CTrafficMonitorController::OnChangeNotifyIcon)
    ON_COMMAND(ID_CHECK_UPDATE, &CTrafficMonitorController::OnCheckUpdate)
    ON_COMMAND(ID_AUTO_RUN_WHEN_START, &CTrafficMonitorController::OnAutoRunWhenStart)
    ON_REGISTERED_MESSAGE(m_WM_TASKBARCREATED, &CTrafficMonitorController::OnTaskBarCreated)
    ON_MESSAGE(WM_TASKBAR_MENU_POPED_UP, &CTrafficMonitorController::OnTaskbarMenuPopedUp)
    ON_COMMAND(ID_SHOW_NET_SPEED, &CTrafficMonitorController::OnShowNetSpeed)
    ON_WM_QUERYENDSESSION()
    ON_WM_CLOSE()
    ON_WM_DESTROY()
    ON_MESSAGE(WM_TASKBAR_WND_CLOSED, &CTrafficMonitorController::OnTaskbarWndClosed)
    ON_MESSAGE(WM_MONITOR_INFO_UPDATED, &CTrafficMonitorController::OnMonitorInfoUpdated)
    ON_MESSAGE(WM_DISPLAYCHANGE, &CTrafficMonitorController::OnDisplaychange)
    ON_COMMAND(ID_PLUGIN_MANAGE, &CTrafficMonitorController::OnPluginManage)
    ON_MESSAGE(WM_REOPEN_TASKBAR_WND, &CTrafficMonitorController::OnReopenTaksbarWnd)
    ON_COMMAND(ID_OPEN_TASK_MANAGER, &CTrafficMonitorController::OnOpenTaskManager)
    ON_MESSAGE(WM_SETTINGS_APPLIED, &CTrafficMonitorController::OnSettingsApplied)
    ON_COMMAND(ID_DISPLAY_SETTINGS, &CTrafficMonitorController::OnDisplaySettings)
    ON_COMMAND(ID_REFRESH_CONNECTION_LIST, &CTrafficMonitorController::OnRefreshConnectionList)
    ON_COMMAND(ID_PLUGIN_OPTIONS_TASKBAR, &CTrafficMonitorController::OnPluginOptionsTaksbar)
    ON_COMMAND(ID_PLUGIN_DETAIL_TASKBAR, &CTrafficMonitorController::OnPluginDetailTaksbar)
    ON_WM_POWERBROADCAST()
END_MESSAGE_MAP()

CTrafficMonitorController::CTrafficMonitorController(UINT nIDTemplate, CWnd* pParent)
    : CDialog(nIDTemplate, pParent),
      m_history_traffic(theApp.m_history_traffic_path),
      m_connection_name_preferd(theApp.m_cfg_data.m_connection_name)
{}

CTrafficMonitorController::~CTrafficMonitorController()
{
    free(m_pIfTable);
    if (m_tBarDlg != nullptr)
    {
        delete m_tBarDlg;
        m_tBarDlg = nullptr;
    }
    for (int i = 0; i < MAX_NOTIFY_ICON; i++)
    {
        if (theApp.m_notify_icons[i] != NULL)
        {
            ::DestroyIcon(theApp.m_notify_icons[i]);
            theApp.m_notify_icons[i] = NULL;
        }
    }
}

CTaskBarDlg* CTrafficMonitorController::GetTaskbarWindow() const
{
    return IsTaskbarWndValid() ? m_tBarDlg : nullptr;
}

CTrafficMonitorController* CTrafficMonitorController::Instance()
{
    return dynamic_cast<CTrafficMonitorController*>(theApp.m_pMainWnd);
}

LRESULT CTrafficMonitorController::OnTaskBarCreated(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);

    if (m_tBarDlg != nullptr)
    {
        CloseTaskBarWnd();
        if (theApp.m_general_data.show_notify_icon)
            ::Shell_NotifyIcon(NIM_ADD, &m_ntIcon);
        OpenTaskBarWnd();
    }
    else
    {
        if (theApp.m_general_data.show_notify_icon)
            ::Shell_NotifyIcon(NIM_ADD, &m_ntIcon);
    }
    return LRESULT();
}


























































































void CTrafficMonitorController::_OnOptions(int tab, CWnd* pParent)
{
    COptionsDlg optionsDlg(tab, pParent);
    if (COptionsDlg::GetUniqueHandel(OPTION_DLG_NAME) == NULL)
    {
        optionsDlg.m_tab2_dlg.m_data = theApp.m_taskbar_data;
        optionsDlg.m_tab3_dlg.m_data = theApp.m_general_data;
    }
    if (optionsDlg.DoModal() == IDOK)
        ApplySettings(optionsDlg);
}

void CTrafficMonitorController::ApplySettings(COptionsDlg& optionsDlg)
{
    const bool is_show_notify_icon_changed =
        (optionsDlg.m_tab3_dlg.m_data.show_notify_icon != theApp.m_general_data.show_notify_icon);
    const bool is_connections_hide_changed =
        (optionsDlg.m_tab3_dlg.m_data.connections_hide.data() != theApp.m_general_data.connections_hide.data());
    const bool taskbar_changed =
        (theApp.m_taskbar_data.show_taskbar_wnd_in_secondary_display != optionsDlg.m_tab2_dlg.m_data.show_taskbar_wnd_in_secondary_display
        || theApp.m_taskbar_data.secondary_display_index != optionsDlg.m_tab2_dlg.m_data.secondary_display_index
        || theApp.m_taskbar_data.disable_d2d != optionsDlg.m_tab2_dlg.m_data.disable_d2d
        || theApp.m_taskbar_data.IsTaskbarTransparent() != optionsDlg.m_tab2_dlg.m_data.IsTaskbarTransparent()
        || theApp.m_taskbar_data.auto_set_background_color != optionsDlg.m_tab2_dlg.m_data.auto_set_background_color);

    theApp.m_taskbar_data = optionsDlg.m_tab2_dlg.m_data;
    theApp.m_general_data = optionsDlg.m_tab3_dlg.m_data;
    theApp.SendSettingsToPlugin();
    CGeneralSettingsDlg::CheckTaskbarDisplayItem();

    if (IsTaskbarWndValid())
    {
        m_tBarDlg->ApplySettings();
        if (taskbar_changed)
        {
            CloseTaskBarWnd();
            OpenTaskBarWnd();
        }
        else
        {
            m_tBarDlg->WidthChanged();
        }
        m_tBarDlg->ApplyWindowTransparentColor();
    }

    if (optionsDlg.m_tab3_dlg.IsAutoRunModified())
    {
        if (!theApp.SetAutoRun(theApp.m_general_data.auto_run, theApp.m_general_data.auto_run_by_task_scheduler))
            MessageBox(CCommon::LoadText(IDS_SET_AUTO_RUN_FAILED_WARNING), nullptr, MB_ICONWARNING | MB_OK);
    }

    if (optionsDlg.m_tab3_dlg.IsShowAllInterfaceModified() || is_connections_hide_changed)
        IniConnection();

    if (optionsDlg.m_tab3_dlg.IsMonitorTimeSpanModified())
    {
        KillTimer(MONITOR_TIMER);
        SetTimer(MONITOR_TIMER, theApp.m_general_data.monitor_time_span, NULL);
    }

    if (is_show_notify_icon_changed)
    {
        if (theApp.IsForceShowNotifyIcon())
            theApp.m_general_data.show_notify_icon = true;
        if (theApp.m_general_data.show_notify_icon)
            AddNotifyIcon();
        else
            DeleteNotifyIcon();
    }

    theApp.SaveConfig();
    theApp.SaveGlobalConfig();
}

BOOL CTrafficMonitorController::OnInitDialog()
{
    CDialog::OnInitDialog();
    SetWindowText(APP_NAME);
    ModifyStyleEx(WS_EX_APPWINDOW, WS_EX_TOOLWINDOW);
    theApp.DPIFromWindow(this);
    theApp.InitMenuResourse();
    IniConnection();
    LoadHistoryTraffic();
    GetLocalTime(&m_start_time);
    SetTimer(MONITOR_TIMER, theApp.m_general_data.monitor_time_span, NULL);
    AfxBeginThread(MonitorThreadCallback, (LPVOID)this);
    OpenTaskBarWnd();
    SetTimer(TASKBAR_TIMER, 1000, NULL);
    return TRUE;
}

void CTrafficMonitorController::OnTimer(UINT_PTR nIDEvent)
{
    if (nIDEvent == MONITOR_TIMER)
    {
        m_monitor_data_required = true;
        m_monitor_request_event.SetEvent();
    }
    if (nIDEvent == TASKBAR_TIMER && IsTaskbarWndValid())
        m_tBarDlg->AdjustWindowPos();
    if (nIDEvent == DELETE_NOTIFY_ICON_TIMER)
    {
        DeleteNotifyIcon();
        KillTimer(DELETE_NOTIFY_ICON_TIMER);
    }
    CDialog::OnTimer(nIDEvent);
}

bool CTrafficMonitorController::IsTemperatureNeeded() const
{
    return false;
}

void CTrafficMonitorController::OnClose()
{
    theApp.m_cannot_save_config_warning = true;
    theApp.m_cannot_save_global_config_warning = true;
    theApp.SaveConfig();
    theApp.SaveGlobalConfig();
    SaveHistoryTrafficFull();
    BackupHistoryTrafficFile();

    if (IsTaskbarWndValid())
        m_tBarDlg->OnCancel();

    for (const auto& item : CBaseDialog::AllUniqueHandels())
        ::SendMessage(item.second, WM_COMMAND, IDCANCEL, 0);

    CDialog::OnClose();
}

void CTrafficMonitorController::OnDestroy()
{
    CDialog::OnDestroy();
    ::Shell_NotifyIcon(NIM_DELETE, &m_ntIcon);
    ExitMonitorThread();
}

LRESULT CTrafficMonitorController::OnMonitorInfoUpdated(WPARAM, LPARAM)
{
    if (IsTaskbarWndValid())
        m_tBarDlg->UpdateToolTips();
    return 0;
}

LRESULT CTrafficMonitorController::OnDisplaychange(WPARAM, LPARAM)
{
    if (IsTaskbarWndValid())
        m_tBarDlg->AdjustWindowPos(true);
    return 0;
}

UINT CTrafficMonitorController::OnPowerBroadcast(UINT nPowerEvent, LPARAM nEventData)
{
    if (nPowerEvent == PBT_APMRESUMESUSPEND)
    {
        KillTimer(INIT_CONNECT_TIMER);
        static CTrafficMonitorController* pThis = this;
        static int check_times = 0;
        SetTimer(INIT_CONNECT_TIMER, 10000, [](HWND, UINT, UINT_PTR, DWORD) {
            pThis->IniConnection();
            check_times++;

            CString info = CCommon::LoadTextFormat(IDS_RESTORE_FROM_SLEEP_LOG, { pThis->m_restart_cnt });
            CCommon::WriteLog(info, theApp.m_log_path.c_str());

            if (pThis->m_connections.empty())
            {
                if (check_times >= 20)
                    pThis->KillTimer(INIT_CONNECT_TIMER);
            }
            else
            {
                pThis->KillTimer(INIT_CONNECT_TIMER);
                check_times = 0;
            }
        });
    }

    return CDialog::OnPowerBroadcast(nPowerEvent, nEventData);
}

BOOL CTrafficMonitorController::OnCommand(WPARAM wParam, LPARAM lParam)
{
    const UINT uMsg = LOWORD(wParam);
    if (uMsg == ID_SELECT_ALL_CONNECTION)
    {
        theApp.m_cfg_data.m_select_all = true;
        theApp.m_cfg_data.m_auto_select = false;
        m_connection_change_flag = true;
        return TRUE;
    }
    if (uMsg == ID_SELETE_CONNECTION)
    {
        AutoSelect();
        theApp.m_cfg_data.m_auto_select = true;
        theApp.m_cfg_data.m_select_all = false;
        theApp.SaveConfig();
        m_connection_change_flag = true;
        return TRUE;
    }
    if (uMsg > ID_SELECT_ALL_CONNECTION && uMsg <= ID_SELECT_ALL_CONNECTION + m_connections.size())
    {
        m_connection_selected = uMsg - ID_SELECT_ALL_CONNECTION - 1;
        theApp.m_cfg_data.m_connection_name = GetConnection(m_connection_selected).description_2;
        m_connection_name_preferd = theApp.m_cfg_data.m_connection_name;
        theApp.m_cfg_data.m_auto_select = false;
        theApp.m_cfg_data.m_select_all = false;
        theApp.SaveConfig();
        m_connection_change_flag = true;
        return TRUE;
    }
    if (uMsg == ID_AUTO_RUN_WHEN_START)
    {
        OnAutoRunWhenStart();
        return TRUE;
    }
    return CDialog::OnCommand(wParam, lParam);
}
