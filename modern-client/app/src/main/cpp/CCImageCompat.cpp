#include "platform/CCImage.h"
#include "platform/CCFileUtils.h"
#include "png.h"

#include <cstring>
#include <new>

NS_CC_BEGIN

CCImage::CCImage()
: m_nWidth(0)
, m_nHeight(0)
, m_nBitsPerComponent(0)
, m_pData(0)
, m_bHasAlpha(false)
, m_bPreMulti(false)
{
}

CCImage::~CCImage()
{
    delete [] m_pData;
    m_pData = 0;
}

bool CCImage::initWithImageFile(const char* strPath, EImageFormat imageType)
{
    if (!strPath) return false;
    unsigned long size = 0;
    std::string fullPath = CCFileUtils::sharedFileUtils()->fullPathForFilename(strPath);
    unsigned char* data = CCFileUtils::sharedFileUtils()->getFileData(fullPath.c_str(), "rb", &size);
    if (!data || size == 0) {
        delete [] data;
        return false;
    }
    bool ok = initWithImageData(data, static_cast<int>(size), imageType);
    delete [] data;
    return ok;
}

bool CCImage::initWithImageFileThreadSafe(const char* fullpath, EImageFormat imageType)
{
    if (!fullpath) return false;
    unsigned long size = 0;
    unsigned char* data = CCFileUtils::sharedFileUtils()->getFileData(fullpath, "rb", &size);
    if (!data || size == 0) {
        delete [] data;
        return false;
    }
    bool ok = initWithImageData(data, static_cast<int>(size), imageType);
    delete [] data;
    return ok;
}

bool CCImage::initWithImageData(void* data, int len, EImageFormat format,
                                int width, int height, int bitsPerComponent)
{
    if (!data || len <= 0) return false;

    if (format == kFmtRawData) {
        return _initWithRawData(data, len, width, height, bitsPerComponent, false);
    }

    const unsigned char* bytes = static_cast<const unsigned char*>(data);
    const bool looksPng = len >= 8 && png_sig_cmp(const_cast<png_bytep>(bytes), 0, 8) == 0;
    if (format == kFmtPng || format == kFmtUnKnown || looksPng) {
        if (looksPng || format == kFmtPng) return _initWithPngData(data, len);
    }

    // The recovered Rocks-D startup path is PNG/PVR based. Keep legacy
    // JPEG/TIFF/WebP codecs out of the ARM64 bootstrap until they are needed.
    return false;
}

bool CCImage::_initWithPngData(void* data, int len)
{
    png_image image;
    std::memset(&image, 0, sizeof(image));
    image.version = PNG_IMAGE_VERSION;

    if (!png_image_begin_read_from_memory(&image, data, static_cast<png_alloc_size_t>(len))) {
        return false;
    }

    image.format = PNG_FORMAT_RGBA;
    png_alloc_size_t outSize = PNG_IMAGE_SIZE(image);
    unsigned char* decoded = new (std::nothrow) unsigned char[outSize];
    if (!decoded) {
        png_image_free(&image);
        return false;
    }

    if (!png_image_finish_read(&image, 0, decoded, 0, 0)) {
        delete [] decoded;
        png_image_free(&image);
        return false;
    }

    delete [] m_pData;
    m_pData = decoded;
    m_nWidth = static_cast<unsigned short>(image.width);
    m_nHeight = static_cast<unsigned short>(image.height);
    m_nBitsPerComponent = 8;
    m_bHasAlpha = true;

    // cocos2d-x 2.1.x expects premultiplied RGBA textures.
    const size_t pixels = static_cast<size_t>(image.width) * static_cast<size_t>(image.height);
    for (size_t i = 0; i < pixels; ++i) {
        unsigned char* p = m_pData + i * 4;
        const unsigned int a = p[3];
        p[0] = static_cast<unsigned char>((static_cast<unsigned int>(p[0]) * (a + 1)) >> 8);
        p[1] = static_cast<unsigned char>((static_cast<unsigned int>(p[1]) * (a + 1)) >> 8);
        p[2] = static_cast<unsigned char>((static_cast<unsigned int>(p[2]) * (a + 1)) >> 8);
    }
    m_bPreMulti = true;
    png_image_free(&image);
    return true;
}

bool CCImage::_initWithRawData(void* data, int len, int width, int height,
                               int bitsPerComponent, bool preMulti)
{
    if (!data || len <= 0 || width <= 0 || height <= 0) return false;
    const size_t expected = static_cast<size_t>(width) * static_cast<size_t>(height) * 4u;
    if (static_cast<size_t>(len) < expected) return false;

    unsigned char* copy = new (std::nothrow) unsigned char[expected];
    if (!copy) return false;
    std::memcpy(copy, data, expected);

    delete [] m_pData;
    m_pData = copy;
    m_nWidth = static_cast<unsigned short>(width);
    m_nHeight = static_cast<unsigned short>(height);
    m_nBitsPerComponent = bitsPerComponent;
    m_bHasAlpha = true;
    m_bPreMulti = preMulti;
    return true;
}

bool CCImage::_initWithJpgData(void*, int) { return false; }
bool CCImage::_initWithTiffData(void*, int) { return false; }
bool CCImage::_initWithWebpData(void*, int) { return false; }

bool CCImage::initWithString(const char*, int, int, ETextAlign, const char*, int)
{
    return false;
}

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID) || (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
bool CCImage::initWithStringShadowStroke(const char*, int, int, ETextAlign,
                                         const char*, int, float, float, float,
                                         bool, float, float, float, float,
                                         bool, float, float, float, float)
{
    return false;
}
#endif

bool CCImage::saveToFile(const char*, bool) { return false; }
bool CCImage::_saveImageToPNG(const char*, bool) { return false; }
bool CCImage::_saveImageToJPG(const char*) { return false; }

NS_CC_END
