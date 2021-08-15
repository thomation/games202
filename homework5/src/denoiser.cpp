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
            auto curId = frameInfo.m_id(x, y);
            if (curId < 0)
                continue;
            Matrix4x4 curModelToWorld = frameInfo.m_matrix[curId];
            Matrix4x4 preModelToWorld = frameInfo.m_matrix[curId];
            Matrix4x4 trans = preWorldToScreen *preModelToWorld * Inverse(curModelToWorld);
            Float3 prePos = trans(frameInfo.m_position(x, y), Float3::EType::Point);
            if (prePos.x >= 0 && prePos.x < width && prePos.y >= 0 && prePos.y < height) {
                auto preId = m_preFrameInfo.m_id(prePos.x, prePos.y);
                if (preId == curId) {
					m_valid(x, y) = true;
                    m_misc(x, y) = m_accColor(prePos.x, prePos.y);
                }
            }
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
            Float3 sigma = Sqr(color);
            int size = 1;
            for (int i = -kernelRadius; i <= kernelRadius; i++) {
                for (int j = -kernelRadius; j <= kernelRadius; j++) {
                    int px = x + i;
                    int py = y + j;
                    if (px >= 0 && px < width && px != x && py >= 0 && py < height && py != y) {
                        color += curFilteredColor(px, py);
                        sigma += Sqr(curFilteredColor(x, y) - curFilteredColor(px, py));
                        size++;
                    }
                }
            }
            Float3 average = color / size;
            sigma /= size;
            sigma = Float3(std::sqrt(sigma.x), std::sqrt(sigma.y), std::sqrt(sigma.z));
            color = Clamp(color, average - sigma * m_colorBoxK, average + sigma * m_colorBoxK);
            // TODO: Exponential moving average
            float alpha = m_valid(x, y) ? m_alpha : 1.0f;
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
                        float dn2 = Sqr(SafeAcos(Dot(frameInfo.m_normal(x, y),
                                         frameInfo.m_normal(px, py))));
                        float pos_dist = Distance(frameInfo.m_position(px, py),
                                                  frameInfo.m_position(x, y));
                        float dp2 = pos_dist == 0 ? 0 : Sqr(Dot(frameInfo.m_normal(x, y),
                            (frameInfo.m_position(px, py) - frameInfo.m_position(x, y)) / pos_dist));
						float weight = std::exp(-0.5 * dis2 / Sqr(m_sigmaCoord) 
                            - 0.5* cd2 / Sqr(m_sigmaColor)
                            - 0.5 * dn2 / Sqr(m_sigmaNormal)
                            - 0.5 * dp2 / Sqr(m_sigmaPlane));
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
