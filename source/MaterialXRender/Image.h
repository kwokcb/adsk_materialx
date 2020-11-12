//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_IMAGE_H
#define MATERIALX_IMAGE_H

/// @file
/// Image class

#include <MaterialXCore/Types.h>

namespace MaterialX
{

class Image;

/// A shared pointer to an image
using ImagePtr = shared_ptr<Image>;

/// A shared pointer to a const image
using ConstImagePtr = shared_ptr<const Image>;

/// A map from strings to images.
using ImageMap = std::unordered_map<string, ImagePtr>;

/// A vetor of images.
using ImageVec = std::vector<ImagePtr>;

/// A pair of images.
using ImagePair = std::pair<ImagePtr, ImagePtr>;

/// A function to perform image buffer deallocation
using ImageBufferDeallocator = std::function<void(void*)>;

/// @class Image
/// Class representing an image in system memory
class Image
{
  public:
    enum class BaseType
    {
        UINT8 = 0,
        HALF = 1,
        FLOAT = 2
    };

  public:
    /// Create an empty image with the given properties.
    static ImagePtr create(unsigned int width, unsigned int height, unsigned int channelCount, BaseType baseType = BaseType::UINT8)
    {
        return ImagePtr(new Image(width, height, channelCount, baseType));
    }

    ~Image();

    /// @name Property Accessors
    /// @{

    /// Return the width of the image.
    unsigned int getWidth() const
    {
        return _width;
    }

    /// Return the height of the image.
    unsigned int getHeight() const
    {
        return _height;
    }

    /// Return the channel count of the image.
    unsigned int getChannelCount() const
    {
        return _channelCount;
    }

    /// Return the base type of the image.
    BaseType getBaseType() const
    {
        return _baseType;
    }

    /// Return the stride of our base type in bytes.
    unsigned int getBaseStride() const;

    /// Return the maximum number of mipmaps for this image.
    unsigned int getMaxMipCount() const;

    /// @}
    /// @name Texel Accessors
    /// @{

    /// Set the texel color at the given coordinates.  If the coordinates
    /// or image resource buffer are invalid, then an exception is thrown.
    void setTexelColor(unsigned int x, unsigned int y, const Color4& color);

    /// Return the texel color at the given coordinates.  If the coordinates
    /// or image resource buffer are invalid, then an exception is thrown.
    Color4 getTexelColor(unsigned int x, unsigned int y) const;

    /// @}
    /// @name Image Analysis
    /// @{

    /// Compute the average color of the image.
    Color4 getAverageColor();

    /// Return true if all texels of this image are identical in color.
    /// @param uniformColor Return the uniform color of the image, if any.
    bool isUniformColor(Color4* uniformColor = nullptr);

    /// @}
    /// @name Image Processing
    /// @{

    /// Apply a 3x3 box blur to this image, returning a new blurred image.
    ImagePtr applyBoxBlur();

    /// Apply a 7x7 Gaussian blur to this image, returning a new blurred image.
    ImagePtr applyGaussianBlur();

    /// Split this image by the given luminance threshold, returning the
    /// resulting underflow and overflow images.
    ImagePair splitByLuminance(float luminance);

    /// @}
    /// @name Resource Buffers
    /// @{

    /// Set the resource buffer for this image.
    void setResourceBuffer(void* buffer)
    {
        _resourceBuffer = buffer;
    }

    /// Return the resource buffer for this image.
    void* getResourceBuffer() const
    {
        return _resourceBuffer;
    }

    /// Allocate a resource buffer for this image that matches its properties.
    void createResourceBuffer();

    /// Release the resource buffer for this image.
    void releaseResourceBuffer();

    /// Set the resource buffer deallocator for this image.
    void setResourceBufferDeallocator(ImageBufferDeallocator deallocator)
    {
        _resourceBufferDeallocator = deallocator;
    }

    /// Return the resource buffer deallocator for this image.
    ImageBufferDeallocator getResourceBufferDeallocator() const
    {
        return _resourceBufferDeallocator;
    }

    /// @}
    /// @name Resource IDs
    /// @{

    /// Set the resource ID for this image.
    void setResourceId(unsigned int id)
    {
        _resourceId = id;
    }

    /// Return the resource ID for this image.
    unsigned int getResourceId() const
    {
        return _resourceId;
    }

    /// @}

  protected:
    Image(unsigned int width, unsigned int height, unsigned int channelCount, BaseType baseType);

  protected:
    unsigned int _width;
    unsigned int _height;
    unsigned int _channelCount;
    BaseType _baseType;

    void* _resourceBuffer;
    ImageBufferDeallocator _resourceBufferDeallocator;
    unsigned int _resourceId;
};

/// Create a uniform-color image with the given properties.
ImagePtr createUniformImage(unsigned int width, unsigned int height, unsigned int channelCount, Image::BaseType baseType, const Color4& color);

/// Create a horizontal image strip from a vector of images with identical resolutions and formats.
ImagePtr createImageStrip(const vector<ImagePtr>& imageVec);

} // namespace MaterialX

#endif
