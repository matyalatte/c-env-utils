#include <kernel/image.h>
#include <String.h>
#include <Path.h>
#include <FindDirectory.h>
#include <AppFileInfo.h>
#include "env_utils_priv.h"

// Use a method from SysInfoView::_GetABIVersion
// https://cgit.haiku-os.org/haiku/tree/src/apps/aboutsystem/AboutSystem.cpp
char *getOSVersionHaiku() {
    BString abiVersion;

    // the version is stored in the BEOS:APP_VERSION attribute of libbe.so
    BPath path;
    if (find_directory(B_BEOS_LIB_DIRECTORY, &path) == B_OK) {
        path.Append("libbe.so");

        BAppFileInfo appFileInfo;
        version_info versionInfo;
        BFile file;
        if (file.SetTo(path.Path(), B_READ_ONLY) == B_OK
            && appFileInfo.SetTo(&file) == B_OK
            && appFileInfo.GetVersionInfo(&versionInfo,
                B_APP_VERSION_KIND) == B_OK
            && versionInfo.short_info[0] != '\0')
            abiVersion = versionInfo.short_info;
    }

    if (abiVersion.IsEmpty())
        abiVersion = "Unknown";

    return AllocStrWithConst(abiVersion.String());
}
