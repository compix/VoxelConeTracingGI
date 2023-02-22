#pragma once
#include "Command.h"
#include <vector>
#include <memory>
#include <cstddef>

/**
* Executes a chain of commands.
*/
class CommandChain : Command
{
public:
    CommandChain() {}

    CommandChain(bool loop)
        :m_loop(loop) {}

    CommandChain(std::initializer_list<std::shared_ptr<Command>> commands, bool loop)
        :m_commands(commands), m_loop(loop) {}

    void operator()(float deltaTime) override
    {
        if (m_curCommandIdx >= m_commands.size())
            return;

        auto& curCommand = m_commands[m_curCommandIdx];
        curCommand->operator()(deltaTime);

        if (curCommand->done())
        {
            if (m_loop)
                curCommand->reset();

            ++m_curCommandIdx;
        }

        if (m_curCommandIdx == m_commands.size() && m_loop)
            m_curCommandIdx = 0;
    }

    bool done() const override { return m_curCommandIdx >= m_commands.size(); }
    void reset() override { m_curCommandIdx = 0; }

    void setLoop(bool loop) { m_loop = loop; }

    void addCommand(std::shared_ptr<Command> command) { m_commands.push_back(command); }

private:
    std::vector<std::shared_ptr<Command>> m_commands;
    std::size_t m_curCommandIdx = 0;
    bool m_loop{ false };
};
