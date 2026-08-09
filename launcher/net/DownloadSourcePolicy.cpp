// SPDX-License-Identifier: GPL-3.0-only

#include "DownloadSourcePolicy.h"

namespace DownloadSourcePolicy
{
namespace
{

const QUrl bmclApiBase(QStringLiteral("https://bmclapi2.bangbang93.com"));
const QUrl mcimBase(QStringLiteral("https://mod.mcimirror.top"));

QUrl urlFromBase(QUrl baseUrl, QString relativePath, const QUrl& originalUrl = {})
{
    QString path = baseUrl.path();

if (!path.endsWith('/')) {
        path += '/';
    }

relativePath.remove(0, relativePath.startsWith('/') ? 1 : 0);
    baseUrl.setPath(path + relativePath);

if (!originalUrl.isEmpty()) {
        baseUrl.setQuery(originalUrl.query());
        baseUrl.setFragment(originalUrl.fragment());
    }

return baseUrl;
}

QUrl minecraftBmclUrl(const ResourceRequest& request, const QUrl& baseUrl)
{
    switch (request.kind) {
    case ResourceKind::MinecraftVersionManifest:
        return urlFromBase(baseUrl, QStringLiteral("mc/game/version_manifest.json"));

case ResourceKind::MinecraftVersionJson:
        return urlFromBase(
            baseUrl,
            QStringLiteral("version/%1/json").arg(request.versionId)
        );

case ResourceKind::MinecraftClient:
        return urlFromBase(
            baseUrl,
            QStringLiteral("version/%1/client").arg(request.versionId)
        );

case ResourceKind::MinecraftAssetIndex:
        return urlFromBase(
            baseUrl,
            QStringLiteral("version/%1/asset_index").arg(request.versionId)
        );

case ResourceKind::MinecraftAssetObject:
        return urlFromBase(
            baseUrl,
            QStringLiteral("assets/%1/%2")
                .arg(request.assetHash.left(2), request.assetHash)
        );

case ResourceKind::MinecraftLibrary:
        return urlFromBase(
            baseUrl,
            QStringLiteral("maven/%1").arg(request.relativePath)
        );

default:
        return request.originalUrl;
    }
}

QUrl modMcimUrl(const ResourceRequest& request, const QUrl& baseUrl)
{
    switch (request.kind) {
    case ResourceKind::ModrinthApi:
        return urlFromBase(
            baseUrl,
            QStringLiteral("modrinth%1").arg(request.originalUrl.path()),
            request.originalUrl
        );

case ResourceKind::ModrinthFile:
        return urlFromBase(
            baseUrl,
            request.originalUrl.path(),
            request.originalUrl
        );

case ResourceKind::CurseForgeApi:
        return urlFromBase(
            baseUrl,
            QStringLiteral("curseforge%1").arg(request.originalUrl.path()),
            request.originalUrl
        );

case ResourceKind::CurseForgeFile:
        return urlFromBase(
            baseUrl,
            request.originalUrl.path(),
            request.originalUrl
        );

default:
        return request.originalUrl;
    }
}

bool isMinecraftResource(ResourceKind kind)
{
    switch (kind) {
    case ResourceKind::MinecraftVersionManifest:
    case ResourceKind::MinecraftVersionJson:
    case ResourceKind::MinecraftClient:
    case ResourceKind::MinecraftAssetIndex:
    case ResourceKind::MinecraftAssetObject:
    case ResourceKind::MinecraftLibrary:
        return true;

default:
        return false;
    }
}

bool isModResource(ResourceKind kind)
{
    switch (kind) {
    case ResourceKind::ModrinthApi:
    case ResourceKind::ModrinthFile:
    case ResourceKind::CurseForgeApi:
    case ResourceKind::CurseForgeFile:
        return true;

default:
        return false;
    }
}

} // namespace

QUrl urlFor(const ResourceRequest& request, Mode mode, const CustomSources& customSources)
{
    if (request.kind == ResourceKind::PrismMetadata) {
        return request.originalUrl;
    }

switch (mode) {
    case Mode::Official:
        return request.originalUrl;

case Mode::Mirror:
        if (isMinecraftResource(request.kind)) {
            return minecraftBmclUrl(request, bmclApiBase);
        }

if (isModResource(request.kind)) {
            return modMcimUrl(request, mcimBase);
        }

return request.originalUrl;

case Mode::Custom:
        if (isMinecraftResource(request.kind)) {
            if (customSources.minecraft.format == MinecraftSourceFormat::BMCLAPI) {
                return minecraftBmclUrl(request, customSources.minecraft.baseUrl);
            }

return urlFromBase(
                customSources.minecraft.baseUrl,
                request.originalUrl.path(),
                request.originalUrl
            );
        }

if (isModResource(request.kind)) {
            if (customSources.mods.format == ModSourceFormat::MCIM) {
                return modMcimUrl(request, customSources.mods.baseUrl);
            }

return urlFromBase(
                customSources.mods.baseUrl,
                request.originalUrl.path(),
                request.originalUrl
            );
        }

return request.originalUrl;
    }

} // namespace DownloadSourcePolicy