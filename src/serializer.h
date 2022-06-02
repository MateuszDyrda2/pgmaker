#pragma once

#include <libpgmaker/timeline.h>
#include "video_manager.h"

#include <nlohmann/json.hpp>

#include <string>

namespace libpgmaker {
using json = nlohmann::json;
struct serializer
{
    const video_manager& videos;
    const libpgmaker::timeline& tl;
};
struct deserializer
{
    std::vector<std::shared_ptr<libpgmaker::video>> videos;
    libpgmaker::timeline tl;
};
void to_json(json& j, const video& vid);
void from_json(const json& j, video& vid);
void to_json(json& j, const std::shared_ptr<video>& vid);
void from_json(const json& j, std::shared_ptr<video>& vid);

void to_json(json& j, const clip& cl);
void from_json(json& j, clip& cl);
void to_json(json& j, const std::unique_ptr<clip>& cl);
void from_json(json& j, std::unique_ptr<clip>& cl);
void to_json(json& j, const channel& ch);
void from_json(const json& j, channel& ch);
void to_json(json& j, const std::unique_ptr<channel>& ch);
void from_json(const json& j, std::unique_ptr<channel>& ch);
void to_json(json& j, const timeline& tl);
void from_json(const json& j, timeline& tl);

void to_json(json& j, const serializer& s);
void from_json(const json& j, serializer& s);
void from_json(const json& j, deserializer& s);
}
