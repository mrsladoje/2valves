#include "NewsScraper.h"
#include <algorithm>
#include <cctype>

using namespace std;

bool NewsScraper::isRelevantForProtests(const std::string& url) const {
    for (const auto& keyword : PROTEST_KEYWORDS) {
        auto it = std::search(url.begin(), url.end(),
            keyword.begin(), keyword.end(),
            [](unsigned char a, unsigned char b) {
                return std::tolower(a) == std::tolower(b);
            });
        if (it != url.end()) {
            return true;
        }
    }
    return false;
}

bool NewsScraper::hasClass(lxb_dom_element_t* element, const string& className) const {
    if (!element) return false;

    // Get class attribute
    lxb_dom_attr_t* attr = lxb_dom_element_attr_by_name(element,
        reinterpret_cast<const lxb_char_t*>("class"),
        5);
    if (!attr) return false;

    // Get attribute value
    size_t value_len;
    const lxb_char_t* value = lxb_dom_attr_value(attr, &value_len);
    if (!value) return false;

    string classValue(reinterpret_cast<const char*>(value), value_len);

    // Simple check if className is present in the class attribute
    return classValue.find(className) != string::npos;
}

string NewsScraper::getHref(lxb_dom_element_t* element) const {
    if (!element) return "";

    // Get href attribute
    lxb_dom_attr_t* attr = lxb_dom_element_attr_by_name(element,
        reinterpret_cast<const lxb_char_t*>("href"),
        4);
    if (!attr) return "";

    // Get attribute value
    size_t value_len;
    const lxb_char_t* value = lxb_dom_attr_value(attr, &value_len);
    if (!value) return "";

    return string(reinterpret_cast<const char*>(value), value_len);
}

string NewsScraper::getTagName(lxb_dom_element_t* element) const {
    if (!element) return "";

    // Get tag name
    size_t tag_name_len;
    const lxb_char_t* tag_name = lxb_dom_element_qualified_name(element, &tag_name_len);
    if (!tag_name) return "";

    return string(reinterpret_cast<const char*>(tag_name), tag_name_len);
}

bool NewsScraper::hasAttribute(lxb_dom_element_t* element, const string& attributeName, const string& attributeValue) const {
    if (!element) return false;

    // Get the specified attribute
    lxb_dom_attr_t* attr = lxb_dom_element_attr_by_name(element,
        reinterpret_cast<const lxb_char_t*>(attributeName.c_str()),
        attributeName.length());

    if (!attr) return false;

    // Get attribute value
    size_t value_len;
    const lxb_char_t* value = lxb_dom_attr_value(attr, &value_len);
    if (!value) return false;

    string actualValue(reinterpret_cast<const char*>(value), value_len);

    // Check if the attribute value matches
    return actualValue == attributeValue;
}