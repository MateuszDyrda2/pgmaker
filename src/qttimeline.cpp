#include "qttimeline.h"

#include <iostream>

QtTimeline::QtTimeline(QObject* parent):
    QObject{ parent }, tl({}), paused(true)
{
    tl.add_channel();
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &QtTimeline::update);
    timer->start(11);
}
void QtTimeline::update()
{
    if(auto frame = tl.get_frame())
        emit frame_sent(frame);
}
void QtTimeline::play()
{
    paused ^= 1;
    tl.set_paused(paused);
}
void QtTimeline::new_clip(std::shared_ptr<libpgmaker::video> v)
{
    auto ch = tl.get_channel(0);
    if(!ch->add_clip(v, std::chrono::milliseconds(0)))
    {
        std::cerr << "Failed to load clip" << '\n';
    }
}