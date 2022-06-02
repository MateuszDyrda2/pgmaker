#include "serializer.h"

#include <libpgmaker/video_reader.h>

namespace libpgmaker {
void to_json(json& j, const std::shared_ptr<video>& vid)
{
    j = json{ { "path", vid->get_info().path } };
}
// void from_json(const json& j, std::shared_ptr<video>& vid)
//{
//     std::string path;
//     j.at("path").get_to(path);
//     vid = video_reader::load_file(path)
//               .load_metadata()
//               .load_thumbnail()
//               .get();
// }
void to_json(json& j, const clip& cl)
{
    j = json{ { "file", cl.get_name() },
              { "startsAt", cl.get_starts_at().count() },
              { "startOffset", cl.get_start_offset().count() },
              { "endOffset", cl.get_end_offset().count() } };
}
// void from_json(json& j, clip& cl)
//{
//     std::string file;
//     int64_t startsAt, startOffset, endOffset;
//     j.at("file").get_to(file);
//     j.at("startsAt").get_to(startsAt);
//     j.at("startOffset").get_to(startOffset);
//     j.at("endOffset").get_to(endOffset);
//     cl = clip(file, std::chrono::milliseconds(startsAt),
//               std::chrono::milliseconds(startOffset),
//               std::chrono::milliseconds(endOffset));
// }
void to_json(json& j, const std::unique_ptr<clip>& cl)
{
    //    j = { { "clip", *cl } };
    j = json{ { "file", cl->get_name() },
              { "startsAt", cl->get_starts_at().count() },
              { "startOffset", cl->get_start_offset().count() },
              { "endOffset", cl->get_end_offset().count() } };
}
// void from_json(json& j, std::unique_ptr<clip>& cl)
//{
//     auto cls = std::make_unique<clip>(j.at("clip").get<clip>());
//     cl       = std::move(cls);
// }
void to_json(json& j, const channel& ch)
{
    j = ch.get_clips();
}
// void from_json(const json& j, channel& ch)
//{
//     auto clips = j.get<std::deque<std::unique_ptr<clip>>>();
//     ch         = channel(std::move(clips));
// }
void to_json(json& j, const std::unique_ptr<channel>& ch)
{
    // j = { { "channel", *ch } };
    j = ch->get_clips();
}
// void from_json(const json& j, std::unique_ptr<channel>& ch)
//{
//     auto chan = std::make_unique<channel>(j.at("channel").get<channel>());
//     ch        = std::move(chan);
// }
void to_json(json& j, const timeline& tl)
{
    j = tl.get_channels();
}
// void from_json(const json& j, timeline& tl)
//{
//     auto channels = j.get<std::deque<std::unique_ptr<channel>>>();
//     tl            = timeline(std::move(channels));
// }
// void to_json(json& j, const serializer& s)
//{
//    j = { { "videos", s.videos }, { "timeline", s.tl } };
//}
// void from_json(const json& j, deserializer& s)
//{
//     j.at("videos").get_to(s.videos);
//     j.at("timeline").get_to(s.tl);
// }
}