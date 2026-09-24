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
    : CDialog(nIDTemplate, pParent)
{
    m_history_traffic.SetFilePath(theApp.m_history_traffic_path);
    m_connection_name_preferd = theApp.m_cfg_data.m_connection_name;
}

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

// Lite controller implementations restored from the shared monitor/taskbar implementation.
static int GetMonitorTimerCount(int second)
{
    int count = second * 1000 / theApp.m_general_data.monitor_time_span;
    if (count <= 0) count = 1;
    return count;
}

bool CTrafficMonitorController::IsTaskbarWndValid() const
{
    return m_tBarDlg != nullptr && ::IsWindow(m_tBarDlg->GetSafeHwnd());
}

void CTrafficMonitorController::ExitMonitorThread()
{
    m_is_thread_exit = true;
    m_monitor_request_event.SetEvent();
    ::WaitForSingleObject(m_threadExitEvent.m_hObject, 1000);
}

void CTrafficMonitorController::ShowNotifyTip(const wchar_t* title, const wchar_t* message)
{
    //要显示通知区提示，必须先将通知区图标显示出来
    if (!theApp.m_general_data.show_notify_icon)
    {
        //添加通知栏图标
        AddNotifyIcon();
    }
    //显示通知提示
    m_ntIcon.uFlags |= NIF_INFO;
    //wcscpy_s(m_ntIcon.szInfo, message ? message : _T(""));
    //wcscpy_s(m_ntIcon.szInfoTitle, title ? title : _T(""));
    CCommon::WStringCopy(m_ntIcon.szInfo, 256, message);
    CCommon::WStringCopy(m_ntIcon.szInfoTitle, 64, title);
    ::Shell_NotifyIcon(NIM_MODIFY, &m_ntIcon);
    m_ntIcon.uFlags &= ~NIF_INFO;

    //如果不显示通知区域图标，则在弹出通知的一段时间后删除通知区图标
    if (!theApp.m_general_data.show_notify_icon)
    {
        //延迟一定时间后删除通知区图标
        KillTimer(DELETE_NOTIFY_ICON_TIMER);
        SetTimer(DELETE_NOTIFY_ICON_TIMER, 8000, NULL);
    }
}

void CTrafficMonitorController::AutoSelect()
{
    unsigned __int64 max_in_out_bytes{};
    unsigned __int64 in_out_bytes;
    //m_connection_selected = m_connections[0].index;
    m_connection_selected = 0;
    //自动选择连接时，查找已发送和已接收字节数之和最多的那个连接，并将其设置为当前查看的连接
    for (size_t i{}; i < m_connections.size(); i++)
    {
        auto table = GetConnectIfTable(i);
        if (table.dwOperStatus == IF_OPER_STATUS_OPERATIONAL)     //只选择网络状态为正常的连接
        {
            in_out_bytes = table.dwInOctets + table.dwOutOctets;
            if (in_out_bytes > max_in_out_bytes)
            {
                max_in_out_bytes = in_out_bytes;
                m_connection_selected = i;
            }
        }
    }
    theApp.m_cfg_data.m_connection_name = GetConnection(m_connection_selected).description_2;
    m_connection_change_flag = true;
}

void CTrafficMonitorController::IniConnection()
{
    //为m_pIfTable开辟所需大小的内存
    free(m_pIfTable);
    m_dwSize = sizeof(MIB_IFTABLE);
    m_pIfTable = (MIB_IFTABLE*)malloc(m_dwSize);
    int rtn;
    rtn = GetIfTable(m_pIfTable, &m_dwSize, FALSE);
    if (rtn == ERROR_INSUFFICIENT_BUFFER)	//如果函数返回值为ERROR_INSUFFICIENT_BUFFER，说明m_pIfTable的大小不够
    {
        free(m_pIfTable);
        m_pIfTable = (MIB_IFTABLE*)malloc(m_dwSize);	//用新的大小重新开辟一块内存
    }
    GetIfTable(m_pIfTable, &m_dwSize, FALSE);

    //获取当前所有的连接，并保存到m_connections容器中
    if (!theApp.m_general_data.show_all_interface)
    {
        m_connections.clear();
        vector<NetWorkConection> connections;
        CAdapterCommon::GetAdapterInfo(connections);
        for (const auto& item : connections)
        {
            if (!theApp.m_general_data.connections_hide.Contains(CCommon::StrToUnicode(item.description.c_str())))
                m_connections.push_back(item);
        }
        CAdapterCommon::GetIfTableInfo(m_connections, m_pIfTable);
    }
    else
    {
        CAdapterCommon::GetAllIfTableInfo(m_connections, m_pIfTable);
    }

    //如果在设置了“显示所有网络连接”时设置了“选择全部”，则改为“自动选择”
    if (theApp.m_general_data.show_all_interface && theApp.m_cfg_data.m_select_all)
    {
        theApp.m_cfg_data.m_select_all = false;
        theApp.m_cfg_data.m_auto_select = true;
    }

    //写入调试日志
    if (theApp.m_debug_log)
    {
        CString log_str;
        log_str += _T("正在初始化网络连接...\n");
        log_str += _T("连接列表：\n");
        for (size_t i{}; i < m_connections.size(); i++)
        {
            log_str += m_connections[i].description.c_str();
            log_str += _T(", ");
            log_str += CCommon::IntToString(m_connections[i].index);
            log_str += _T("\n");
        }
        log_str += _T("IfTable:\n");
        for (size_t i{}; i < m_pIfTable->dwNumEntries; i++)
        {
            log_str += CCommon::IntToString(i);
            log_str += _T(" ");
            log_str += (const char*)m_pIfTable->table[i].bDescr;
            log_str += _T("\n");
        }
        CCommon::WriteLog(log_str, (theApp.m_config_dir + L".\\connections.log").c_str());
    }

    //if (m_connection_selected < 0 || m_connection_selected >= m_connections.size() || theApp.m_cfg_data.m_auto_select)
    //  AutoSelect();
    //选择网络连接
    if (theApp.m_cfg_data.m_auto_select)    //自动选择
    {
        if (m_restart_cnt != -1)    //当m_restart_cnt不等于-1时，即不是第一次初始化时，需要延时5秒再重新初始化连接
        {
            KillTimer(DELAY_TIMER);
            SetTimer(DELAY_TIMER, 5000, NULL);
        }
        else
        {
            AutoSelect();
        }
    }
    else        //查找网络名为上次选择的连接
    {
        m_connection_selected = 0;
        for (size_t i{}; i < m_connections.size(); i++)
        {
            if (m_connections[i].description_2 == m_connection_name_preferd)
                m_connection_selected = i;
        }
    }
    if (m_connection_selected < 0 || m_connection_selected >= m_connections.size())
        m_connection_selected = 0;
    theApp.m_cfg_data.m_connection_name = GetConnection(m_connection_selected).description_2;

    //根据已获取到的连接在菜单中添加相应项目
    IniConnectionMenu(theApp.m_main_menu.GetSubMenu(0)->GetSubMenu(0));      //向“选择网络连接”子菜单项添加项目
    IniConnectionMenu(theApp.m_main_menu_plugin.GetSubMenu(0)->GetSubMenu(0));      //向“选择网络连接”子菜单项添加项目

    IniTaskBarConnectionMenu();     //初始化任务栏窗口中的“选择网络连接”子菜单项

    m_restart_cnt++;    //记录初始化次数
    m_connection_change_flag = true;
}

MIB_IFROW CTrafficMonitorController::GetConnectIfTable(int connection_index)
{
    if (connection_index >= 0 && connection_index < static_cast<int>(m_connections.size()))
    {
        int index = m_connections[connection_index].index;
        if (m_pIfTable != nullptr && index >= 0 && index < m_pIfTable->dwNumEntries)
            return m_pIfTable->table[index];
    }
    return MIB_IFROW();
}

NetWorkConection CTrafficMonitorController::GetConnection(int connection_index)
{
    if (connection_index >= 0 && connection_index < static_cast<int>(m_connections.size()))
        return m_connections[connection_index];
    else
        return NetWorkConection();
}

void CTrafficMonitorController::IniConnectionMenu(CMenu* pMenu)
{
    ASSERT(pMenu != nullptr);
    if (pMenu != nullptr)
    {
        //先将ID_SELECT_ALL_CONNECTION后面的所有菜单项删除
        int start_pos = CCommon::GetMenuItemPosition(pMenu, ID_SELECT_ALL_CONNECTION) + 1;
        while (pMenu->GetMenuItemCount() > start_pos)
        {
            pMenu->DeleteMenu(start_pos, MF_BYPOSITION);
        }

        CString connection_descr;
        for (size_t i{}; i < m_connections.size(); i++)
        {
            connection_descr = CCommon::StrToUnicode(m_connections[i].description.c_str()).c_str();
            pMenu->AppendMenu(MF_STRING | MF_ENABLED, ID_SELECT_ALL_CONNECTION + i + 1, connection_descr);
        }

        //添加“刷新网络列表”命令
        pMenu->AppendMenu(MF_SEPARATOR);
        pMenu->AppendMenu(MF_STRING | MF_ENABLED, ID_REFRESH_CONNECTION_LIST, CCommon::LoadText(IDS_REFRESH_CONNECTION_LIST));
    }
}

void CTrafficMonitorController::IniTaskBarConnectionMenu()
{
    //向“选择网络连接”子菜单项添加项目
    IniConnectionMenu(theApp.m_taskbar_menu.GetSubMenu(0)->GetSubMenu(0));
    IniConnectionMenu(theApp.m_taskbar_menu_plugin.GetSubMenu(0)->GetSubMenu(0));
}

void CTrafficMonitorController::SetConnectionMenuState(CMenu* pMenu)
{
    if (theApp.m_cfg_data.m_select_all)
        pMenu->CheckMenuRadioItem(0, m_connections.size() + 1, 1, MF_BYPOSITION | MF_CHECKED);
    else if (theApp.m_cfg_data.m_auto_select)       //theApp.m_cfg_data.m_auto_select为true时为自动选择，选中菜单的第1项
        pMenu->CheckMenuRadioItem(0, m_connections.size() + 1, 0, MF_BYPOSITION | MF_CHECKED);
    else        //theApp.m_cfg_data.m_auto_select为false时非自动选择，根据m_connection_selected的值选择对应的项
        pMenu->CheckMenuRadioItem(0, m_connections.size() + 1, m_connection_selected + 2, MF_BYPOSITION | MF_CHECKED);

    //没有设置为“选择全部”时，将当前选择项设置为默认菜单项（加粗显示）
    if (!theApp.m_cfg_data.m_select_all)
        pMenu->SetDefaultItem(m_connection_selected + 2, TRUE);
    else
        pMenu->SetDefaultItem(-1, TRUE);
}

void CTrafficMonitorController::CloseTaskBarWnd()
{
    if (m_tBarDlg != nullptr)
    {
        if (IsTaskbarWndValid())
            m_tBarDlg->OnCancel();
        delete m_tBarDlg;
        m_tBarDlg = nullptr;
        theApp.m_taskbar_data.update_layered_window_error_code = 0;
    }
}

void CTrafficMonitorController::OpenTaskBarWnd()
{
    // 强制初始化theApp.m_is_windows11_taskbar的值
    theApp.CheckWindows11Taskbar();
    if (theApp.m_win_version.IsWine())
        m_tBarDlg = new CWineTaskbarDlg();
    else if (theApp.IsWindows11Taskbar())
        m_tBarDlg = new CWin11TaskbarDlg();
    else
        m_tBarDlg = new CClassicalTaskbarDlg();

    CSupportedRenderEnums supported_render_enums{};
    CTaskBarDlg::DisableRenderFeatureIfNecessary(supported_render_enums);
    auto render_type = supported_render_enums.GetAutoFitEnum();
    // WS_EX_LAYERED 和 WS_EX_NOREDIRECTIONBITMAP 可以共存，见微软示例代码
    // https://github.com/microsoft/Windows-classic-samples/blob/7cbd99ac1d2b4a0beffbaba29ea63d024ceff700/Samples/DynamicDPI/cpp/SampleDesktopWindow.cpp#L179
    // 但是WS_EX_NOREDIRECTIONBITMAP似乎会导致UpdateLayeredWindowIndirect失败
    switch (render_type)
    {
        using namespace DrawCommonHelper;
    case RenderType::D2D1_WITH_DCOMPOSITION:
        m_tBarDlg->Create(IDD_TASK_BAR_DIALOG_NOREDIRECTIONBITMAP, this);
        break;
    // 包括RenderType::D2D1在内的其他值
    default:
        m_tBarDlg->Create(IDD_TASK_BAR_DIALOG, this);
        break;
    }
    m_tBarDlg->ShowWindow(SW_SHOW);
    //m_tBarDlg->ShowInfo();
    //IniTaskBarConnectionMenu();
}

void CTrafficMonitorController::AddNotifyIcon()
{
    if (theApp.m_cfg_data.m_show_task_bar_wnd)
        CloseTaskBarWnd();
    //添加通知栏图标
    ::Shell_NotifyIcon(NIM_ADD, &m_ntIcon);
    if (theApp.m_cfg_data.m_show_task_bar_wnd)
        OpenTaskBarWnd();
}

void CTrafficMonitorController::DeleteNotifyIcon()
{
    if (theApp.m_cfg_data.m_show_task_bar_wnd)
        CloseTaskBarWnd();
    //删除通知栏图标
    ::Shell_NotifyIcon(NIM_DELETE, &m_ntIcon);
    if (theApp.m_cfg_data.m_show_task_bar_wnd)
        OpenTaskBarWnd();
}

void CTrafficMonitorController::UpdateNotifyIconTip()
{
    CString strTip;         //鼠标指向图标时显示的提示
#ifdef _DEBUG
    strTip = CCommon::LoadText(IDS_TRAFFICMONITOR, _T(" (Debug)"));
#else
    strTip = CCommon::LoadText(IDS_TRAFFICMONITOR);
#endif

    CString in_speed = CCommon::DataSizeToString(theApp.m_in_speed);
    CString out_speed = CCommon::DataSizeToString(theApp.m_out_speed);

    strTip += CCommon::StringFormat(_T("\r\n<%1%>: <%2%>/s"), { CCommon::LoadText(IDS_UPLOAD), out_speed });
    strTip += CCommon::StringFormat(_T("\r\n<%1%>: <%2%>/s"), { CCommon::LoadText(IDS_DOWNLOAD), in_speed });
    strTip += CCommon::StringFormat(_T("\r\nCPU: <%1%> %"), { theApp.m_cpu_usage });
    strTip += CCommon::StringFormat(_T("\r\n<%1%>: <%2%> %"), { CCommon::LoadText(IDS_MEMORY), theApp.m_memory_usage });
    if (IsTemperatureNeeded())
    {
        if (theApp.m_general_data.IsHardwareEnable(HI_GPU) && theApp.m_gpu_usage >= 0)
            strTip += CCommon::StringFormat(_T("\r\n<%1%>: <%2%> %"), { CCommon::LoadText(IDS_GPU_USAGE), theApp.m_gpu_usage });
        if (theApp.m_general_data.IsHardwareEnable(HI_CPU) && theApp.m_cpu_temperature > 0)
            strTip += CCommon::StringFormat(_T("\r\n<%1%>: <%2%> °C"), { CCommon::LoadText(IDS_CPU_TEMPERATURE), static_cast<int>(theApp.m_cpu_temperature) });
        if (theApp.m_general_data.IsHardwareEnable(HI_GPU) && theApp.m_gpu_temperature > 0)
            strTip += CCommon::StringFormat(_T("\r\n<%1%>: <%2%> °C"), { CCommon::LoadText(IDS_GPU_TEMPERATURE), static_cast<int>(theApp.m_gpu_temperature) });
        if (theApp.m_general_data.IsHardwareEnable(HI_HDD) && theApp.m_hdd_temperature > 0)
            strTip += CCommon::StringFormat(_T("\r\n<%1%>: <%2%> °C"), { CCommon::LoadText(IDS_HDD_TEMPERATURE), static_cast<int>(theApp.m_hdd_temperature) });
        if (theApp.m_general_data.IsHardwareEnable(HI_MBD) && theApp.m_main_board_temperature > 0)
            strTip += CCommon::StringFormat(_T("\r\n<%1%>: <%2%> °C"), { CCommon::LoadText(IDS_MAINBOARD_TEMPERATURE), static_cast<int>(theApp.m_main_board_temperature) });
        if (theApp.m_general_data.IsHardwareEnable(HI_HDD) && theApp.m_hdd_usage >= 0)
            strTip += CCommon::StringFormat(_T("\r\n<%1%>: <%2%> %"), { CCommon::LoadText(IDS_HDD_USAGE), theApp.m_hdd_usage });
    }

    CCommon::WStringCopy(m_ntIcon.szTip, 128, strTip);
    ::Shell_NotifyIcon(NIM_MODIFY, &m_ntIcon);

}

void CTrafficMonitorController::SaveHistoryTraffic()
{
    // 使用增量保存，只更新第一行和今天的记录，减少I/O操作
    m_history_traffic.SaveTodayOnly();
}

void CTrafficMonitorController::SaveHistoryTrafficFull()
{
    // 完整保存，用于程序退出时确保所有数据都保存
    m_history_traffic.Save();
}

void CTrafficMonitorController::LoadHistoryTraffic()
{
    m_history_traffic.Load();
    CHistoryTrafficFile backup_file(theApp.m_history_traffic_path + L".bak");
    backup_file.LoadSize();     //读取备份文件中流量记录的数量
    
    // 如果备份文件中流量记录的数量大于当前的数量，尝试从备份文件中恢复
    if (backup_file.Size() > m_history_traffic.Size())
    {
        size_t size_before = m_history_traffic.Size();
        backup_file.Load();     //加载备份文件（会清理"未来"的记录）
        size_t backup_size_after_load = backup_file.Size();  //加载后实际的记录数（可能因为清理"未来"记录而减少）
        
        // 加载后，如果备份文件的记录数仍然大于当前文件，才进行恢复
        if (backup_size_after_load > m_history_traffic.Size())
        {
            m_history_traffic.Merge(backup_file, true);
            size_t size_after = m_history_traffic.Size();
            size_t recovered_count = size_after - size_before;  //实际恢复的记录数
            
            // 只有当实际恢复了记录时才记录日志
            if (recovered_count > 0)
            {
                CString log_info = CCommon::LoadTextFormat(IDS_HISTORY_TRAFFIC_LOST_ERROR_LOG, { size_before, recovered_count });
                CCommon::WriteLog(log_info, theApp.m_log_path.c_str());
            }
        }
    }

    theApp.m_today_up_traffic = m_history_traffic.GetTodayUpTraffic();
    theApp.m_today_down_traffic = m_history_traffic.GetTodayDownTraffic();
}

void CTrafficMonitorController::BackupHistoryTrafficFile()
{
    // 确保文件已保存到磁盘
    wstring latest_file_path = theApp.m_history_traffic_path;
    wstring backup_file_path = latest_file_path + L".bak";
    
    // 检查当前文件是否存在
    if (!CCommon::FileExist(latest_file_path.c_str()))
    {
        return; // 当前文件不存在，无需备份
    }
    
    // 直接备份当前文件（当前文件是最新的，包含最新的数据）
    // 备份文件可能包含"未来"的记录，但恢复时会自动清理，所以总是备份当前文件即可
    CopyFile(latest_file_path.c_str(), backup_file_path.c_str(), FALSE);
}

void CTrafficMonitorController::TaskbarShowHideItem(DisplayItem type)
{
    if (IsTaskbarWndValid())
    {
        bool show = (theApp.m_taskbar_data.display_item.Contains(type));
        if (show)
        {
            theApp.m_taskbar_data.display_item.Remove(type);
        }
        else
        {
            theApp.m_taskbar_data.display_item.Add(type);
        }
        //CloseTaskBarWnd();
        //OpenTaskBarWnd();
        m_tBarDlg->WidthChanged();
    }
}

void CTrafficMonitorController::DoMonitorAcquisition()
{
    //获取网络连接速度
    int rtn{};
    auto getLfTable = [&]() {
        __try
        {
            rtn = GetIfTable(m_pIfTable, &m_dwSize, FALSE);
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            free(m_pIfTable);
            m_dwSize = sizeof(MIB_IFTABLE);
            m_pIfTable = (MIB_IFTABLE*)malloc(m_dwSize);
            rtn = GetIfTable(m_pIfTable, &m_dwSize, FALSE);
            if (rtn == ERROR_INSUFFICIENT_BUFFER)	//如果函数返回值为ERROR_INSUFFICIENT_BUFFER，说明m_pIfTable的大小不够
            {
                free(m_pIfTable);
                m_pIfTable = (MIB_IFTABLE*)malloc(m_dwSize);	//用新的大小重新开辟一块内存
            }
            GetIfTable(m_pIfTable, &m_dwSize, FALSE);
        }
    };

    getLfTable();

    if (!theApp.m_cfg_data.m_select_all)        //获取当前选中连接的网速
    {
        auto table = GetConnectIfTable(m_connection_selected);
        m_in_bytes = table.dwInOctets;
        m_out_bytes = table.dwOutOctets;
    }
    else        //获取全部连接的网速
    {
        m_in_bytes = 0;
        m_out_bytes = 0;
        for (size_t i{}; i < m_connections.size(); i++)
        {
            auto table = GetConnectIfTable(i);
            //if (i > 0 && m_pIfTable->table[m_connections[i].index].dwInOctets == m_pIfTable->table[m_connections[i - 1].index].dwInOctets
            //  && m_pIfTable->table[m_connections[i].index].dwOutOctets == m_pIfTable->table[m_connections[i - 1].index].dwOutOctets)
            //  continue;       //连接列表中可能会有相同的连接，统计所有连接的网速时，忽略掉已发送和已接收字节数完全相同的连接
            m_in_bytes += table.dwInOctets;
            m_out_bytes += table.dwOutOctets;
        }
    }

    unsigned __int64 cur_in_speed{}, cur_out_speed{};       //本次监控时间间隔内的上传和下载速度

    //如果发送和接收的字节数为0或上次发送和接收的字节数为0或当前连接已改变时，网速无效
    if ((m_in_bytes == 0 && m_out_bytes == 0) || (m_last_in_bytes == 0 && m_last_out_bytes == 0) || m_connection_change_flag
        || m_last_in_bytes > m_in_bytes || m_last_out_bytes > m_out_bytes)
    {
        cur_in_speed = 0;
        cur_out_speed = 0;
    }
    else
    {
        cur_in_speed = m_in_bytes - m_last_in_bytes;
        cur_out_speed = m_out_bytes - m_last_out_bytes;
    }
    ////如果大于1GB/s，说明可能产生了异常，网速无效
    //if (cur_in_speed > 1073741824)
    //  cur_in_speed = 0;
    //if (cur_out_speed > 1073741824)
    //  cur_out_speed = 0;

    //计算两次获取网速的时间间隔
    static ULONGLONG last_net_speed_time = 0;
    ULONGLONG net_speed_time = CCommon::GetCurrentTimeSinceEpochMilliseconds();
    int time_span = theApp.m_general_data.monitor_time_span;
    if (last_net_speed_time != 0)
        time_span = static_cast<int>(net_speed_time - last_net_speed_time);
    last_net_speed_time = net_speed_time;

    //将当前监控时间间隔的流量转换成每秒时间间隔内的流量
    theApp.m_in_speed = static_cast<unsigned __int64>(cur_in_speed * 1000 / time_span);
    theApp.m_out_speed = static_cast<unsigned __int64>(cur_out_speed * 1000 / time_span);

    m_connection_change_flag = false;    //清除连接发生变化的标志

    m_last_in_bytes = m_in_bytes;
    m_last_out_bytes = m_out_bytes;

    //处于自动选择状态时，如果连续30秒没有网速，则可能自动选择的网络不对，此时执行一次自动选择
    if (theApp.m_cfg_data.m_auto_select)
    {
        if (cur_in_speed == 0 && cur_out_speed == 0)
            m_zero_speed_cnt++;
        else
            m_zero_speed_cnt = 0;
        if (m_zero_speed_cnt >= GetMonitorTimerCount(30))
        {
            AutoSelect();
            m_zero_speed_cnt = 0;
        }
    }

    //检测当前日期是否改变，如果已改变，就向历史流量列表插入一个新的日期
    SYSTEMTIME current_time;
    GetLocalTime(&current_time);
    static int last_check_day = -1;  //用于检测日期变化，重置保存状态
    if (m_history_traffic.GetTodayTraffic().day != current_time.wDay)
    {
        m_history_traffic.OnDateChanged();
        theApp.m_today_up_traffic = 0;
        theApp.m_today_down_traffic = 0;
        last_check_day = -1;  //重置日期标记，下次检查时会重新初始化保存状态
    }

    //统计今天已使用的流量
    theApp.m_today_up_traffic += cur_out_speed;
    theApp.m_today_down_traffic += cur_in_speed;
    m_history_traffic.GetTodayTraffic().up_kBytes = theApp.m_today_up_traffic / 1024u;
    m_history_traffic.GetTodayTraffic().down_kBytes = theApp.m_today_down_traffic / 1024u;
    //每隔30秒保存一次流量历史记录
    if (m_monitor_time_cnt % GetMonitorTimerCount(30) == GetMonitorTimerCount(30) - 1)
    {
        static unsigned __int64 last_today_kbytes = 0;
        static bool last_today_kbytes_initialized = false;
        unsigned __int64 current_kbytes = m_history_traffic.GetTodayTraffic().kBytes();
        
        //如果日期改变了，重置初始化状态
        if (last_check_day != current_time.wDay)
        {
            last_today_kbytes_initialized = false;
            last_check_day = current_time.wDay;
        }
        
        //首次检查时初始化，不保存
        if (!last_today_kbytes_initialized)
        {
            last_today_kbytes = current_kbytes;
            last_today_kbytes_initialized = true;
        }
        else
        {
            //只有当30秒内流量变化超过10MB时才保存历史流量记录，防止磁盘写入过于频繁
            unsigned __int64 change_kbytes = current_kbytes - last_today_kbytes;
            if (change_kbytes >= 10240u) // 10MB = 10240KB
            {
                SaveHistoryTraffic();
                last_today_kbytes = current_kbytes;
            }
        }
    }

    if (rtn == ERROR_INSUFFICIENT_BUFFER)
    {
        IniConnection();
        CString info = CCommon::LoadText(IDS_INSUFFICIENT_BUFFER);
        info.Replace(_T("<%cnt%>"), CCommon::IntToString(m_restart_cnt));
        CCommon::WriteLog(info, theApp.m_log_path.c_str());
    }

    if (m_monitor_time_cnt % GetMonitorTimerCount(3) == GetMonitorTimerCount(3) - 1)
    {
        //重新获取当前连接数量
        static DWORD last_interface_num = -1;
        DWORD interface_num;
        GetNumberOfInterfaces(&interface_num);
        if (last_interface_num != -1 && interface_num != last_interface_num)    //如果连接数发生变化，则重新初始化连接
        {
            if (theApp.m_debug_log)
            {
                CString info = CCommon::LoadText(IDS_CONNECTION_NUM_CHANGED);
                info.Replace(_T("<%before%>"), CCommon::IntToString(last_interface_num));
                info.Replace(_T("<%after%>"), CCommon::IntToString(interface_num));
                info.Replace(_T("<%cnt%>"), CCommon::IntToString(m_restart_cnt + 1));
                CCommon::WriteLog(info, theApp.m_log_path.c_str());
            }
            IniConnection();
            last_interface_num = interface_num;
        }

        string descr;
        descr = (const char*)GetConnectIfTable(m_connection_selected).bDescr;
        if (descr != theApp.m_cfg_data.m_connection_name)
        {
            //写入额外的调试信息
            if (theApp.m_debug_log)
            {
                CString log_str;
                log_str = _T("连接名称不匹配：\r\n");
                log_str += _T("IfTable description: ");
                log_str += descr.c_str();
                log_str += _T("\r\nm_connection_name: ");
                log_str += theApp.m_cfg_data.m_connection_name.c_str();
                CCommon::WriteLog(log_str, (theApp.m_config_dir + L".\\connections.log").c_str());
            }

            IniConnection();
            CString info = CCommon::LoadText(IDS_CONNECTION_NOT_MATCH);
            info.Replace(_T("<%cnt%>"), CCommon::IntToString(m_restart_cnt));
            CCommon::WriteLog(info, theApp.m_log_path.c_str());
        }
    }

    bool lite_version = false;
#ifdef WITHOUT_TEMPERATURE
    lite_version = true;
#endif

    bool cpu_freq_acquired = false;
    bool gpu_usage_acquired = false;
    m_get_disk_usage_by_pdh = false;

    //获取CPU使用率
    theApp.m_cpu_usage = m_cpu_usage_helper.GetCpuUsage(theApp.m_general_data.cpu_usage_acquire_method == GeneralSettingData::CA_CPU_TIME);

    //获取CPU频率
    //if (lite_version || is_arm64ec || !theApp.m_general_data.IsHardwareEnable(HI_CPU))
    //{
    if (m_cpu_freq_helper.GetCpuFreq(theApp.m_cpu_freq))
        cpu_freq_acquired = true;
    //}

    //获取GPU利用率
    if (lite_version /*|| is_arm64ec*/ || !theApp.m_general_data.IsHardwareEnable(HI_GPU))
    {
        if (m_gpu_usage_helper.GetGpuUsage(theApp.m_gpu_usage))
            gpu_usage_acquired = true;
        else
            theApp.m_gpu_usage = -1;
    }

    //获取硬盘利用率
    if (lite_version /*|| is_arm64ec*/ || !theApp.m_general_data.IsHardwareEnable(HI_HDD))
    {
        int disk_index = m_disk_usage_helper.FindDiskIndex(theApp.m_general_data.hard_disk_name);
        //没有找到要监控的硬盘时默认使用总体利用率
        if (disk_index < 0)
        {
            disk_index = m_disk_usage_helper.FindDiskIndex(L"_Total");
            if (disk_index >= 0)
            {
                theApp.m_general_data.hard_disk_name = L"_Total";
            }
            //仍然没有找到使用第1块硬盘
            else
            {
                const auto& disk_names = m_disk_usage_helper.GetDiskNames();
                if (!disk_names.empty())
                {
                    disk_index = 0;
                    theApp.m_general_data.hard_disk_name = disk_names.front();
                }
            }
        }
        if (m_disk_usage_helper.GetDiskUsage(disk_index, theApp.m_hdd_usage))
            m_get_disk_usage_by_pdh = true;
        else
            theApp.m_hdd_usage = -1;
    }

    //获取内存利用率
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    GlobalMemoryStatusEx(&statex);
    theApp.m_memory_usage = statex.dwMemoryLoad;
    theApp.m_used_memory = static_cast<int>((statex.ullTotalPhys - statex.ullAvailPhys) / 1024);
    theApp.m_total_memory = static_cast<int>(statex.ullTotalPhys / 1024);

#ifndef WITHOUT_TEMPERATURE
    //获取温度
    if (IsTemperatureNeeded() && theApp.m_pMonitor != nullptr)
    {
        CSingleLock sync(&theApp.m_minitor_lib_critical, TRUE);
        CString error_info = CCommon::LoadText(IDS_HARDWARE_INFO_ACQUIRE_FAILED_ERROR);

        auto getHardwareInfo = [&]() {
            __try
            {
                theApp.m_pMonitor->GetHardwareInfo();
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                AfxMessageBox(error_info, MB_ICONERROR | MB_OK);
            }
        };

        getHardwareInfo();
        auto monitor_error_message{ OpenHardwareMonitorApi::GetErrorMessage() };
        if (!monitor_error_message.empty())
        {
            AfxMessageBox(monitor_error_message.c_str(), MB_ICONERROR | MB_OK);
        }
        //theApp.m_cpu_temperature = theApp.m_pMonitor->CpuTemperature();
        theApp.m_gpu_temperature = theApp.m_pMonitor->GpuTemperature();
        //theApp.m_hdd_temperature = theApp.m_pMonitor->HDDTemperature();
        theApp.m_main_board_temperature = theApp.m_pMonitor->MainboardTemperature();
        if (!gpu_usage_acquired)
            theApp.m_gpu_usage = theApp.m_pMonitor->GpuUsage();
        if (!cpu_freq_acquired)
            theApp.m_cpu_freq = theApp.m_pMonitor->CpuFreq();
        //获取CPU温度
        if (!theApp.m_pMonitor->AllCpuTemperature().empty())
        {
            if (theApp.m_general_data.cpu_core_name == CCommon::LoadText(IDS_AVREAGE_TEMPERATURE).GetString())  //如果选择了平均温度
            {
                theApp.m_cpu_temperature = theApp.m_pMonitor->CpuTemperature();
            }
            else
            {
                auto iter = theApp.m_pMonitor->AllCpuTemperature().find(theApp.m_general_data.cpu_core_name);
                if (iter == theApp.m_pMonitor->AllCpuTemperature().end())
                {
                    iter = theApp.m_pMonitor->AllCpuTemperature().begin();
                    theApp.m_general_data.cpu_core_name = iter->first;
                }
                theApp.m_cpu_temperature = iter->second;
            }
        }
        else
        {
            theApp.m_cpu_temperature = -1;
        }
        //获取硬盘温度
        if (!theApp.m_pMonitor->AllHDDTemperature().empty())
        {
            auto iter = theApp.m_pMonitor->AllHDDTemperature().find(theApp.m_general_data.hard_disk_name);
            if (iter == theApp.m_pMonitor->AllHDDTemperature().end())
            {
                iter = theApp.m_pMonitor->AllHDDTemperature().begin();
                theApp.m_general_data.hard_disk_name = iter->first;
            }
            theApp.m_hdd_temperature = iter->second;
        }
        else
        {
            theApp.m_hdd_temperature = -1;
        }
        //获取硬盘利用率
        if (!m_get_disk_usage_by_pdh)
        {
            if (!theApp.m_pMonitor->AllHDDUsage().empty())
            {
                auto iter = theApp.m_pMonitor->AllHDDUsage().find(theApp.m_general_data.hard_disk_name);
                if (iter == theApp.m_pMonitor->AllHDDUsage().end())
                {
                    iter = theApp.m_pMonitor->AllHDDUsage().begin();
                    theApp.m_general_data.hard_disk_name = iter->first;
                }
                theApp.m_hdd_usage = iter->second;
            }
            else
            {
                theApp.m_hdd_usage = -1;
            }
        }
    }
#endif

    //通知插件获取数据，以及向插件传递监控数据
    for (const auto& plugin_info : theApp.m_plugins.GetPlugins())
    {
        if (plugin_info.plugin != nullptr)
        {
            plugin_info.plugin->DataRequired();
            ITMPlugin::MonitorInfo monitor_info;
            monitor_info.up_speed = theApp.m_out_speed;
            monitor_info.down_speed = theApp.m_in_speed;
            monitor_info.cpu_usage = theApp.m_cpu_usage;
            monitor_info.memory_usage = theApp.m_memory_usage;
            monitor_info.gpu_usage = theApp.m_gpu_usage;
            monitor_info.hdd_usage = theApp.m_hdd_usage;
            monitor_info.cpu_temperature = theApp.m_cpu_temperature;
            monitor_info.gpu_temperature = theApp.m_gpu_temperature;
            monitor_info.hdd_temperature = theApp.m_hdd_temperature;
            monitor_info.cpu_freq = theApp.m_cpu_freq;
            monitor_info.main_board_temperature = theApp.m_main_board_temperature;
            plugin_info.plugin->OnMonitorInfo(monitor_info);
        }
    }

    m_monitor_time_cnt++;

    //发送监控信息更新消息
    SendMessage(WM_MONITOR_INFO_UPDATED);
}

UINT CTrafficMonitorController::MonitorThreadCallback(LPVOID dwUser)
{
    CTrafficMonitorController* pThis = reinterpret_cast<CTrafficMonitorController*>(dwUser);
    if (pThis == nullptr)
        return 0;

    while (true)
    {
        ::WaitForSingleObject(pThis->m_monitor_request_event.m_hObject, INFINITE);
        if (pThis->m_is_thread_exit)
            break;

        pThis->DoMonitorAcquisition();
        pThis->m_monitor_data_required = false;
    }

    pThis->m_threadExitEvent.SetEvent();
    return 0;
}

void CTrafficMonitorController::OnNetworkInfo()
{
    // TODO: 在此添加命令处理程序代码
    //弹出“连接详情”对话框
    CNetworkInfoDlg aDlg(m_connections, m_pIfTable->table, m_connection_selected);
    ////向CNetworkInfoDlg类传递自启动以来已发送和接收的字节数
    //aDlg.m_in_bytes = m_pIfTable->table[m_connections[m_connection_selected].index].dwInOctets - m_connections[m_connection_selected].in_bytes;
    //aDlg.m_out_bytes = m_pIfTable->table[m_connections[m_connection_selected].index].dwOutOctets - m_connections[m_connection_selected].out_bytes;
    aDlg.m_start_time = m_start_time;
    aDlg.DoModal();
    //SetAlwaysOnTop(); //由于在“连接详情”对话框内设置了取消窗口置顶，所有在对话框关闭后，重新设置窗口置顶
    if (m_tBarDlg != nullptr)
        m_tBarDlg->m_tool_tips.SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);  //重新设置任务栏窗口的提示信息置顶
}

void CTrafficMonitorController::OnShowNotifyIcon()
{
    // TODO: 在此添加命令处理程序代码
    if (theApp.m_general_data.show_notify_icon)
    {
        DeleteNotifyIcon();
        theApp.m_general_data.show_notify_icon = false;
    }
    else
    {
        AddNotifyIcon();
        theApp.m_general_data.show_notify_icon = true;
    }
    theApp.SaveConfig();
}

void CTrafficMonitorController::OnShowCpuMemory2()
{
    // TODO: 在此添加命令处理程序代码
    if (IsTaskbarWndValid())
    {
        bool show_cpu_memory = (theApp.m_taskbar_data.display_item.Contains(TDI_CPU) || theApp.m_taskbar_data.display_item.Contains(TDI_MEMORY));
        if (show_cpu_memory)
        {
            theApp.m_taskbar_data.display_item.Remove(TDI_CPU);
            theApp.m_taskbar_data.display_item.Remove(TDI_MEMORY);
        }
        else
        {
            theApp.m_taskbar_data.display_item.Add(TDI_CPU);
            theApp.m_taskbar_data.display_item.Add(TDI_MEMORY);
        }
        //theApp.m_cfg_data.m_tbar_show_cpu_memory = !theApp.m_cfg_data.m_tbar_show_cpu_memory;
        //切换显示CPU和内存利用率时，删除任务栏窗口，再重新显示
        //CloseTaskBarWnd();
        //OpenTaskBarWnd();
        m_tBarDlg->WidthChanged();
    }
}

void CTrafficMonitorController::OnAppAbout()
{
    // TODO: 在此添加命令处理程序代码
    //弹出“关于”对话框
    CAboutDlg aDlg;
    aDlg.DoModal();
}

void CTrafficMonitorController::OnTrafficHistory()
{
    // TODO: 在此添加命令处理程序代码
    CHistoryTrafficDlg historyDlg(m_history_traffic.GetTraffics());
    historyDlg.DoModal();
}

void CTrafficMonitorController::OnOptions2()
{
    CWnd* pParent = this;
    if (IsTaskbarWndValid())
        pParent = m_tBarDlg;
    _OnOptions(1, pParent);
}

void CTrafficMonitorController::OnChangeNotifyIcon()
{
    // TODO: 在此添加命令处理程序代码
    CIconSelectDlg dlg(theApp.m_cfg_data.m_notify_icon_selected);
    dlg.SetAutoAdaptNotifyIcon(theApp.m_cfg_data.m_notify_icon_auto_adapt);
    if (dlg.DoModal() == IDOK)
    {
        theApp.m_cfg_data.m_notify_icon_selected = dlg.GetIconSelected();
        theApp.m_cfg_data.m_notify_icon_auto_adapt = dlg.AutoAdaptNotifyIcon();
        m_ntIcon.hIcon = theApp.m_notify_icons[theApp.m_cfg_data.m_notify_icon_selected];
        if (theApp.m_cfg_data.m_notify_icon_auto_adapt)
            theApp.AutoSelectNotifyIcon();
        if (theApp.m_general_data.show_notify_icon)
        {
            DeleteNotifyIcon();
            AddNotifyIcon();
        }
        theApp.SaveConfig();
    }
}

void CTrafficMonitorController::OnCheckUpdate()
{
    // TODO: 在此添加命令处理程序代码
    theApp.CheckUpdateInThread(true);
}

void CTrafficMonitorController::OnAutoRunWhenStart()
{
    bool registry_auto_run = theApp.GetAutoRun(nullptr, false);
    bool task_scheduler_auto_run = theApp.GetAutoRun(nullptr, true);
    bool currently_enabled = registry_auto_run || task_scheduler_auto_run;

#ifdef WITHOUT_TEMPERATURE
    // Lite uses the per-user Registry startup entry and avoids the elevated task-scheduler path.
    const bool enable = !currently_enabled;
    if (theApp.SetAutoRun(enable, false))
    {
        theApp.m_general_data.auto_run = enable;
    }
#else
    // Full builds keep the user's existing startup method where possible.
    const bool enable = !currently_enabled;
    const bool use_task_scheduler = task_scheduler_auto_run;
    if (theApp.SetAutoRun(enable, use_task_scheduler))
    {
        theApp.m_general_data.auto_run = enable;
        theApp.m_general_data.auto_run_by_task_scheduler = use_task_scheduler;
    }
#endif
    theApp.SaveConfig();
}

afx_msg LRESULT CTrafficMonitorController::OnTaskbarMenuPopedUp(WPARAM wParam, LPARAM lParam)
{
    //设置“选择连接”子菜单项中各单选项的选择状态
    SetConnectionMenuState(theApp.m_taskbar_menu.GetSubMenu(0)->GetSubMenu(0));
    SetConnectionMenuState(theApp.m_taskbar_menu_plugin.GetSubMenu(0)->GetSubMenu(0));
    return 0;
}

void CTrafficMonitorController::OnShowNetSpeed()
{
    // TODO: 在此添加命令处理程序代码
    if (IsTaskbarWndValid())
    {
        bool show_net_speed = (theApp.m_taskbar_data.display_item.Contains(TDI_UP) || theApp.m_taskbar_data.display_item.Contains(TDI_DOWN));
        if (show_net_speed)
        {
            theApp.m_taskbar_data.display_item.Remove(TDI_UP);
            theApp.m_taskbar_data.display_item.Remove(TDI_DOWN);
        }
        else
        {
            theApp.m_taskbar_data.display_item.Add(TDI_UP);
            theApp.m_taskbar_data.display_item.Add(TDI_DOWN);
        }
        //CloseTaskBarWnd();
        //OpenTaskBarWnd();
        m_tBarDlg->WidthChanged();
    }
}

BOOL CTrafficMonitorController::OnQueryEndSession()
{
    if (!CDialog::OnQueryEndSession())
        return FALSE;

    // TODO:  在此添加专用的查询结束会话代码
    theApp.SaveConfig();
    theApp.SaveGlobalConfig();
    SaveHistoryTrafficFull();  // 系统关机时使用完整保存，确保所有数据都保存
    BackupHistoryTrafficFile();

    if (theApp.m_debug_log)
    {
        CCommon::WriteLog(_T("TrafficMonitor进程已被终止，设置已保存。"), (theApp.m_config_dir + L".\\debug.log").c_str());
    }

    return TRUE;
}

afx_msg LRESULT CTrafficMonitorController::OnTaskbarWndClosed(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);

    theApp.m_cfg_data.m_show_task_bar_wnd = true;
    PostMessage(WM_REOPEN_TASKBAR_WND, 0, 0);
    return 0;
}

void CTrafficMonitorController::OnPluginManage()
{
    // TODO: 在此添加命令处理程序代码
    CPluginManagerDlg dlg;
    dlg.DoModal();
}

LRESULT CTrafficMonitorController::OnReopenTaksbarWnd(WPARAM wParam, LPARAM lParam)
{
    CloseTaskBarWnd();
    OpenTaskBarWnd();
    return 0;
}

void CTrafficMonitorController::OnOpenTaskManager()
{
    ShellExecuteW(NULL, _T("open"), (theApp.m_system_dir + L"\\Taskmgr.exe").c_str(), NULL, NULL, SW_NORMAL);       //打开任务管理器
}

afx_msg LRESULT CTrafficMonitorController::OnSettingsApplied(WPARAM wParam, LPARAM lParam)
{
    COptionsDlg* pOptionsDlg = (COptionsDlg*)wParam;
    if (pOptionsDlg != nullptr)
    {
        ApplySettings(*pOptionsDlg);
    }
    return 0;
}

void CTrafficMonitorController::OnDisplaySettings()
{
    // TODO: 在此添加命令处理程序代码
    CSetItemOrderDlg dlg;
    dlg.SetItemOrder(theApp.m_taskbar_data.item_order.GetItemOrderConst());
    dlg.SetDisplayItem(theApp.m_taskbar_data.display_item);
    dlg.SetPluginDisplayItem(theApp.m_taskbar_data.plugin_display_item);
    if (dlg.DoModal() == IDOK)
    {
        theApp.m_taskbar_data.item_order.SetOrder(dlg.GetItemOrder());
        theApp.m_taskbar_data.display_item = dlg.GetDisplayItem();
        theApp.m_taskbar_data.plugin_display_item = dlg.GetPluginDisplayItem();
        //CloseTaskBarWnd();
        //OpenTaskBarWnd();
        if (IsTaskbarWndValid())
            m_tBarDlg->WidthChanged();
    }
}

void CTrafficMonitorController::OnRefreshConnectionList()
{
    IniConnection();

}

void CTrafficMonitorController::OnPluginOptionsTaksbar()
{
    //判断任务栏窗口中点击的项目是否是插件项目
    if (IsTaskbarWndValid() && m_tBarDlg->GetClickedItem().IsPlugin())
    {
        //找到对应的插件
        ITMPlugin* plugin = theApp.m_plugins.GetPluginByItem(m_tBarDlg->GetClickedItem().PluginItem());
        if (plugin != nullptr)
        {
            //显示插件的选项设置
            auto rtn = plugin->ShowOptionsDialog(m_tBarDlg->GetSafeHwnd());
            if (rtn == ITMPlugin::OR_OPTION_CHANGED)    //选项设置有更改，重新打开任务栏窗口
            {
                //CloseTaskBarWnd();
                //OpenTaskBarWnd();
                m_tBarDlg->WidthChanged();
            }
            if (rtn == ITMPlugin::OR_OPTION_NOT_PROVIDED)
                MessageBox(CCommon::LoadText(IDS_PLUGIN_NO_OPTIONS_INFO), nullptr, MB_ICONINFORMATION | MB_OK);
        }
    }
}

void CTrafficMonitorController::OnPluginDetailTaksbar()
{
    if (IsTaskbarWndValid() && m_tBarDlg->GetClickedItem().IsPlugin())
    {
        //找到对应的插件
        ITMPlugin* plugin = theApp.m_plugins.GetPluginByItem(m_tBarDlg->GetClickedItem().PluginItem());
        if (plugin != nullptr)
        {
            int index = theApp.m_plugins.GetPluginIndex(plugin);
            CPluginInfoDlg dlg(index);
            dlg.DoModal();
        }
    }
}

