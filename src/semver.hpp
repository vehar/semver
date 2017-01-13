#pragma once

#include <stdint.h>
#include <stddef.h>
#include <cstring>
#include <cstdio>

#pragma pack(push,1)
struct Version {
    enum class Pre {
        None                = 0,
        Development         = 1,
        Alpha               = 2,
        Betha               = 3,
        ReleaseCandidate    = 4,
    };

    Version(const uint16_t major = 0, 
        const uint16_t minor = 1, 
        const uint16_t patch = 0, 
        const Pre preRelease = Pre::None,
        const uint16_t preReleaseVersion = 0);

    uint16_t    major;
    uint16_t    minor;
    uint16_t    patch; 
    uint16_t    preReleaseVersion;
    Pre         preRelease;

    void set(const uint16_t major = 0, 
        const uint16_t minor = 1, 
        const uint16_t patch = 0, 
        const Pre preRelease = Pre::None,
        const uint16_t preReleaseVersion = 0);

    bool isEquals      (const Version& ver) const;
    bool isNewerThen   (const Version& ver) const;
    bool isOlderThen   (const Version& ver) const;

    bool operator>     (const Version& ver) const { return this->isNewerThen(ver); }
    bool operator<     (const Version& ver) const { return this->isOlderThen(ver); }
    bool operator==    (const Version& ver) const { return this->isEquals(ver); }
};
#pragma pack(pop)

class Semver {
public:
    static const size_t bufferSize = 30;    // major.minor.patch-preVersion.version\0
                                            //   5  1  5  1  5  1   5      1   5    1  -> 29 + 1
    Semver() = delete;

    static char*        toString    (const Version& ver, char* const buffer, const size_t bufferLength = bufferSize);
    static Version      fromString  (const char* versionStr);
private:
    static const size_t preReleaseBufferLength  = 13;   // -preVersion.version\0
                                                        // 1     5    1   5    1  -> 12 + 1

    static char*        getPreStr   (const Version::Pre rel, const uint16_t version);
    static Version::Pre getPreVal   (const char* str);
};

// ========================================================================

Version::Version(const uint16_t major, 
    const uint16_t minor, 
    const uint16_t patch, 
    const Version::Pre preRelease,
    const uint16_t preReleaseVersion) 
: major(major), minor(minor), patch(patch), 
preRelease(preRelease), preReleaseVersion(preReleaseVersion) {

}

void Version::set(const uint16_t major, 
    const uint16_t minor, 
    const uint16_t patch, 
    const Version::Pre preRelease,
    const uint16_t preReleaseVersion) {
    this->major = major;
    this->minor = minor;
    this->patch = patch;
    this->preRelease = preRelease;
    this->preReleaseVersion = preReleaseVersion;
}

bool Version::isEquals(const Version& ver) const {
    if(this->major == ver.major &&
        this->minor == ver.minor && 
        this->patch == ver.patch &&
        this->preRelease == ver.preRelease &&
        this->preReleaseVersion == ver.preReleaseVersion) {
        return true;
    }
    return false;
}

bool Version::isNewerThen(const Version& ver) const {
    if(this->major > ver.major)
        return true;

    if(this->minor > ver.minor)
        return true;

    if(this->patch > ver.patch)
        return true;

    if(this->preRelease > ver.preRelease)
        return true;

    if(this->preReleaseVersion > ver.preReleaseVersion)
        return true;

    return false;
}

bool Version::isOlderThen(const Version& ver) const {
    if(this->major < ver.major)
        return true;

    if(this->minor < ver.minor)
        return true;

    if(this->patch < ver.patch)
        return true;

    if(this->preRelease < ver.preRelease)
        return true;

    if(this->preReleaseVersion < ver.preReleaseVersion)
        return true;

    return false;
}

// ----------------------------------------------------------------------------

char* Semver::toString(const Version& version, char* const buffer, const size_t bufferLength) {
    memset(buffer, 0, bufferLength);

    const char* preReleaseBuffer = Semver::getPreStr(version.preRelease, version.preReleaseVersion);
    std::snprintf(buffer, bufferLength, "%u.%u.%u%s", version.major, version.minor, version.patch, preReleaseBuffer);

    return buffer;
}

Version Semver::fromString(const char* versionStr) {
    Version result;
    result.major = 0;
    result.minor = 0;
    result.patch = 0;
    result.preRelease = Version::Pre::None;
    result.preReleaseVersion = 0;

    char preReleaseStr[Semver::preReleaseBufferLength] = {0};
    std::sscanf(versionStr, "%u.%u.%u-%5[^.].%u",
        &result.major,
        &result.minor, 
        &result.patch,
        preReleaseStr,
        &result.preReleaseVersion);

    result.preRelease = Semver::getPreVal(preReleaseStr);

    return result;
}

// ------------------------------------------------------------------------
// Internal methods

char* Semver::getPreStr(const Version::Pre rel, const uint16_t version) {
    constexpr size_t preVersionNumberLength = 6; // 65535\0 -> 6 symbols

    static char buffer[Semver::preReleaseBufferLength] = {0};
    static char versionBuffer[preVersionNumberLength] = {0};

    // clear
    memset(buffer, 0, Semver::preReleaseBufferLength);
    memset(versionBuffer, 0, preVersionNumberLength);

    if(version > 0) {
        std::sprintf(versionBuffer, ".%u",version);
    }    

    switch(rel) {
    case Version::Pre::None:
        buffer[0] = '\0';
        break;
    case Version::Pre::Development:
        std::sprintf(buffer, "-dev%s\0", versionBuffer);
        break;
    case Version::Pre::Alpha:
        std::sprintf(buffer, "-alpha%s\0", versionBuffer);
        break;
    case Version::Pre::Betha:
        std::sprintf(buffer, "-betha%s\0", versionBuffer);
        break;
    case Version::Pre::ReleaseCandidate:
        std::sprintf(buffer, "-rc%s\0", versionBuffer);
        break;
    }
    return buffer;
}

Version::Pre Semver::getPreVal(const char* str) {
    constexpr size_t preVersionStringLength = 5;
    if(std::strncmp(str, "alpha", preVersionStringLength) == 0) {
        return Version::Pre::Alpha;
    } else if(std::strncmp(str, "betha", preVersionStringLength) == 0) {
        return Version::Pre::Betha;
    } else if(std::strncmp(str, "rc", preVersionStringLength) == 0) {
        return Version::Pre::ReleaseCandidate;
    } else if(std::strncmp(str, "dev", preVersionStringLength) == 0) {
        return Version::Pre::Development;
    }
    return Version::Pre::None;
}