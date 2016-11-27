#pragma once
#include <unordered_map>
#include <typeindex>
#include <vector>
#include <functional>
#include <algorithm>
#include <memory>
#include <assert.h>

class BaseReceiver
{
public:
    virtual ~BaseReceiver() { }
};

template <class TEvent>
class Receiver : public BaseReceiver
{
public:
    Receiver();
    virtual ~Receiver();

    virtual void receive(const TEvent& event) = 0;
};

class EventTransmitter
{
    friend class Event;

    typedef std::function<void(const void* event)> BaseReceiveFunc;
    typedef std::vector<BaseReceiver*> ReceiverContainer;
public:
    template <class TEvent, class... Args>
    void transmit(Args&&... args);

    template <class TEvent>
    void subscribe(Receiver<TEvent>* receiver) noexcept;

    template <class TEvent>
    void unsubscribe(Receiver<TEvent>* receiver) noexcept;

    template <class TEvent>
    void sub(Receiver<TEvent>* receiver) noexcept { subscribe(receiver); }

    template <class TEvent>
    void unsub(Receiver<TEvent>* receiver) noexcept { unsubscribe(receiver); }

private:
    EventTransmitter() { }

private:
    std::unordered_map<std::type_index, ReceiverContainer> m_receiverMap;
};

class Event
{
public:
    template <class TEvent, class... Args>
    static void transmit(Args&&... args);

    static EventTransmitter& transmitter()
    {
        static EventTransmitter* m_transmitter = new EventTransmitter();
        return *m_transmitter;
    }
};

template <class TEvent, class... Args>
void Event::transmit(Args&&... args) { transmitter().transmit<TEvent>(std::forward<Args>(args)...); }

template <class TEvent>
Receiver<TEvent>::Receiver() { Event::transmitter().subscribe(this); }

template <class TEvent>
Receiver<TEvent>::~Receiver() { Event::transmitter().unsubscribe(this); }

template <class TEvent, class... Args>
void EventTransmitter::transmit(Args&&... args)
{
    TEvent event{std::forward<Args>(args)...};
    auto eventId = std::type_index(typeid(TEvent));
    auto& receivers = m_receiverMap[eventId];
    for (auto r : receivers)
    {
        Receiver<TEvent>* receiver = static_cast<Receiver<TEvent>*>(r);
        receiver->receive(event);
    }
}

template <class TEvent>
void EventTransmitter::unsubscribe(Receiver<TEvent>* receiver) noexcept
{
    auto eventId = std::type_index(typeid(TEvent));
    auto& receivers = m_receiverMap[eventId];
    auto it = std::remove(receivers.begin(), receivers.end(), receiver);
    if (it != receivers.end())
        receivers.erase(it);
}

template <class TEvent>
void EventTransmitter::subscribe(Receiver<TEvent>* receiver) noexcept
{
    auto eventId = std::type_index(typeid(TEvent));
    auto& receivers = m_receiverMap[eventId];
    assert(std::find(receivers.begin(), receivers.end(), receiver) == receivers.end());
    receivers.push_back(receiver);
}
