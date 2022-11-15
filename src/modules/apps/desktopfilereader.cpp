/*
 * Copyright (C) 2022 ~ 2022 Deepin Technology Co., Ltd.
 *
 * Author:     donghualin <donghualin@uniontech.com>
 *
 * Maintainer: donghualin <donghualin@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "desktopfilereader.h"
#include "basedir.h"

#include <QDir>

#define BAMF_INDEX_NAME "bamf-2.index"

DesktopFileReader *DesktopFileReader::instance()
{
    static DesktopFileReader instance;
    return &instance;
}

QString DesktopFileReader::fileName(const QString &instanceName) const
{
    for (const BamfData &lineData: m_bamfLineData) {
        if (lineData.instanceName.toLower() == instanceName.toLower()) {
            QString name = lineData.lineData.split("\t").first();
            return QString("%1%2").arg(lineData.directory).arg(name);
        }
    }

    // 如果根据instanceName没有找到，则根据空格来进行分隔
    for (const BamfData &lineData: m_bamfLineData) {
        QStringList lines = lineData.lineData.split("\t");
        if (lines.size() < 2)
            continue;

        QStringList cmds = lines[2].split(" ");
        if (cmds.size() > 1 && cmds[1].toLower() == instanceName.toLower())
            return QString("%1%2").arg(lineData.directory).arg(lines.first());
    }

    return instanceName;
}

DesktopFileReader::DesktopFileReader()
{
    loadDesktopFiles();
}

DesktopFileReader::~DesktopFileReader()
{
}

QStringList DesktopFileReader::applicationDirs() const
{
    std::vector<std::string> appDirs = BaseDir::appDirs();
    QStringList directions;
    for (std::string appDir : appDirs)
        directions << appDir.c_str();

    return directions;
}

void DesktopFileReader::loadDesktopFiles()
{
    QStringList directions = applicationDirs();
    for (const QString &direction : directions) {
        // 读取后缀名为
        QDir dir(direction);
        dir.setNameFilters(QStringList() << BAMF_INDEX_NAME);
        QFileInfoList fileList = dir.entryInfoList();
        if (fileList.size() == 0)
            continue;

        QFileInfo fileInfo = fileList.at(0);
        QFile file(fileInfo.absoluteFilePath());
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            continue;

        QList<QPair<QString, QString>> bamfLine;
        while (!file.atEnd()) {
            QString line = file.readLine();
            QStringList part = line.split("\t");
            BamfData bamf;
            bamf.directory = direction;
            if (part.size() > 2)
                bamf.instanceName = part[2].trimmed();
            bamf.lineData = line;
            m_bamfLineData << bamf;
        }
    }
}
