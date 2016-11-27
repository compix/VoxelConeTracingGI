#pragma once

struct Command
{
    virtual ~Command() {}
    virtual void operator()(float deltaTime) = 0;
    virtual bool done() const { return true; }
    virtual void reset() {}
};
