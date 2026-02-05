const fs = require("fs");
const path = require("path");
const files = fs.readdirSync(process.cwd(), { recursive: true }).filter(x => x.endsWith(".html")).map(x => path.join(process.cwd(), x));

for (const file of files) {
    const content = fs.readFileSync(file, "utf8");
    fs.writeFileSync(file, content
        .replace(/&lt;MoreIcons\.hpp>/g, "&lt;hiimjustin000.more_icons/include/MoreIcons.hpp>")
        .replace(/&lt;IconInfo\.hpp>/g, "&lt;hiimjustin000.more_icons/include/IconInfo.hpp>")
        .replace(/&lt;TrailInfo\.hpp>/g, "&lt;hiimjustin000.more_icons/include/TrailInfo.hpp>")
        .replace(/(\/classes\/[^'"]+)/g, "https://docs.geode-sdk.org$1")
        .replace(/https:\/\/docs\.geode-sdk\.org\/classes\/IconInfo/g, "/classes/IconInfo")
        .replace(/https:\/\/docs\.geode-sdk\.org\/classes\/TrailInfo/g, "/classes/TrailInfo")
        .replace(/\/en\.cppreference\.com/g, "https://en.cppreference.com")
        .replace(/\/w\/cpp\/vector\/vector/g, "/w/cpp/container/vector")
        .replace(/\/w\/cpp\/vector\.h\/vector/g, "/w/cpp/container/vector")
        .replace(/\/w\/cpp\/xstring\/string/g, "/w/cpp/string/basic_string")
        .replace(/\/w\/cpp\/string\.h\/string/g, "/w/cpp/string/basic_string")
        .replace(/\/w\/cpp\/xstring\/string_view/g, "/w/cpp/string/basic_string_view")
        .replace(/\/w\/cpp\/__msvc_string_view\.hpp\/string_view/g, "/w/cpp/string/basic_string_view")
        .replace(/\/w\/cpp\/string_view\.h\/string_view/g, "/w/cpp/string/basic_string_view")
        .replace(/\/w\/cpp\/map\/map/g, "/w/cpp/container/map"));
}
