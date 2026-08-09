// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Prism Launcher - Minecraft Launcher
 *  Copyright (C) 2022 Sefa Eyeoglu <contact@scrumplex.net>
 *  Copyright (c) 2022 Jamie Mansfield <jmansfield@cadixdev.org>
 *  Copyright (c) 2022 Lenny McLennington <lenny@sneed.church>
 *  Copyright (C) 2023 TheKodeToad <TheKodeToad@proton.me>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 3.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * This file incorporates work covered by the following copyright and
 * permission notice:
 *
 *      Copyright 2013-2021 MultiMC Contributors
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *          http://www.apache.org/licenses/LICENSE-2.0
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 */

#include "APIPage.h"
#include "ui_APIPage.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QTabBar>
#include <QValidator>
#include <QVariant>

#include "Application.h"
#include "BuildConfig.h"
#include "net/PasteUpload.h"
#include "settings/SettingsObject.h"
#include "tools/BaseProfiler.h"

APIPage::APIPage(QWidget* parent) : QWidget(parent), ui(new Ui::APIPage)
{
    // This is here so you can reorder the entries in the combobox without messing stuff up
    int comboBoxEntries[] = { PasteUpload::PasteType::Mclogs, PasteUpload::PasteType::NullPointer, PasteUpload::PasteType::PasteGG,
                              PasteUpload::PasteType::Hastebin };

    static const QRegularExpression s_validUrlRegExp("https?://.+");
    static const QRegularExpression s_validMSAClientID(
        QRegularExpression::anchoredPattern("[0-9a-f]{8}-[0-9a-f]{4}-4[0-9a-f]{3}-[89ab][0-9a-f]{3}-[0-9a-f]{12}"));

    ui->setupUi(this);

    for (auto pasteType : comboBoxEntries) {
        ui->pasteTypeComboBox->addItem(PasteUpload::PasteTypes.at(pasteType).name, pasteType);
    }

    void (QComboBox::*currentIndexChangedSignal)(int)(&QComboBox::currentIndexChanged);
    connect(ui->pasteTypeComboBox, currentIndexChangedSignal, this, &APIPage::updateBaseURLPlaceholder);
    // This function needs to be called even when the ComboBox's index is still in its default state.
    updateBaseURLPlaceholder(ui->pasteTypeComboBox->currentIndex());
    // NOTE: this allows http://, but we replace that with https later anyway
    ui->metaURL->setValidator(new QRegularExpressionValidator(s_validUrlRegExp, ui->metaURL));
    ui->resourceURL->setValidator(new QRegularExpressionValidator(s_validUrlRegExp, ui->resourceURL));
    ui->baseURLEntry->setValidator(new QRegularExpressionValidator(s_validUrlRegExp, ui->baseURLEntry));
    ui->legacyFMLLibsURL->setValidator(new QRegularExpressionValidator(s_validUrlRegExp, ui->legacyFMLLibsURL));
    ui->msaClientID->setValidator(new QRegularExpressionValidator(s_validMSAClientID, ui->msaClientID));

    ui->metaURL->setPlaceholderText(BuildConfig.META_URL);
    ui->resourceURL->setPlaceholderText(BuildConfig.DEFAULT_RESOURCE_BASE);
    ui->legacyFMLLibsURL->setPlaceholderText(BuildConfig.LEGACY_FMLLIBS_BASE_URL);
    ui->userAgentLineEdit->setPlaceholderText(BuildConfig.USER_AGENT);

    loadSettings();

    updateDownloadSourceMode();
    connect(ui->downloadSourceModeComboBox, currentIndexChangedSignal, this, &APIPage::updateDownloadSourceMode);

    resetBaseURLNote();
    connect(ui->pasteTypeComboBox, currentIndexChangedSignal, this, &APIPage::updateBaseURLNote);
    connect(ui->baseURLEntry, &QLineEdit::textEdited, this, &APIPage::resetBaseURLNote);
}

APIPage::~APIPage()
{
    delete ui;
}

void APIPage::resetBaseURLNote()
{
    ui->baseURLNote->hide();
    baseURLPasteType = ui->pasteTypeComboBox->currentIndex();
}

void APIPage::updateBaseURLNote(int index)
{
    if (baseURLPasteType == index) {
        ui->baseURLNote->hide();
    } else if (!ui->baseURLEntry->text().isEmpty()) {
        ui->baseURLNote->show();
    }
}

void APIPage::updateBaseURLPlaceholder(int index)
{
    int pasteType = ui->pasteTypeComboBox->itemData(index).toInt();
    QString pasteDefaultURL = PasteUpload::PasteTypes.at(pasteType).defaultBase;
    ui->baseURLEntry->setPlaceholderText(pasteDefaultURL);
}

void APIPage::updateDownloadSourceMode()
{
    ui->customSourceWidget->setVisible(ui->downloadSourceModeComboBox->currentIndex() == 2);
}

void APIPage::loadSettings()
{
    auto s = APPLICATION->settings();

    int pasteType = s->get("PastebinType").toInt();
    QString pastebinURL = s->get("PastebinCustomAPIBase").toString();

    ui->baseURLEntry->setText(pastebinURL);
    int pasteTypeIndex = ui->pasteTypeComboBox->findData(pasteType);
    if (pasteTypeIndex == -1) {
        pasteTypeIndex = ui->pasteTypeComboBox->findData(PasteUpload::PasteType::Mclogs);
        ui->baseURLEntry->clear();
    }

    ui->pasteTypeComboBox->setCurrentIndex(pasteTypeIndex);

    if (bool fallbackMRBlockedMods = s->get("FallbackMRBlockedMods").toBool()) {
        ui->FallbackMRBlockedMods->setChecked(fallbackMRBlockedMods);
    }

    QString msaClientID = s->get("MSAClientIDOverride").toString();
    ui->msaClientID->setText(msaClientID);
    QString metaURL = s->get("MetaURLOverride").toString();
    ui->metaURL->setText(metaURL);
    ui->metaRefreshOnLaunchCB->setCheckState(s->get("MetaRefreshOnLaunch").toBool() ? Qt::Checked : Qt::Unchecked);
    QString resourceURL = s->get("ResourceURLOverride").toString();
    ui->resourceURL->setText(resourceURL);
    QString fmlLibsURL = s->get("LegacyFMLLibsURLOverride").toString();
    ui->legacyFMLLibsURL->setText(fmlLibsURL);
    QString flameKey = s->get("FlameKeyOverride").toString();
    ui->flameKey->setText(flameKey);
    QString modrinthToken = s->get("ModrinthToken").toString();
    ui->modrinthToken->setText(modrinthToken);
    QString customUserAgent = s->get("UserAgentOverride").toString();
    ui->userAgentLineEdit->setText(customUserAgent);
    ui->technicClientID->setText(s->get("TechnicClientID").toString());

    auto modeStr = s->get("DownloadSourceMode").toString();
    if (modeStr == QStringLiteral("Official"))
        ui->downloadSourceModeComboBox->setCurrentIndex(1);
    else if (modeStr == QStringLiteral("Custom"))
        ui->downloadSourceModeComboBox->setCurrentIndex(2);
    else
        ui->downloadSourceModeComboBox->setCurrentIndex(0);

    ui->customMcBaseUrl->setText(s->get("CustomMinecraftBaseUrl").toString());
    auto mcFmt = s->get("CustomMinecraftFormat").toString();
    ui->customMcFormatComboBox->setCurrentIndex(mcFmt == QStringLiteral("BMCLAPI") ? 1 : 0);

    ui->customModsBaseUrl->setText(s->get("CustomModsBaseUrl").toString());
    auto modFmt = s->get("CustomModsFormat").toString();
    ui->customModsFormatComboBox->setCurrentIndex(modFmt == QStringLiteral("MCIM") ? 1 : 0);
}

void APIPage::applySettings()
{
    auto s = APPLICATION->settings();

    s->set("PastebinType", ui->pasteTypeComboBox->currentData().toInt());
    s->set("PastebinCustomAPIBase", ui->baseURLEntry->text());

    QString msaClientID = ui->msaClientID->text();
    s->set("MSAClientIDOverride", msaClientID);
    QUrl metaURL(ui->metaURL->text());
    QUrl resourceURL(ui->resourceURL->text());
    QUrl fmlLibsURL(ui->legacyFMLLibsURL->text());

    auto addRequiredTrailingSlash = [](QUrl& url) {
        if (!url.isEmpty() && !url.path().endsWith('/')) {
            QString path = url.path();
            path.append('/');
            url.setPath(path);
        }
    };
    addRequiredTrailingSlash(metaURL);
    addRequiredTrailingSlash(resourceURL);
    addRequiredTrailingSlash(fmlLibsURL);

    auto isLocalhost = [](const QUrl& url) { return url.host() == "localhost" || url.host() == "127.0.0.1" || url.host() == "::1"; };
    auto isUnsafe = [isLocalhost](const QUrl& url) { return !url.isEmpty() && url.scheme() == "http" && !isLocalhost(url); };
    auto upgradeToHTTPS = [isUnsafe](QUrl& url) {
        if (isUnsafe(url)) {
            url.setScheme("https");
        }
    };

    upgradeToHTTPS(metaURL);
    upgradeToHTTPS(resourceURL);
    upgradeToHTTPS(fmlLibsURL);

    s->set("FallbackMRBlockedMods", ui->FallbackMRBlockedMods->checkState());
    s->set("MetaURLOverride", metaURL.toString());
    s->set("MetaRefreshOnLaunch", ui->metaRefreshOnLaunchCB->checkState() == Qt::Checked);
    s->set("ResourceURLOverride", resourceURL.toString());
    s->set("LegacyFMLLibsURLOverride", fmlLibsURL.toString());
    QString flameKey = ui->flameKey->text();
    s->set("FlameKeyOverride", flameKey);
    QString modrinthToken = ui->modrinthToken->text();
    s->set("ModrinthToken", modrinthToken);
    s->set("UserAgentOverride", ui->userAgentLineEdit->text());
    s->set("TechnicClientID", ui->technicClientID->text());

    auto modeIdx = ui->downloadSourceModeComboBox->currentIndex();
    if (modeIdx == 1)
        s->set("DownloadSourceMode", QStringLiteral("Official"));
    else if (modeIdx == 2)
        s->set("DownloadSourceMode", QStringLiteral("Custom"));
    else
        s->set("DownloadSourceMode", QStringLiteral("Mirror"));

    s->set("CustomMinecraftBaseUrl", ui->customMcBaseUrl->text());
    s->set("CustomMinecraftFormat", ui->customMcFormatComboBox->currentIndex() == 1 ? QStringLiteral("BMCLAPI") : QStringLiteral("Mojang"));

    s->set("CustomModsBaseUrl", ui->customModsBaseUrl->text());
    s->set("CustomModsFormat", ui->customModsFormatComboBox->currentIndex() == 1 ? QStringLiteral("MCIM") : QStringLiteral("Official"));
}

bool APIPage::apply()
{
    applySettings();
    return true;
}

void APIPage::retranslate()
{
    ui->retranslateUi(this);
}
