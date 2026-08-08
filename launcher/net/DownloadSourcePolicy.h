// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <QList>
#include <QUrl>
namespace DownloadSourcePolicy
{
    enum class Mode 
    {
    MirrorFirst,
    Official,
    Custom
    };
    enum class ResourceKind
    {
    Unknown,
    AssetIndex,
    AssetObject,
    MojangClient,
    MojangLibrary
    };
    struct CustomSources
    {
    QUrl assets;
    QUrl libraries;
    };
    QList<QUrl> candidatesFor(
    QUrl originalUrl,
    ResourceKind kind,
    Mode mode,
    const CustomSources& customSources
    );
}