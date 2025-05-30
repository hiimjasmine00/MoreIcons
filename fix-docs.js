const fs = require("fs");
const path = require("path");
const files = fs.readdirSync(__dirname, { recursive: true }).filter(x => x.endsWith(".html")).map(x => path.join(__dirname, x));

for (const file of files) {
    const content = fs.readFileSync(file, "utf8");
    fs.writeFileSync(file, content
        .replace(/&LTMoreIcons\.hpp>/g, "&LThiimjustin000.more_icons/include/MoreIcons.hpp>")
        .replace(/&LTIconInfo\.hpp>/g, "&LThiimjustin000.more_icons/include/IconInfo.hpp>")
        .replace(/(\/classes\/[^'"]+)/g, "https://docs.geode-sdk.org$1")
        .replace(/https:\/\/docs\.geode-sdk\.org\/classes\/IconInfo/g, "/classes/IconInfo")
        .replace(/https:\/\/docs\.geode-sdk\.org\/classes\/MoreIcons/g, "/classes/MoreIcons")
        .replace(/\/en\.cppreference\.com/g, "https://en.cppreference.com")
        .replace(/\/w\/cpp\/vector\/vector/g, "/w/cpp/container/vector")
        .replace(/\/w\/cpp\/xstring\/string/g, "/w/cpp/string/basic_string")
        .replace(/\/w\/cpp\/string\.h\/string/g, "/w/cpp/string/basic_string")
        .replace(/\/w\/cpp\/xstring\/string_view/g, "/w/cpp/string/basic_string_view")
        .replace(/\/w\/cpp\/__msvc_string_view\.hpp\/string_view/g, "/w/cpp/string/basic_string_view")
        .replace(/\/w\/cpp\/string_view\.h\/string_view/g, "/w/cpp/string/basic_string_view"));
}
