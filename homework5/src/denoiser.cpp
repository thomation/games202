#include "denoiser.h"
#include <math.h>

Denoiser::Denoiser() : m_useTemportal(false) {}

void Denoiser::Reprojection(const FrameInfo &frameInfo) {
    int height = m_accColor.m_height;
    int width = m_accColor.m_width;
    Matrix4x4 preWorldToScreen =
        m_preFrameInfo.m_matrix[m_preFrameInfo.m_matrix.size() - 1];
    Matrix4x4 preWorldToCamera =
        m_preFrameInfo.m_matrix[m_preFrameInfo.m_matrix.size() - 2];
#pragma omp parallel for
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // TODO: Reproject
            m_valid(x, y) = false;
            m_misc(x, y) = Float3(0.f);
        }
    }
    std::swap(m_misc, m_accColor);
}

void Denoiser::TemporalAccumulation(const Buffer2D<Float3> &curFilteredColor) {
    int height = m_accColor.m_height;
    int width = m_accColor.m_width;
    int kernelRadius = 3;
#pragma omp parallel for
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // TODO: Temporal clamp
            Float3 color = m_accColor(x, y);
            // TODO: Exponential moving average
            float alpha = 1.0f;
            m_misc(x, y) = Lerp(color, curFilteredColor(x, y), alpha);
        }
    }
    std::swap(m_misc, m_accColor);
}

Buffer2D<Float3> Denoiser::Filter(const FrameInfo &frameInfo) {
    int height = frameInfo.m_beauty.m_height;
    int width = frameInfo.m_beauty.m_width;
    Buffer2D<Float3> filteredImage = CreateBuffer2D<Float3>(width, height);
    int kernelRadius = 16;
#pragma omp parallel for
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // TODO: Joint bilateral filter
            float sum_weight = 0.0;
            for (int i = - kernelRadius; i <= kernelRadius; i ++) {
                for (int j = - kernelRadius; j <= kernelRadius; j ++) {
                    int px = x + i;
                    int py = y + j;
                    if (i == 0 && j == 0) {
                        filteredImage(x, y) += frameInfo.m_beauty(x, y);
                        sum_weight += 1.0;
                    }
                    else if (px >= 0 && px < width && py >= 0 && py < height) {
						float dis2 = i * i + j * j;
                        float cd2 = SqrDistance(frameInfo.m_beauty(px, px),
                                                frameInfo.m_beauty(x, y)); 
                        float dn2 = Sqr(acos(Dot(frameInfo.m_normal(x, y),
                                         frameInfo.m_normal(px, py))));
                        float pos_dist = Distance(frameInfo.m_position(px, py),
                                                  frameInfo.m_position(x, y));
                        float dp2 = pos_dist == 0 ? 0 : Sqr(Dot(frameInfo.m_normal(x, y),
                            (frameInfo.m_position(px, py) - frameInfo.m_position(x, y)) / pos_dist));
						float weight = std::exp(-0.5 * dis2 / Sqr(m_sigmaCoord) - 0.5* cd2 / Sqr(m_sigmaColor) - 0.5 * dn2 / Sqr(m_sigmaNormal) - 0.5 * dp2 / Sqr(m_sigmaPlane));
						filteredImage(x, y) += frameInfo.m_beauty(px, py) * weight;
						sum_weight += weight;
                    }
                }
            }
			filteredImage(x, y) /= sum_weight;
        }
    }
    return filteredImage;
}

void Denoiser::Init(const FrameInfo &frameInfo, const Buffer2D<Float3> &filteredColor) {
    m_accColor.Copy(filteredColor);
    int height = m_accColor.m_height;
    int width = m_accColor.m_width;
    m_misc = CreateBuffer2D<Float3>(width, height);
    m_valid = CreateBuffer2D<bool>(width, height);
}

void Denoiser::Maintain(const FrameInfo &frameInfo) { m_preFrameInfo = frameInfo; }

Buffer2D<Float3> Denoiser::ProcessFrame(const FrameInfo &frameInfo) {
    // Filter current frame
    Buffer2D<Float3> filteredColor;
    filteredColor = Filter(frameInfo);

    // Reproject previous frame color to current
    if (m_useTemportal) {
        Reprojection(frameInfo);
        TemporalAccumulation(filteredColor);
    } else {
        Init(frameInfo, filteredColor);
    }

    // Maintain
    Maintain(frameInfo);
    if (!m_useTemportal) {
        m_useTemportal = true;
    }
    return m_accColor;
}
