#pragma once

#include "PasswordGenerator.hpp"
#include "delegate/RowColorDelegate.hpp"
#include "Struct/Storage.hpp"
#include "model/PasswordsModel.hpp"

#include <QMainWindow>
#include <QSortFilterProxyModel>

namespace Ui {
    class MainWindow;
} // namespace Ui

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() noexcept;

private slots:
    // Menu actions
    void OnActionCreateDatabase();
    void OnActionOpenDatabase();
    void OnActionUnlockDatabase();
    bool OnCreateLayer(bool is_last = false);

    // Show page
    void OnPushButtonRemoveEntry();
    void OnPushButtonShowPasswords();

    // Password Generator page
    void UpdateGeneratedPassword();

    // Settings page
    void UpdateSettings();

private:
    bool eventFilter(QObject* target, QEvent* event) override;

    void ConnectActionSlots();
    void ConnectSlots();

    void SetCurrentPage(int idx);
    void CopyToClipboard(const QString& text);
    void SetPGFlag(core::PasswordGenerator::Flag flag, const bool use);
    void SetupSettings();

    // Timers
    void StartCopyTimer();
    void StartLockDBTimer();

private:
    std::shared_ptr<Ui::MainWindow> m_ui;
    std::unique_ptr<core::Storage> m_storage;
    std::shared_ptr<PasswordsModel> m_passwords_model;
    std::unique_ptr<QSortFilterProxyModel> m_passwords_proxy_model;
    int m_prev_page{};
    std::unique_ptr<RowColorDelegate> m_row_color_delegate;

    std::unique_ptr<QTimer> m_copy_timer;
    std::unique_ptr<QTimer> m_lock_db_timer;
};