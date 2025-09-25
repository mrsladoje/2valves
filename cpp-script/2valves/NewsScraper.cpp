#include "NewsScraper.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <algorithm>
#include <cctype>
#include <cpr/cpr.h>

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

string NewsScraper::cleanTextContent(const string& text) const {
    string cleaned = text;

    // Remove extra whitespace and normalize line breaks
    size_t start = cleaned.find_first_not_of(" \t\n\r");
    if (start == string::npos) return "";

    size_t end = cleaned.find_last_not_of(" \t\n\r");
    cleaned = cleaned.substr(start, end - start + 1);

    // Replace multiple spaces with single space
    size_t pos = 0;
    while ((pos = cleaned.find("  ", pos)) != string::npos) {
        cleaned.replace(pos, 2, " ");
        pos += 1;
    }

    return cleaned;
}

lxb_dom_element_t* NewsScraper::findElementByClass(lxb_dom_node_t* root, const string& className) const {
    if (!root) return nullptr;

    // If this is an element node, check if it has the class
    if (root->type == LXB_DOM_NODE_TYPE_ELEMENT) {
        lxb_dom_element_t* element = lxb_dom_interface_element(root);
        if (hasClass(element, className)) {
            return element;
        }
    }

    // Search children regardless of node type
    lxb_dom_node_t* child = lxb_dom_node_first_child(root);
    while (child) {
        lxb_dom_element_t* found = findElementByClass(child, className);
        if (found) return found;
        child = lxb_dom_node_next(child);
    }

    return nullptr;
}

lxb_dom_element_t* NewsScraper::findElementByClassAndTag(lxb_dom_node_t* root, const string& className, const string& tagName) const {
    if (!root) return nullptr;

    // If this is an element node, check if it matches both class and tag
    if (root->type == LXB_DOM_NODE_TYPE_ELEMENT) {
        lxb_dom_element_t* element = lxb_dom_interface_element(root);
        if (hasClass(element, className) && getTagName(element) == tagName) {
            return element;
        }
    }

    // Search children regardless of node type
    lxb_dom_node_t* child = lxb_dom_node_first_child(root);
    while (child) {
        lxb_dom_element_t* found = findElementByClassAndTag(child, className, tagName);
        if (found) return found;
        child = lxb_dom_node_next(child);
    }

    return nullptr;
}

lxb_dom_element_t* NewsScraper::findElementByTag(lxb_dom_node_t* root, const string& tagName) const {
    if (!root) return nullptr;

    // If this is an element node, check if it has the tag
    if (root->type == LXB_DOM_NODE_TYPE_ELEMENT) {
        lxb_dom_element_t* element = lxb_dom_interface_element(root);
        if (getTagName(element) == tagName) {
            return element;
        }
    }

    // Search children regardless of node type
    lxb_dom_node_t* child = lxb_dom_node_first_child(root);
    while (child) {
        lxb_dom_element_t* found = findElementByTag(child, tagName);
        if (found) return found;
        child = lxb_dom_node_next(child);
    }

    return nullptr;
}

string NewsScraper::getElementTextContent(lxb_dom_element_t* element) const {
    if (!element) return "";

    lxb_dom_node_t* node = lxb_dom_interface_node(element);
    size_t text_len;
    const lxb_char_t* text = lxb_dom_node_text_content(node, &text_len);

    if (!text) return "";
    return string(reinterpret_cast<const char*>(text), text_len);
}

bool NewsScraper::hasId(lxb_dom_element_t* element, const string& id) const {
    size_t attr_len;
    const lxb_char_t* attr_value = lxb_dom_element_get_attribute(element,
        reinterpret_cast<const lxb_char_t*>("id"), 2, &attr_len);

    if (attr_value && attr_len > 0) {
        string elementId(reinterpret_cast<const char*>(attr_value), attr_len);
        return elementId == id;
    }
    return false;
}

string NewsScraper::fetchHtmlContent(const string& url) {
    try {
        cpr::Response response = cpr::Get(
            cpr::Url{ url },
            cpr::Header{ {"User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36"} },
            cpr::Timeout{ REQUEST_TIMEOUT * 1000 }, // Convert to milliseconds
            cpr::ConnectTimeout{ 10000 } // 10 second connect timeout
        );

        if (response.status_code == 200) {
            return response.text;
        }
        else {
            cerr << "HTTP request failed with status: " << response.status_code << endl;
            return "";
        }
    }
    catch (const exception& e) {
        cerr << "Error fetching URL: " << e.what() << endl;
        return "";
    }
}

string NewsScraper::fetchHtmlContentWithRetry(const string& url) {
    for (int attempt = 1; attempt <= MAX_RETRIES; ++attempt) {
        string content = fetchHtmlContent(url);
        if (!content.empty()) {
            return content;
        }

        if (attempt < MAX_RETRIES) {
            this_thread::sleep_for(chrono::seconds(RETRY_DELAY));
        }
    }

    cerr << "Failed to fetch content after " << MAX_RETRIES << " attempts: " << url << endl;
    return "";
}