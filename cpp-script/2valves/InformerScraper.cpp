#include "InformerScraper.h"
#include <iostream>
#include <cpr/cpr.h>

using namespace std;

vector<string> InformerScraper::fetchAndExtractNewsLinks() {
    string url = BASE_URL + "/najnovije-vesti";
    string htmlContent = fetchHtmlContent(url);
    if (htmlContent.empty()) {
        cerr << "Failed to fetch HTML content from: " << url << endl;
        return vector<string>();
    }

    return extractNewsLinks(htmlContent);
}

string InformerScraper::fetchHtmlContent(const string& url) {
    try {
        cpr::Response response = cpr::Get(
            cpr::Url{ url },
            cpr::Header{ {"User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36"} }
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

vector<string> InformerScraper::extractNewsLinks(const string& htmlContent) {
    vector<string> links;

    // Initialize lexbor
    lxb_html_document_t* document = lxb_html_document_create();
    if (!document) {
        cerr << "Failed to create HTML document" << endl;
        return links;
    }

    // Parse HTML
    lxb_status_t status = lxb_html_document_parse(document,
        reinterpret_cast<const lxb_char_t*>(htmlContent.c_str()),
        htmlContent.length());

    if (status != LXB_STATUS_OK) {
        cerr << "Failed to parse HTML document" << endl;
        lxb_html_document_destroy(document);
        return links;
    }

    // Get the root node
    lxb_dom_node_t* root = lxb_dom_interface_node(document);

    // Search for news links
    findNewsLinks(root, links);

    // Clean up
    lxb_html_document_destroy(document);

    return links;
}


void InformerScraper::findNewsLinks(lxb_dom_node_t* node, vector<string>& links) {
    if (!node) return;

    // Check if this is an element node - use the correct API
    if (node->type == LXB_DOM_NODE_TYPE_ELEMENT) {
        lxb_dom_element_t* element = lxb_dom_interface_element(node);

        // Check if this is an h2 with class "news-item-title"
        if (getTagName(element) == "h2" && hasClass(element, "news-item-title")) {
            // Look for anchor tag inside this h2
            lxb_dom_node_t* child = lxb_dom_node_first_child(node);
            while (child) {
                if (child->type == LXB_DOM_NODE_TYPE_ELEMENT) {
                    lxb_dom_element_t* childElement = lxb_dom_interface_element(child);

                    if (getTagName(childElement) == "a") {
                        string href = getHref(childElement);
                        if (!href.empty() && href.substr(0, 9) == "/politika" && isRelevantForProtests(href)) {
                            links.push_back(BASE_URL + href);
                        }
                    }
                }
                child = lxb_dom_node_next(child);
            }
        }
    }

    // Recursively search children
    lxb_dom_node_t* child = lxb_dom_node_first_child(node);
    while (child) {
        findNewsLinks(child, links);
        child = lxb_dom_node_next(child);
    }
}

string InformerScraper::fetchAndExtractArticleContent(const string& url) {
    string htmlContent = fetchHtmlContent(url);
    if (htmlContent.empty()) {
        cerr << "Failed to fetch HTML content from: " << url << endl;
        return "";
    }

    // Initialize lexbor
    lxb_html_document_t* document = lxb_html_document_create();
    if (!document) {
        cerr << "Failed to create HTML document" << endl;
        return "";
    }

    // Parse HTML
    lxb_status_t status = lxb_html_document_parse(document,
        reinterpret_cast<const lxb_char_t*>(htmlContent.c_str()),
        htmlContent.length());

    if (status != LXB_STATUS_OK) {
        cerr << "Failed to parse HTML document" << endl;
        lxb_html_document_destroy(document);
        return "";
    }

    string articleContent = "";
    lxb_dom_node_t* root = lxb_dom_interface_node(document);

    string date = extractDate(root);
    if (!date.empty()) {
        articleContent += date + "\n\n";
    }

    // Extract title
    string title = extractTitle(root);
    if (!title.empty()) {
        articleContent += title + "\n\n";
    }

    // Extract subtitle/lead
    string subtitle = extractSubtitle(root);
    if (!subtitle.empty()) {
        articleContent += subtitle + "\n\n";
    }

    // Extract main content
    string mainContent = extractMainContent(root);
    if (!mainContent.empty()) {
        articleContent += mainContent;
    }

    // Clean up
    lxb_html_document_destroy(document);

    return cleanTextContent(articleContent);
}

string InformerScraper::extractTitle(lxb_dom_node_t* root) {
    if (!root) return "";

    lxb_dom_element_t* titleElement = findElementByClassAndTag(root, "single-news-title", "h1");
    if (titleElement) {
        return getElementTextContent(titleElement);
    }
    return "";
}

string InformerScraper::extractSubtitle(lxb_dom_node_t* root) {
    if (!root) return "";

    // Look for h5 element within single-news-content
    lxb_dom_element_t* contentDiv = findElementByClass(root, "single-news-content");
    if (contentDiv) {
        lxb_dom_node_t* contentNode = lxb_dom_interface_node(contentDiv);
        lxb_dom_element_t* subtitleElement = findElementByTag(contentNode, "h5");
        if (subtitleElement) {
            return getElementTextContent(subtitleElement);
        }
    }
    return "";
}

string InformerScraper::extractMainContent(lxb_dom_node_t* root) {
    if (!root) return "";

    string content = "";
    lxb_dom_element_t* contentDiv = findElementByClass(root, "single-news-content");
    if (contentDiv) {
        lxb_dom_node_t* contentNode = lxb_dom_interface_node(contentDiv);
        extractParagraphs(contentNode, content);
    }
    return content;
}

void InformerScraper::extractParagraphs(lxb_dom_node_t* node, string& content) {
    if (!node) return;

    if (node->type == LXB_DOM_NODE_TYPE_ELEMENT) {
        lxb_dom_element_t* element = lxb_dom_interface_element(node);
        string tagName = getTagName(element);

        // Skip banner/ad elements
        if (hasClass(element, "banner-mobile-single-news") ||
            hasClass(element, "bnr") ||
            hasClass(element, "related-news") ||
            hasClass(element, "embed-responsive") ||
            hasClass(element, "platforms-box") ||
            hasClass(element, "single-news-tags")) {
            return;
        }

        // Extract paragraph content
        if (tagName == "p") {
            string paragraphText = getElementTextContent(element);
            if (!paragraphText.empty() && paragraphText.length() > 10) { // Filter out very short paragraphs
                content += paragraphText + "\n\n";
            }
        }
    }

    // Recursively process children
    lxb_dom_node_t* child = lxb_dom_node_first_child(node);
    while (child) {
        extractParagraphs(child, content);
        child = lxb_dom_node_next(child);
    }
}

string InformerScraper::extractDate(lxb_dom_node_t* root) {
    if (!root) return "";

    lxb_dom_element_t* authorTimeDiv = findElementByClass(root, "single-news-author-time");
    if (authorTimeDiv) {
        lxb_dom_node_t* authorTimeNode = lxb_dom_interface_node(authorTimeDiv);

        // Look for p elements in this div
        lxb_dom_node_t* child = lxb_dom_node_first_child(authorTimeNode);
        while (child) {
            if (child->type == LXB_DOM_NODE_TYPE_ELEMENT) {
                lxb_dom_element_t* childElement = lxb_dom_interface_element(child);
                if (getTagName(childElement) == "p") {
                    string dateText = getElementTextContent(childElement);
                    // Check if this looks like a date (dd.mm.yyyy format, no time)
                    if (dateText.find('.') != string::npos &&
                        dateText.find_first_of("0123456789") != string::npos &&
                        dateText.find(':') == string::npos && // Exclude time entries
                        dateText.find('>') == string::npos) { // Exclude time range entries

                        // Parse dd.mm.yyyy format
                        size_t firstDot = dateText.find('.');
                        size_t secondDot = dateText.find('.', firstDot + 1);

                        if (firstDot != string::npos && secondDot != string::npos) {
                            string day = dateText.substr(0, firstDot);
                            string month = dateText.substr(firstDot + 1, secondDot - firstDot - 1);
                            string year = dateText.substr(secondDot + 1);

                            // Remove any trailing whitespace from year
                            year.erase(year.find_last_not_of(" \t\n\r") + 1);

                            // Pad day and month with leading zeros if needed
                            if (day.length() == 1) day = "0" + day;
                            if (month.length() == 1) month = "0" + month;

                            return year + "-" + month + "-" + day;
                        }
                    }
                }
            }
            child = lxb_dom_node_next(child);
        }
    }
    return "";
}