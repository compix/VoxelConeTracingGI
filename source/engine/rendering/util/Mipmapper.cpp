#include "Mipmapper.h"
#include <engine/util/Logger.h>
#include "engine/resource/ResourceManager.h"

std::shared_ptr<Shader> Mipmapper::m_mipmapShader;

void Mipmapper::init()
{
    m_mipmapShader = ResourceManager::getComputeShader("shaders/util/mipmap.comp");
}

void Mipmapper::generateMipmap3D(GLuint texture, int startDimension)
{
    m_mipmapShader->bind();
    m_mipmapShader->bindTexture3D(texture, "u_srcTex", 0);

    int curDimension = startDimension / 2;
    int mipLevel = 0;

    while (curDimension >= 1)
    {
        m_mipmapShader->setInt("u_mipDimension", curDimension);
        m_mipmapShader->setInt("u_mipLevel", mipLevel);

        glActiveTexture(GL_TEXTURE0 + 1);
        glBindImageTexture(1, texture, mipLevel + 1, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);
        glUniform1i(glGetUniformLocation(m_mipmapShader->getProgram(), "u_dstTex"), 1);

        auto workGroups = static_cast<unsigned>(glm::ceil(curDimension / 8.0f));
        m_mipmapShader->dispatchCompute(workGroups, workGroups, workGroups);
        glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        ++mipLevel;
        curDimension /= 2;
    }
}
