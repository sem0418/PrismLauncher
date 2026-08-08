// SPDX-License-Identifier: GPL-3.0-only

#include "DownloadSourcePolicy.h"

namespace DownloadSourcePolicy
{
    QList<QUrl> candidatesFor(
        QUrl originalUrl,
        ResourceKind,
        Mode,
        const CustomSources&
    )
    {
        return { originalUrl };
    }
}