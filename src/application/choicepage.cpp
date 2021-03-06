/***************************************************************************
 *   Copyright 2001 Bernd Gehrmann <bernd@kdevelop.org>                    *
 *   Copyright 2004-2005 Sascha Cunz <sascha@kdevelop.org>                 *
 *   Copyright 2007 Alexander Dymo <adymo@kdevelop.org>                    *
 *   Copyright 2008 Anne-Marie Mahfouf <annma@kde.org>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include <KLocalizedString>

#include <QDir>
#include <QString>
#include <QPixmap>
#include <QStandardItem>
#include <QStandardPaths>
#include <QFileDialog>
#include <QPointer>

#include <KNS3/DownloadDialog>
#include <KTar>
#include <KZip>

#include "choicepage.h"
#include "apptemplatesmodel.h"
#include "prefs.h"
#include "logging.h"

ChoicePage::ChoicePage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(i18n("Choose your project template"));
    ui_choice.setupUi(this);
    //Get the model
    templatesModel = new AppTemplatesModel(this);
    templatesModel->refresh();
    ui_choice.appTree->setModel(templatesModel);
    ui_choice.appTree->expandAll();
    connect(ui_choice.kcfg_appName, SIGNAL(textChanged(const QString &)), this, SIGNAL(completeChanged()));
    connect(this, SIGNAL(completeChanged()), this, SLOT(saveConfig()));
    connect(ui_choice.appTree->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)), this, SLOT(itemSelected(QModelIndex)));
    connect(ui_choice.installButton, &QPushButton::clicked, this, &ChoicePage::loadFromFile);
    connect(ui_choice.getNewButton, &QPushButton::clicked, this, &ChoicePage::getMoreTemplates);
    QRegExp rx("[a-zA-Z0-9_]*");
    QValidator *validator = new QRegExpValidator(rx, this);
    ui_choice.kcfg_appName->setValidator(validator);
    registerField("appName*", ui_choice.kcfg_appName);
    registerField("tempName", this, "templateName", "templateNameChanged");
}

bool ChoicePage::isComplete () const
{
    if (!m_baseName.isEmpty() && !ui_choice.kcfg_appName->text().isEmpty()) {
        return true;
    }
    return false;
}

void ChoicePage::saveConfig()
{
    Prefs::setAppName(ui_choice.kcfg_appName->text());
    Prefs::self()->save();
}

static
QPixmap generateTemplatePreviewPicture(const QString& iconName, const QString& archiveBaseName)
{
    if (iconName.isEmpty()) {
        return QPixmap();
    }

    // find archive
    QString archivePath;
    const QStringList templatePaths = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, "/kdevappwizard/templates/", QStandardPaths::LocateDirectory);
    foreach (const QString &templatePath, templatePaths) {
        foreach (const QString &templateArchive, QDir(templatePath).entryList(QDir::Files)) {
            const QString baseName = QFileInfo(templateArchive).baseName();
            if (archiveBaseName.compare(baseName) == 0) {
                archivePath = templatePath + templateArchive;
                break;
            }
        }
    }

    // read icon from archive
    if (!archivePath.isEmpty()) {
        QScopedPointer<KArchive> templateArchive;
        if (QFileInfo(archivePath).completeSuffix() == QLatin1String("zip")) {
            templateArchive.reset(new KZip(archivePath));
        } else {
            templateArchive.reset(new KTar(archivePath));
        }

        if (templateArchive->open(QIODevice::ReadOnly)) {
            const KArchiveFile* iconFile = templateArchive->directory()->file(iconName);
            if (iconFile) {
                const auto data = iconFile->data();
                QPixmap pixmap;
                const bool loadSuccess = pixmap.loadFromData(iconFile->data());
                if (loadSuccess) {
                    return pixmap;
                }
                qCWarning(KAPPTEMPLATE) << "Could not load preview icon" << iconName << "from" << archivePath;
            }
        }
    }


    // support legacy templates with image files installed separately in the filesystem
    const QString iconFilePath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("/kdevappwizard/template_previews/") + iconName);
    if (!iconFilePath.isEmpty()) {
        QPixmap pixmap(iconFilePath);
        if (!pixmap.isNull()) {
            return pixmap;
        }
        qCWarning(KAPPTEMPLATE) << "Could not load preview icon" << iconFilePath << "as wanted for" << archivePath;
    }

    // try theme icon (inofficial feature)
    if (QIcon::hasThemeIcon(iconName)) {
        return QIcon::fromTheme(iconName).pixmap(128, 128);
    }

    return QPixmap();
}

void ChoicePage::itemSelected(const QModelIndex &index)
{
    if (!index.isValid()){
        emit completeChanged();
        return;
    }

    QPixmap picture = generateTemplatePreviewPicture(index.data(AppTemplatesModel::PictureNameRole).toString(), index.data(AppTemplatesModel::BaseNameRole).toString());

    if (picture.isNull()) {
        ui_choice.pictureLabel->setText(i18n("No sample picture available."));
    } else {
        const QSize labelSize = ui_choice.pictureLabel->minimumSize();
        if ((labelSize.height() < picture.height() || labelSize.width() < picture.width()) ||
            (labelSize.height() > picture.height()*2 || labelSize.width() > picture.width()*2)) {
            picture = picture.scaled(labelSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
        ui_choice.pictureLabel->setPixmap(picture);
    }

    //and description
    QString description = index.data(AppTemplatesModel::DescriptionFileRole).toString();
    if (description.isEmpty()) {
        description = i18n("Template description"); //default if none
    }

    ui_choice.descriptionLabel->setText(description);

    m_baseName = index.data(AppTemplatesModel::BaseNameRole).toString();
    //baseName can check if an item is selected.
    if (!m_baseName.isEmpty()) {
        ui_choice.kcfg_appName->setFocus(Qt::MouseFocusReason);
    }

    emit templateNameChanged(m_baseName);
    emit completeChanged();
}

void ChoicePage::loadFromFile()
{
    QPointer<QFileDialog> fileDialog = new QFileDialog(this);

    fileDialog->setMimeTypeFilters(QStringList()
        << QStringLiteral("application/x-bzip-compressed-tar")
        << QStringLiteral("application/zip"));
    fileDialog->setFileMode(QFileDialog::ExistingFiles);

    if (!fileDialog->exec()) {
        delete fileDialog;
        return;
    }

    const QStringList selectedFiles = fileDialog->selectedFiles();
    delete fileDialog;

    const QString saveLocation = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/kdevappwizard/templates/");

    QDir dir(saveLocation);
    if (!dir.exists()) {
        dir.mkpath(QStringLiteral("."));
    }

    foreach (const QString& fileName, selectedFiles) {
        qCDebug(KAPPTEMPLATE) << "Copying" << fileName << "to" << saveLocation;
        QFileInfo info(fileName);
        QFile::copy(fileName, saveLocation + info.fileName());
    }

    templatesModel->refresh();
    ui_choice.appTree->expandAll();
    ui_choice.appTree->setFocus(Qt::OtherFocusReason);
}

void ChoicePage::getMoreTemplates()
{
    QPointer<KNS3::DownloadDialog> dialog = new KNS3::DownloadDialog(QStringLiteral("kapptemplate.knsrc"), this);

    if (!dialog->exec()) {
        delete dialog;
        return;
    }

    const bool hasChangedEntries = !dialog->changedEntries().isEmpty();
    delete dialog;

    if (hasChangedEntries) {
        templatesModel->refresh();
        ui_choice.appTree->expandAll();
        ui_choice.appTree->setFocus(Qt::OtherFocusReason);
    }
}
