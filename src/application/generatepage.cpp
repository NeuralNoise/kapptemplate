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
#include <KMacroExpander>
#include <KMessageBox>
#include <KTar>
#include <KZip>
#include <KIO/CopyJob>

#include <QTemporaryDir>
#include <QDir>
#include <QFileInfo>
#include <QTextCodec>

#include "kapptemplate.h"
#include "generatepage.h"
#include "prefs.h"
#include "logging.h"

GeneratePage::GeneratePage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(i18n("Generating your project"));
    ui_generate.setupUi(this);
}

bool GeneratePage::unpackArchive(const KArchiveDirectory *dir, const QString &dest)
{
    qCDebug(KAPPTEMPLATE) << "unpacking dir:" << dir->name() << "to" << dest;
    QStringList entries = dir->entries();
    qCDebug(KAPPTEMPLATE) << "entries:" << entries.join(",");

    QTemporaryDir tdir;

    bool ret = true;

    //create path were we want copy files to
    if (!QDir::root().mkpath(dest)) {
        displayError(i18n("%1 cannot be created.", dest));
        return false;
    }

    int progress = 0;

    foreach (const QString &entry, entries) {
        progress++;
        ui_generate.progressBar->setValue((progress / entries.size()) * 100);

        if (entry.endsWith(".kdevtemplate"))
            continue;

        if (entry == templateName + ".png")
            continue;

        if (dir->entry(entry)->isDirectory()) {
            const KArchiveDirectory *file = (KArchiveDirectory *)dir->entry(entry);
            QString newdest = dest + "/" + file->name();
            if (!QFileInfo(newdest).exists()) {
                if (!QDir::root().mkdir(newdest)) {
                    displayError(i18n("Path %1 could not be created.", newdest));
                    return false;
                }
            }
            ret |= unpackArchive(file, newdest);
        }
        else if (dir->entry(entry)->isFile()) {
            const KArchiveFile *file = (KArchiveFile *)dir->entry(entry);
            file->copyTo(tdir.path());
            QString destName = KMacroExpander::expandMacros(dest + '/' + file->name(), m_variables);
            if (QFile(QDir::cleanPath(tdir.path() + '/' + file->name())).copy(destName)) {
                if (!extractFileMacros(destName)) {
                    displayError(i18n("The extraction of your personal information in file %1 was not successful", destName));
                }
            } else {
                displayError(i18n("Could not copy template file to %1.", destName));
            }
        }
    }
    return ret;
}

bool GeneratePage::extractFileMacros(const QString &entry)
{
    QString text;
    QFile file(entry);
    if(file.exists() && file.open(QFile::ReadOnly)) {
        QTextStream input(&file);
        text = KMacroExpander::expandMacros(input.readAll(), m_variables);
        file.close();
        if(file.open(QFile::WriteOnly | QIODevice::Truncate)) {
            QTextStream output(&file);
            output << text;
            file.close();
            return true;
        }
    }
    return false;
}

void GeneratePage::initializePage()
{
    feedback = i18n("Generation Progress\n");
    ui_generate.label->setText(feedback);
    templateName = field("tempName").toString();
    if (templateName.isEmpty()) {
        templateName = "kde4";
    }

    QString archName;
    const QStringList templatePaths = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, "/kdevappwizard/templates/", QStandardPaths::LocateDirectory);
    foreach (const QString &templatePath, templatePaths) {
        foreach (const QString &templateArchive, QDir(templatePath).entryList(QDir::Files)) {
            const QString baseName = QFileInfo(templateArchive).baseName();
            if (templateName.compare(baseName) == 0) {
                archName = templatePath + templateArchive;
                break;
            }
        }
    }

    if (archName.isEmpty())
        return;

    //create dir where template project will be copied
    QString appName = field("appName").toString();
    QString version = field("version").toString();
    m_variables.clear();
    m_variables["CURRENT_YEAR"] = QString().setNum(QDate::currentDate().year());
    m_variables["APPNAME"] = appName;
    m_variables["APPNAMEUC"] = appName.toUpper();
    m_variables["APPNAMELC"] = appName.toLower();
    m_variables["PROJECTDIRNAME"] = appName.toLower();
    m_variables["APPNAMEFU"] = appName.replace(0, 1, appName.toUpper().at(0));
    m_variables["AUTHOR"] = field("author").toString();
    m_variables["EMAIL"] = field("email").toString();
    m_variables["VERSION"] = version;
    m_variables["VERSIONCONTROLPLUGIN"] = version;
    m_variables["PROJECTDIRNAME"] = appName.toLower() + "-" + version; // TODO what for? change "dest" to that?


    KArchive* arch = 0;
    if (archName.endsWith(".zip")) {
        arch = new KZip(archName);
    } else {
        arch = new KTar(archName, "application/x-bzip");
    }

    QString dest(field("url").toString() + '/' + appName.toLower());
    if (arch->open(QIODevice::ReadOnly)) {
        if (!QFileInfo(dest).exists()) {
            QDir::root().mkdir(dest);
            qCDebug(KAPPTEMPLATE) << "Creating output directory:" << dest;
        }
        unpackArchive(arch->directory(), dest);
    }
    delete arch;

    feedback.append(i18n("Succeeded.\n"));
    ui_generate.label->setText(feedback);

    QString resume;
    QString url = field("url").toString();
    resume = i18n("Your project name is: <b>%1</b>, based on the %2 template.<br />", appName, templateName);
    resume.append(i18n("Version: %1 <br /><br />", version));
    resume.append(i18n("Installed in: %1 <br /><br />", url));
    resume.append(i18n("You will find a README in your project folder <b>%1</b><br /> to help you get started with your project.", url + '/' + appName.toLower()));
    ui_generate.summaryLabel->setText(resume);
}

void GeneratePage::displayError(const QString &error)
{
    KMessageBox::sorry(0, error, i18n("Error"));
    feedback.append("\n\n" + error);
    ui_generate.label->setText(feedback);
}