#include "activevideos.h"

#include <QFileInfo>
#include <iostream>

ActiveVideos::ActiveVideos(QObject* parent):
    QObject{ parent }
{
}
void ActiveVideos::video_chosen(const QString& path)
{
    std::string filePath = path.toStdString();
    try
    {
        auto vid = reader.load_file(filePath);
        emit new_clip(vid);
    }
    catch(const std::runtime_error& err)
    {
        std::cerr << "libpgmaker error: " << err.what() << '\n';
    }
}
