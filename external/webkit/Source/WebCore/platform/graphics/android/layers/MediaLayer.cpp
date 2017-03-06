/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "MediaLayer"
#define LOG_NDEBUG 1

#include "config.h"
#include "MediaLayer.h"

#include "AndroidLog.h"
#include "MediaTexture.h"
#include "TilesManager.h"

#if USE(ACCELERATED_COMPOSITING)

namespace WebCore {

MediaLayer::MediaLayer(jobject webViewRef, jobject webViewCoreRef) : LayerAndroid((RenderLayer*) NULL)
{
    m_mediaTexture = new MediaTexture(webViewRef, webViewCoreRef);
    m_mediaTexture->incStrong(this);

    m_isCopy = false;
    m_outlineSize = 0;
    ALOGV("Creating Media Layer %p", this);
}

MediaLayer::MediaLayer(const MediaLayer& layer) : LayerAndroid(layer)
{
    m_mediaTexture = layer.m_mediaTexture;
    m_mediaTexture->incStrong(this);

    m_isCopy = true;
    m_outlineSize = layer.m_outlineSize;
    ALOGV("Creating Media Layer Copy %p -> %p", &layer, this);
}

MediaLayer::~MediaLayer()
{
    ALOGV("Deleting Media Layer");
    m_mediaTexture->decStrong(this);
}

bool MediaLayer::drawGL(bool layerTilesDisabled)
{
    FloatRect clippingRect = TilesManager::instance()->shader()->rectInInvViewCoord(drawClip());
    TilesManager::instance()->shader()->clip(clippingRect);

    // when the plugin gains focus webkit applies an outline to the
    // widget, which causes the layer to expand to accommodate the
    // outline. Therefore, we shrink the rect by the outline's dimensions
    // to ensure the plugin does not draw outside of its bounds.
    SkRect mediaBounds;
    mediaBounds.set(0, 0, getSize().width(), getSize().height());
    mediaBounds.inset(m_outlineSize, m_outlineSize);

    // check to see if we need to create a content or video texture
    m_mediaTexture->initNativeWindowIfNeeded();

    // the layer's shader draws the content inverted so we must undo
    // that change in the transformation matrix
    TransformationMatrix m = m_drawTransform;
    if (!m_mediaTexture->isContentInverted()) {
        m.flipY();
        m.translate(0, -getSize().height());
    }

    // draw any content or video if present
    m_mediaTexture->draw(m, m_drawTransform, mediaBounds);
    return false;
}

ANativeWindow* MediaLayer::acquireNativeWindowForContent()
{
    return m_mediaTexture->getNativeWindowForContent();
}


ANativeWindow* MediaLayer::acquireNativeWindowForVideo()
{
    return m_mediaTexture->requestNativeWindowForVideo();
}

void MediaLayer::setWindowDimensionsForContent(const SkRect& dimensions)
{
    m_mediaTexture->setDimensions(m_mediaTexture->getNativeWindowForContent(), dimensions);
}

void MediaLayer::setWindowDimensionsForVideo(const ANativeWindow* window, const SkRect& dimensions)
{
    //TODO validate that the dimensions do not exceed the plugin's bounds
    m_mediaTexture->setDimensions(window, dimensions);
}

void MediaLayer::releaseNativeWindowForVideo(ANativeWindow* window)
{
    m_mediaTexture->releaseNativeWindow(window);
}

void MediaLayer::setFramerateCallback(const ANativeWindow* window, FramerateCallbackProc callback)
{
    m_mediaTexture->setFramerateCallback(window, callback);
}

} // namespace WebCore

#endif // USE(ACCELERATED_COMPOSITING)
