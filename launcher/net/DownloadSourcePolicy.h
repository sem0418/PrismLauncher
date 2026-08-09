// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <QUrl>
#include <QString>
namespace DownloadSourcePolicy
{

enum class Mode
{
    Mirror,
    Official,
    Custom
};

enum class ResourceKind
{
    Unknown,

    MinecraftVersionManifest,
    MinecraftVersionJson,
    MinecraftClient,
    MinecraftAssetIndex,
    MinecraftAssetObject,
    MinecraftLibrary,

    ModrinthApi,
    ModrinthFile,
    CurseForgeApi,
    CurseForgeFile,

    PrismMetadata
};

enum class MinecraftSourceFormat
{
    Mojang,
    BMCLAPI
};

enum class ModSourceFormat
{
    Official,
    MCIM
};

struct MinecraftSource
{
    QUrl baseUrl;
    MinecraftSourceFormat format = MinecraftSourceFormat::Mojang;
};

struct ModSource
{
    QUrl baseUrl;
    ModSourceFormat format = ModSourceFormat::Official;
};

struct CustomSources
{
    MinecraftSource minecraft;
    ModSource mods;
};

struct ResourceRequest
{
    ResourceKind kind = ResourceKind::Unknown;
    QUrl originalUrl;

    QString versionId;
    QString assetHash;
    QString relativePath;
};

QUrl urlFor(
    const ResourceRequest& request,
    Mode mode,
    const CustomSources& customSources
);
Mode currentMode();
CustomSources currentCustomSources();

} // namespace DownloadSourcePolicy