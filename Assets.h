#pragma once
#include <string>
#include <filesystem>
#include <windows.h>

// Resolves asset paths relative to the running executable instead of the
// current working directory.
//
// Why: the game loads assets with relative paths like "Assets/Player/...".
// Those resolve against the WORKING DIRECTORY, which is the exe's own folder
// when double-clicked - and there is no Assets folder next to the exe in
// Build/x64/<Config>/bin/. So every load failed for anyone who ran the exe
// directly (e.g. someone who cloned the repo).
//
// How: GetModuleFileNameA gives the full path of the running exe. We walk up
// from there looking for a folder that contains "Assets", and return that as
// the root. Falls back to "" (the old relative behaviour) if none is found,
// so this can never be worse than before.
//
// AssetRoot() is idempotent: passing it a path that is already rooted returns
// the original string unchanged, so it is safe to apply more than once.
namespace AssetPaths
{
    inline const std::string& Root()
    {
        static std::string cached;
        static bool resolved = false;

        if (resolved)
            return cached;
        resolved = true;

        char buffer[MAX_PATH] = { 0 };
        DWORD len = GetModuleFileNameA(nullptr, buffer, MAX_PATH);

        if (len == 0 || len == MAX_PATH)
        {
            cached = "";
            return cached;
        }

        std::filesystem::path dir =
            std::filesystem::path(buffer).parent_path();

        // Walk up until we find a folder containing "Assets".
        // 8 levels is far more than the bin/ -> root distance.
        for (int i = 0; i < 8 && !dir.empty(); ++i)
        {
            std::error_code ec;
            if (std::filesystem::exists(dir / "Assets", ec) && !ec)
            {
                cached = dir.string() + "/";
                return cached;
            }
            dir = dir.parent_path();
        }

        // Not found: fall back to the old relative behaviour.
        cached = "";
        return cached;
    }

    // Prefix a relative asset path with the executable-relative root.
    // Idempotent: absolute paths and already-rooted paths pass through.
    inline std::string Path(const std::string& relativePath)
    {
        if (relativePath.empty())
            return relativePath;

        // Already absolute? (drive letter, or leading slash)
        const bool hasDrive =
            relativePath.size() >= 2 &&
            ((relativePath[0] >= 'A' && relativePath[0] <= 'Z') ||
             (relativePath[0] >= 'a' && relativePath[0] <= 'z')) &&
            relativePath[1] == ':';
        const bool leadingSlash =
            relativePath[0] == '/' || relativePath[0] == '\\';

        if (hasDrive || leadingSlash)
            return relativePath;

        return Root() + relativePath;
    }
}
