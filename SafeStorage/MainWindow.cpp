// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "MainWindow.hpp"
#include "Util.hpp"
#include "Settings.hpp"
#include "ui/ui_MainWindow.h"

#include <QClipboard>
#include <QFileDialog>
#include <QInputEvent>
#include <QMessageBox>
#include <QSpinBox>
#include <QTimer>

enum Pages : int
{
    Start,
    UnlockDB,
    StorageParams,
    Settings,
    Show,
    PasswordGenerator,
};

MainWindow::MainWindow(QWidget* parent /* = nullptr */)
    : QMainWindow(parent)
    , m_ui(std::make_shared<Ui::MainWindow>())
    , m_passwords_model(std::make_shared<PasswordsModel>())
    , m_passwords_proxy_model(std::make_unique<QSortFilterProxyModel>())
    , m_row_color_delegate(std::make_unique<RowColorDelegate>())
    , m_copy_timer(std::make_unique<QTimer>(this))
    , m_lock_db_timer(std::make_unique<QTimer>(this))
{
    QApplication::instance()->installEventFilter(this);

    m_ui->setupUi(this);
    ConnectActionSlots();
    ConnectSlots();
    SetupSettings();

    m_ui->stackedWidget->setCurrentIndex(Pages::Start);

    m_ui->tableView->setItemDelegate(m_row_color_delegate.get());
}

#pragma region Menu

void MainWindow::OnActionCreateDatabase()
{
    if (!OnCreateLayer(true))
        return;

    QString filepath = QFileDialog::getSaveFileName(this, tr("Create database"), QDir::currentPath() + QDir::separator() + m_ui->lineEditDatabaseName->text(), tr("Safe Storage Database (*.ssdb)"));
    if (filepath.isEmpty())
        return;

    m_storage->SaveToFile(filepath.toStdString());

    m_passwords_model->SetData(m_storage->GetPasswords());
    m_passwords_proxy_model->setSourceModel(m_passwords_model.get());
    m_ui->tableView->setModel(m_passwords_model.get());

    SetCurrentPage(Pages::Show);

    // Invalidate
    m_ui->lineEditDatabaseName->setText("");
}

MainWindow::~MainWindow() noexcept
{
    core::Settings::Save();
}

void MainWindow::OnActionOpenDatabase()
{
    QString filepath = QFileDialog::getOpenFileName(this, tr("Open database"), QDir::currentPath(), tr("Safe Storage Database (*.ssdb)"));
    if (filepath.isEmpty())
        return;

    m_storage = std::make_unique<core::Storage>();
    if (!m_storage || !m_storage->LoadFromFile(filepath.toStdString()))
    {
        QMessageBox::critical(this, tr("Error"), tr("Failed to open database"), QMessageBox::StandardButton::Ok);

        SetCurrentPage(Pages::Start);
        return;
    }

    m_ui->labelUnlockDBPath->setText(filepath);
    SetCurrentPage(Pages::UnlockDB);
}

void MainWindow::OnActionUnlockDatabase()
{
    const auto update_ui = [&] {
        const auto layers_num = m_storage->GetLayersNumber();
        m_ui->labelUnlockDBLayer->setText(tr("Layer: ") + QString::number(layers_num ? layers_num : 1));
        m_ui->lineEditUnlockDBEnterPassword->setText("");
    };

    if (!m_storage->UnlockLayer(m_ui->lineEditUnlockDBEnterPassword->text().toStdString()))
    {
        QMessageBox::critical(this, tr("Error"), tr("Failed to unlock layer"), QMessageBox::StandardButton::Ok);
        update_ui();
        return;
    }

    if (!m_storage->IsLastLayerDecrypted())
    {
        update_ui();
        return;
    }

    m_passwords_model->SetData(m_storage->GetPasswords());
    m_passwords_proxy_model->setSourceModel(m_passwords_model.get());
    m_ui->tableView->setModel(m_passwords_model.get());

    SetCurrentPage(Pages::Show);

    if (core::Settings::startup.minimize_after_unlocking)
        setWindowState(Qt::WindowState::WindowMinimized);

    // Invalidate
    update_ui();
}

bool MainWindow::OnCreateLayer(bool is_last /*= false*/)
{
    auto lineEditEnterPassword = m_ui->lineEditEnterPassword;
    if (auto pass = lineEditEnterPassword->text(), confirmed = m_ui->lineEditConfirmPassword->text();
        pass.isEmpty() || confirmed.isEmpty() || pass != confirmed)
    {
        QMessageBox::critical(this, tr("Error"), tr("Passwords do not match"), QMessageBox::StandardButton::Ok);
        return false;
    }

    if (!m_storage)
        m_storage = std::make_unique<core::Storage>();

    if (!m_storage)
    {
        QMessageBox::critical(this, tr("Error"), tr("Failed to create database"), QMessageBox::StandardButton::Ok);
        return false;
    }

    m_storage->AddLayer(core::Layer(
        lineEditEnterPassword->text().toStdString(),
        static_cast<core::CipherId>(m_ui->comboBoxEncryptionAlgorithm->currentIndex() + 1),
        static_cast<uint64_t>(m_ui->spinBoxTransformRounds->value()),
        static_cast<core::CompressionType>(m_ui->comboBoxCompressionType->currentIndex()),
        static_cast<core::RandomStreamId>(m_ui->comboBoxStreamCipher->currentIndex()),
        is_last)
    );

    // invalidate input
    lineEditEnterPassword->setText("");
    m_ui->lineEditConfirmPassword->setText("");

    return true;
}

#pragma endregion

#pragma region Show page

void MainWindow::OnPushButtonRemoveEntry()
{
    const auto select = m_ui->tableView->selectionModel();
    if (!select || !select->hasSelection())
        return;

    QMessageBox msg_box(this);
    msg_box.setIcon(QMessageBox::Icon::Warning);
    msg_box.setWindowTitle(tr("Warning"));
    msg_box.setText(tr("Are you sure you want to delete this entry?"));
    msg_box.setStandardButtons(QMessageBox::StandardButton::Apply | QMessageBox::StandardButton::Cancel);
    msg_box.setDefaultButton(QMessageBox::StandardButton::Cancel);
    if (const auto ret = msg_box.exec(); ret == QMessageBox::StandardButton::Apply)
    {
        m_passwords_model->RemoveEntry(select->selectedRows().first());
        m_row_color_delegate->SetRows(std::move(m_passwords_model->GetLeakedPasswords()));
        m_ui->tableView->reset();
    }
}

void MainWindow::OnPushButtonShowPasswords()
{
    const auto cur_show = m_passwords_model->GetShowPasswords();
    m_passwords_model->SetShowPasswords(!cur_show);
    m_ui->pushButtonShowPasswords->setText(cur_show ? tr("Show") : tr("Hide"));
    m_ui->tableView->reset();
}

#pragma endregion

bool MainWindow::eventFilter(QObject* target, QEvent* event)
{
    if (dynamic_cast<QInputEvent*>(event) && m_storage && m_storage->IsLastLayerDecrypted() && m_ui->stackedWidget->currentIndex() == Pages::Show)
        StartLockDBTimer();

    return QWidget::eventFilter(target, event);
}

void MainWindow::ConnectActionSlots()
{
    // Database
    connect(m_ui->actionNewDatabase, &QAction::triggered, [this] { SetCurrentPage(Pages::StorageParams); });
    connect(m_ui->actionOpenDatabase, SIGNAL(triggered()), this, SLOT(OnActionOpenDatabase()));
    connect(m_ui->actionQuit, &QAction::triggered, [this] {
        if (m_storage)
            m_storage->SaveToFile();

        close();
    });

    // Tools
    connect(m_ui->actionPasswordGenerator, &QAction::triggered, [this] { SetCurrentPage(Pages::PasswordGenerator); });
    connect(m_ui->actionSettings, &QAction::triggered, [this] { SetCurrentPage(Pages::Settings); });
}

void MainWindow::ConnectSlots()
{
    auto return_to_start = [this] { SetCurrentPage(Pages::Start); };

    // Start page
    connect(m_ui->pushButtonCreateNewDatabase, &QPushButton::clicked, [this] { SetCurrentPage(Pages::StorageParams); });
    connect(m_ui->pushButtonOpenExistingDatabase, SIGNAL(clicked()), this, SLOT(OnActionOpenDatabase()));

    // Storage params
    connect(m_ui->pushButtonCreateLayer, SIGNAL(clicked()), this, SLOT(OnCreateLayer()));
    connect(m_ui->pushButtonDone, SIGNAL(clicked()), this, SLOT(OnActionCreateDatabase()));
    connect(m_ui->pushButtonCancel, &QPushButton::clicked, return_to_start);

    // UnlockDB page
    connect(m_ui->pushButtonUnlockDB, SIGNAL(clicked()), this, SLOT(OnActionUnlockDatabase()));
    connect(m_ui->pushButtonUnlockDBClose, &QPushButton::clicked, [&] {
        if (m_storage)
            m_storage->Clear();
        m_ui->lineEditUnlockDBEnterPassword->setText("");
        SetCurrentPage(Pages::Start);
    });

    // Show page
    connect(m_ui->pushButtonAddEntry, &QPushButton::clicked, [&] {
        m_passwords_model->AddEntry();
        m_row_color_delegate->SetRows(std::move(m_passwords_model->GetLeakedPasswords()));
    });
    connect(m_ui->pushButtonRemoveEntry, SIGNAL(clicked()), this, SLOT(OnPushButtonRemoveEntry()));
    connect(m_ui->pushButtonShowPasswords, SIGNAL(clicked()), this, SLOT(OnPushButtonShowPasswords()));
    connect(m_ui->pushButtonShowClose, &QPushButton::clicked, [&] {
        m_storage->SaveToFile();
        m_storage->Clear();
        SetCurrentPage(Pages::Start);
    });
    connect(m_ui->pushButtonShowCheck, &QPushButton::clicked, [&] {
        auto&& leaked = m_passwords_model->CheckPasswords();
        const auto leaked_num = leaked.size();
        m_row_color_delegate->SetRows(std::move(leaked));
        m_ui->tableView->update();
        QMessageBox::warning(this, tr("Passwords checking"), tr("Found %1 leaked passwords").arg(leaked_num), QMessageBox::StandardButton::Ok);
    });

    // Password Generator page
    auto& pg = core::PasswordGenerator::GetInstance();
    connect(m_ui->pushButtonGenerate, SIGNAL(clicked()), this, SLOT(UpdateGeneratedPassword()));
    connect(m_ui->pushButtonCopy, &QPushButton::clicked, [&] { CopyToClipboard(m_ui->lineEditGeneratedPassword->text()); });
    connect(m_ui->pushButtonClosePG, &QPushButton::clicked, [&] { SetCurrentPage(m_prev_page); });

    connect(m_ui->horizontalSliderPasswordLength, &QSlider::valueChanged, [&](const uint8_t value) {
        pg.SetLength(value);
        m_ui->spinBoxPasswordLength->setValue(value);
        UpdateGeneratedPassword();
    });
    connect(m_ui->spinBoxPasswordLength, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [&](const uint8_t value) {
        pg.SetLength(value);
        m_ui->horizontalSliderPasswordLength->setValue(value);
        UpdateGeneratedPassword();
    });

    using pg_t = core::PasswordGenerator;
    connect(m_ui->checkBoxUseCapitals,    &QCheckBox::clicked, [&](const bool use) { SetPGFlag(pg_t::Flag::Capitals,    use); });
    connect(m_ui->checkBoxUseLowercase,   &QCheckBox::clicked, [&](const bool use) { SetPGFlag(pg_t::Flag::Lowercase,   use); });
    connect(m_ui->checkBoxUseDigits,      &QCheckBox::clicked, [&](const bool use) { SetPGFlag(pg_t::Flag::Digits,      use); });
    connect(m_ui->checkBoxUseSpecial1,    &QCheckBox::clicked, [&](const bool use) { SetPGFlag(pg_t::Flag::Special1,    use); });
    connect(m_ui->checkBoxUseSpecial2,    &QCheckBox::clicked, [&](const bool use) { SetPGFlag(pg_t::Flag::Special2,    use); });
    connect(m_ui->checkBoxUseSpecial3,    &QCheckBox::clicked, [&](const bool use) { SetPGFlag(pg_t::Flag::Special3,    use); });
    connect(m_ui->checkBoxUseSpecial4,    &QCheckBox::clicked, [&](const bool use) { SetPGFlag(pg_t::Flag::Special4,    use); });
    connect(m_ui->checkBoxUseSpecial5,    &QCheckBox::clicked, [&](const bool use) { SetPGFlag(pg_t::Flag::Special5,    use); });
    connect(m_ui->checkBoxUseParentheses, &QCheckBox::clicked, [&](const bool use) { SetPGFlag(pg_t::Flag::Parentheses, use); });

    // Settings page
    connect(m_ui->pushButtonSettingsSave, &QPushButton::clicked, [&] {
        UpdateSettings();
        SetCurrentPage(m_prev_page);
    });
    connect(m_ui->pushButtonSettingsCancel, &QPushButton::clicked, [&] { SetupSettings(); SetCurrentPage(m_prev_page); });
    connect(m_ui->checkBoxClearClipboard, &QCheckBox::toggled, [&](const bool toggled) { m_ui->spinBoxClearClipboardAfter->setEnabled(toggled); });
    connect(m_ui->checkBoxLockDatabaseAfterInactivity, &QCheckBox::toggled, [&](const bool toggled) { m_ui->spinBoxLockDatabaseAfterInactivity->setEnabled(toggled); });
}

void MainWindow::SetCurrentPage(int idx)
{
    m_prev_page = m_ui->stackedWidget->currentIndex();
    m_ui->stackedWidget->setCurrentIndex(idx);
}

void MainWindow::CopyToClipboard(const QString& text)
{
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(text, QClipboard::Clipboard);
    if (clipboard->supportsSelection())
        clipboard->setText(text, QClipboard::Selection);

#ifdef Q_OS_LINUX
    QThread::msleep(1);
#endif
}

void MainWindow::UpdateGeneratedPassword()
{
    const auto& pg = core::PasswordGenerator::GetInstance();
    const auto text = QString::fromStdString(pg.GeneratePassword());
    m_ui->lineEditGeneratedPassword->setText(text);
    m_ui->pushButtonCopy->setEnabled(!text.isEmpty());
    m_ui->labelEntropy->setText(tr("Entropy: ") + QString::number(pg.GetEntropy()) + tr(" bit"));
}

void MainWindow::UpdateSettings()
{
    using namespace core;
    using namespace std::chrono;
    using namespace std::chrono_literals;
    
    // Startup
    Settings::startup.start_only_one_instance = m_ui->checkBoxStartOnlyASingleInstance->isChecked();
    Settings::startup.launch_at_system_startup =m_ui->checkBoxAutomaticallyLaunchAtSystemStartup->isChecked();
    Settings::startup.minimize_at_startup = m_ui->checkBoxMinimizeWindowAtApplicationStartup->isChecked();
    Settings::startup.minimize_after_unlocking = m_ui->checkBoxMinimizeWindowAfterUnlockingDatabase->isChecked();
    Settings::startup.remember_databases = m_ui->checkBoxRememberPreviouslyUsedDatabases->isChecked();

    // Timeouts
    Settings::timeouts.clear_clipboard = m_ui->checkBoxClearClipboard->isChecked() ? seconds(m_ui->spinBoxClearClipboardAfter->value()) : 0s;
    Settings::timeouts.lock_db = m_ui->checkBoxLockDatabaseAfterInactivity->isChecked() ? seconds(m_ui->spinBoxLockDatabaseAfterInactivity->value()) : 0s;

    // Security

    core::Settings::Save();
}

void MainWindow::SetPGFlag(core::PasswordGenerator::Flag flag, const bool use)
{
    auto& pg = core::PasswordGenerator::GetInstance();
    pg.SetUseFlag(flag, use);
    UpdateGeneratedPassword();

    m_ui->pushButtonGenerate->setEnabled(pg.IsAnyFlagSet());
}

void MainWindow::SetupSettings()
{
    using namespace core;

    // Startup
    m_ui->checkBoxStartOnlyASingleInstance->setChecked(Settings::startup.start_only_one_instance);
    m_ui->checkBoxAutomaticallyLaunchAtSystemStartup->setChecked(Settings::startup.launch_at_system_startup);
    m_ui->checkBoxMinimizeWindowAtApplicationStartup->setChecked(Settings::startup.minimize_at_startup);
    m_ui->checkBoxMinimizeWindowAfterUnlockingDatabase->setChecked(Settings::startup.minimize_after_unlocking);
    m_ui->checkBoxRememberPreviouslyUsedDatabases->setChecked(Settings::startup.remember_databases);

    // Timeouts
    if (auto cnt = Settings::timeouts.clear_clipboard.count())
    {
        m_ui->checkBoxClearClipboard->setChecked(true);
        m_ui->spinBoxClearClipboardAfter->setEnabled(true);
        m_ui->spinBoxClearClipboardAfter->setValue(cnt);
    }

    if (auto cnt = Settings::timeouts.lock_db.count())
    {
        m_ui->checkBoxLockDatabaseAfterInactivity->setChecked(true);
        m_ui->spinBoxLockDatabaseAfterInactivity->setEnabled(true);
        m_ui->spinBoxLockDatabaseAfterInactivity->setValue(cnt);
    }

    // Security
}

#pragma region Timers

void MainWindow::StartCopyTimer()
{
    if (!core::Settings::timeouts.clear_clipboard.count())
        return;

    m_copy_timer->start(core::Settings::timeouts.clear_clipboard);
    connect(m_copy_timer.get(), &QTimer::timeout, [this] {
        CopyToClipboard("");
    });
}

void MainWindow::StartLockDBTimer()
{
    if (!core::Settings::timeouts.lock_db.count())
        return;

    m_lock_db_timer->start(core::Settings::timeouts.clear_clipboard);
    connect(m_lock_db_timer.get(), &QTimer::timeout, [this] {
        if (!m_storage || m_ui->stackedWidget->currentIndex() != Pages::Show)
            return;

        const auto path = m_storage->GetPath().string();
        m_storage->SaveToFile();
        m_storage->Clear();
        SetCurrentPage(Pages::Start);
    });
}

#pragma endregion