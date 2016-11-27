#pragma once
#include <string>

class RenderPipeline;

class RenderPass
{
    friend class RenderPipeline;

public:
    RenderPass(const std::string& name)
        : m_name(name) { }

    virtual ~RenderPass() { }

    virtual void update() = 0;

    bool isEnabled() const { return m_enabled; }

    void setEnabled(bool enabled) { m_enabled = enabled; }

protected:
    RenderPipeline* m_renderPipeline{nullptr};

    bool m_enabled{true};
    std::string m_name;
};
