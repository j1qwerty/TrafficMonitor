#pragma once
#pragma comment (lib, "iphlpapi.lib")
#include "afxwin.h"
#include "Common.h"
#include "AdapterCommon.h"
#include "HistoryTrafficFile.h"
#include "PdhHardwareQuery/CPUUsage.h"
#include "PdhHardwareQuery/CpuFreq.h"
#include "PdhHardwareQuery/GpuUsage.h"
#include "PdhHardwareQuery/DiskUsage.h"

class CTaskBarDlg;
class COptionsDlg;

class CTrafficMonitorController : public CDialog
{
public:
    explicit CTrafficMonitorController(UINT nIDTemplate, CWnd* pParent = nullptr);
    ~CTrafficMonitorController() override;

    CTaskBarDlg* GetTaskbarWindow() const;
    static CTrafficMonitorController* Instance();
    virtual void SetAlwaysOnTop() {}
    virtual void ApplySettings(COptionsDlg& optionsDlg);

    bool IsTaskbarWndValid() const;
    bool IsTemperatureNeeded() const;

    void ExitMonitorThread();
    void ShowNotifyTip(const wchar_t* title, const wchar_t* message);

protected:
    NOTIFYICONDATA m_ntIcon{};
    CTaskBarDlg* m_tBarDlg{};
    vector<NetWorkConection> m_connections;
    MIB_IFTABLE* m_pIfTable{};
    DWORD m_dwSize{};
    int m_connection_selected{};
    unsigned __int64 m_in_bytes{};
    unsigned __int64 m_out_bytes{};
    unsigned __int64 m_last_in_bytes{};
    unsigned __int64 m_last_out_bytes{};
    CCPUUsage m_cpu_usage_helper;
    CPdhCpuFreq m_cpu_freq_helper;
    CPdhGPUUsage m_gpu_usage_helper;
    CPdhDiskUsage m_disk_usage_helper;
    bool m_get_disk_usage_by_pdh{};
    int m_restart_cnt{ -1 };
    unsigned int m_monitor_time_cnt{};
    int m_zero_speed_cnt{};
    int m_insert_to_taskbar_cnt{};
    int m_cannot_insert_to_task_bar_warning{ true };
    static unsigned int m_WM_TASKBARCREATED;
    SYSTEMTIME m_start_time{};
    CHistoryTrafficFile m_history_traffic;
    bool m_connection_change_flag{};
    bool m_is_thread_exit{};
    bool m_monitor_data_required{};
    CEvent m_monitor_request_event;
    CEvent m_threadExitEvent;
    string m_connection_name_preferd;

    void AutoSelect();
    void IniConnection();
    MIB_IFROW GetConnectIfTable(int connection_index);
    NetWorkConection GetConnection(int connection_index);
    void IniConnectionMenu(CMenu* pMenu);
    void IniTaskBarConnectionMenu();
    void SetConnectionMenuState(CMenu* pMenu);
    void CloseTaskBarWnd();
    void OpenTaskBarWnd();
    void AddNotifyIcon();
    void DeleteNotifyIcon();
    void UpdateNotifyIconTip();
    void SaveHistoryTraffic();
    void SaveHistoryTrafficFull();
    void LoadHistoryTraffic();
    void BackupHistoryTrafficFile();
    void _OnOptions(int tab, CWnd* pParent);
    void TaskbarShowHideItem(DisplayItem type);
    void DoMonitorAcquisition();
    static UINT MonitorThreadCallback(LPVOID dwUser);

    virtual BOOL OnInitDialog();
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
    DECLARE_MESSAGE_MAP()

    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnNetworkInfo();
    afx_msg LRESULT OnTaskBarCreated(WPARAM wParam, LPARAM lParam);
    afx_msg void OnShowNotifyIcon();
    afx_msg void OnShowCpuMemory2();
    afx_msg void OnAppAbout();
    afx_msg void OnTrafficHistory();
    afx_msg void OnOptions2();
    afx_msg void OnChangeNotifyIcon();
    afx_msg void OnCheckUpdate();
    afx_msg void OnAutoRunWhenStart();
    afx_msg LRESULT OnTaskbarMenuPopedUp(WPARAM, LPARAM);
    afx_msg void OnShowNetSpeed();
    afx_msg BOOL OnQueryEndSession();
    afx_msg void OnClose();
    afx_msg void OnDestroy();
    afx_msg LRESULT OnTaskbarWndClosed(WPARAM, LPARAM);
    afx_msg LRESULT OnMonitorInfoUpdated(WPARAM, LPARAM);
    afx_msg LRESULT OnDisplaychange(WPARAM, LPARAM);
    afx_msg void OnPluginManage();
    afx_msg LRESULT OnReopenTaksbarWnd(WPARAM, LPARAM);
    afx_msg void OnOpenTaskManager();
    afx_msg LRESULT OnSettingsApplied(WPARAM, LPARAM);
    afx_msg void OnDisplaySettings();
    afx_msg void OnRefreshConnectionList();
    afx_msg void OnPluginOptionsTaksbar();
    afx_msg void OnPluginDetailTaksbar();
    afx_msg UINT OnPowerBroadcast(UINT nPowerEvent, LPARAM nEventData);
};
