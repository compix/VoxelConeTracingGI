#pragma once
#include <engine/camera/CameraComponent.h>
#include <unordered_map>
#include "RPMapKey.h"
#include "RPDataHandle.h"
#include <memory>
#include "RenderPass.h"
#include <engine/util/Logger.h>

class RenderPipeline
{
    struct MemoryBlock
    {
        MemoryBlock() {}

        MemoryBlock(size_t size, char* data)
            :size(size), data(data) {}

        template<class T>
        MemoryBlock(T element)
            : size(sizeof(T))
        {
            data = new char[size];
            memcpy(data, &element, size);
        }

        void free()
        {
            if (data)
            {
                delete[] data;
                data = nullptr;
            }
        }

        template<class T>
        T* get() { return reinterpret_cast<T*>(data); }

        size_t size{ 0 };
        char* data{ nullptr };
    };

    // Multiple render passes of the same name/class are allowed -> a container is necessary
    using RenderPasses = std::vector<std::shared_ptr<RenderPass>>;
public:
    explicit RenderPipeline(ComponentPtr<CameraComponent> camera);
    ~RenderPipeline();

    template <class T>
    T* fetchPtr(const RPMapKey& key);

    template <class T>
    void putPtr(const RPMapKey& key, T* data);

    template <class T>
    T fetch(const RPMapKey& key);

    template <class T>
    void put(const RPMapKey& key, const T& data);

    template <class T>
    void addRenderPass(std::shared_ptr<T> renderPass);

    template <class T>
    void addRenderPasses(std::shared_ptr<T> renderPass);

    template <class T, class ... Ts>
    void addRenderPasses(std::shared_ptr<T> renderPass, std::shared_ptr<Ts> ... renderPasses);

    std::shared_ptr<RenderPass> getRenderPass(const std::string& name, size_t idx = 0);

    template <class T>
    std::shared_ptr<T> getRenderPass(size_t idx = 0);

    void update();

    ComponentPtr<CameraComponent> getCamera() const { return m_camera; }

private:
    RPDataHandle fetchPtr(const RPMapKey& key);

private:
    ComponentPtr<CameraComponent> m_camera;

    std::unordered_map<RPMapKey, RPDataHandle> m_dataHandles;
    std::unordered_map<RPMapKey, MemoryBlock> m_data;

    std::vector<std::shared_ptr<RenderPass>> m_renderPasses;
    std::unordered_map<std::string, RenderPasses> m_renderPassesMap;
    std::unordered_map<std::type_index, RenderPasses> m_renderPassesByTypeIdx;
};

template <class T>
T* RenderPipeline::fetchPtr(const RPMapKey& key)
{
    RPDataHandle handle = fetchPtr(key);

    return reinterpret_cast<T*>(handle.get());
}

template <class T>
void RenderPipeline::putPtr(const RPMapKey& key, T* data)
{
    m_dataHandles[key] = RPDataHandle(data);
}

template <class T>
T RenderPipeline::fetch(const RPMapKey& key)
{
    auto it = m_data.find(key);

    if (it == m_data.end())
        LOG_ERROR("RenderPipeline - Error: Expected " << key << " but not found.");

    assert(it != m_data.end());

    return *it->second.get<T>();
}

template <class T>
void RenderPipeline::put(const RPMapKey& key, const T& data)
{
    auto it = m_data.find(key);
    if (it != m_data.end())
        it->second.free();

    m_data[key] = MemoryBlock(data);
}

template <class T>
void RenderPipeline::addRenderPass(std::shared_ptr<T> renderPass)
{
    static_assert(std::is_base_of<RenderPass, T>::value, "renderPass needs to derive from RenderPass");

    m_renderPasses.push_back(renderPass);

    auto it = m_renderPassesMap.find(renderPass->m_name);
    if (it != m_renderPassesMap.end())
    {
        LOG_ERROR("RenderPipeline - Error: Trying to add a second RenderPass with the same name: " << renderPass->m_name);
        return;
    }

    renderPass->m_renderPipeline = this;
    m_renderPassesMap[renderPass->m_name].push_back(renderPass);
    m_renderPassesByTypeIdx[std::type_index(typeid(T))].push_back(renderPass);
}

template <class T>
void RenderPipeline::addRenderPasses(std::shared_ptr<T> renderPass)
{
    addRenderPass(renderPass);
}

template <class T, class ... Ts>
void RenderPipeline::addRenderPasses(std::shared_ptr<T> renderPass, std::shared_ptr<Ts> ... renderPasses)
{
    addRenderPasses(renderPass);
    addRenderPasses(renderPasses...);
}

template <class T>
std::shared_ptr<T> RenderPipeline::getRenderPass(size_t idx)
{
    auto typeIdx = std::type_index(typeid(T));
    auto it = m_renderPassesByTypeIdx.find(typeIdx);
    if (it == m_renderPassesByTypeIdx.end())
    {
        LOG_ERROR("RenderPipeline - Error: The requested RenderPass " << typeIdx.name() << " does not exist.");
        return nullptr;
    }

    auto& renderPasses = it->second;
    assert(renderPasses.size() > 0);

    if (renderPasses.size() <= idx)
    {
        LOG_ERROR("RenderPipeline - Error: The requested Index " << idx << " of RenderPass " << typeIdx.name() << " does not exist.");
        return nullptr;
    }

    return std::static_pointer_cast<T>(renderPasses[idx]);
}
