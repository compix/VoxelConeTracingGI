#include "RenderPipeline.h"
#include "engine/util/QueryManager.h"
#include <cstddef>

RenderPipeline::RenderPipeline(ComponentPtr<CameraComponent> camera)
    : m_camera(camera) {}

RenderPipeline::~RenderPipeline()
{
    for (auto& p : m_data)
        p.second.free();
}

std::shared_ptr<RenderPass> RenderPipeline::getRenderPass(const std::string& name, std::size_t idx)
{
    auto it = m_renderPassesMap.find(name);
    if (it == m_renderPassesMap.end())
    {
        LOG_ERROR("RenderPipeline - Error: The requested RenderPass " << name << " does not exist.");
        return nullptr;
    }

    auto& renderPasses = it->second;
    assert(renderPasses.size() > 0);

    if (renderPasses.size() <= idx)
    {
        LOG_ERROR("RenderPipeline - Error: The requested Index " << idx << " of RenderPass " << name << " does not exist.");
        return nullptr;
    }

    assert(renderPasses[idx]->m_name == name);

    return renderPasses[idx];
}

void RenderPipeline::update()
{
    for (auto& renderPass : m_renderPasses)
    {
        if (!renderPass->isEnabled())
            continue;

        QueryManager::beginElapsedTime(QueryTarget::CPU, renderPass->m_name);
        QueryManager::beginElapsedTime(QueryTarget::GPU, renderPass->m_name);

        renderPass->m_renderPipeline = this;
        renderPass->update();

        QueryManager::endElapsedTime(QueryTarget::CPU, renderPass->m_name);
        QueryManager::endElapsedTime(QueryTarget::GPU, renderPass->m_name);
    }
}

RPDataHandle RenderPipeline::fetchPtr(const RPMapKey& key)
{
    auto it = m_dataHandles.find(key);

    if (it != m_dataHandles.end())
        return it->second;

    LOG_ERROR("Could not find requested data: " << key);
    return RPDataHandle();
}
