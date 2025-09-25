#include "KurirScraper.h"
#include <iostream>

using namespace std;

vector<string> KurirScraper::fetchAndExtractNewsLinks() {
    string url = BASE_URL + "/najnovije-vesti";
    string htmlContent = fetchHtmlContentWithRetry(url);
    if (htmlContent.empty()) {
        cerr << "Failed to fetch HTML content from: " << url << endl;
        return vector<string>();
    }

    return extractNewsLinks(htmlContent);
}

vector<string> KurirScraper::extractNewsLinks(const string& htmlContent) {
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

void KurirScraper::findNewsLinks(lxb_dom_node_t* node, vector<string>& links) {
    if (!node) return;

    // Check if this is an element node
    if (node->type == LXB_DOM_NODE_TYPE_ELEMENT) {
        lxb_dom_element_t* element = lxb_dom_interface_element(node);

        // Check if this is an anchor tag with class "card-c in-feed"
        if (getTagName(element) == "a" && hasClass(element, "card-c") && hasClass(element, "in-feed")) {
            string href = getHref(element);
            if (!href.empty()) {
                // Check if the link starts with "/vesti/politika" or "/politika"
                if ((href.substr(0, 15) == "/vesti/politika" || href.substr(0, 9) == "/politika") &&
                    isRelevantForProtests(href)) {
                    // Prepend the BASE_URL to the link (not VESTI_PREFIX)
                    links.push_back(BASE_URL + href);
                }
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

string KurirScraper::fetchAndExtractArticleContent(const string& url) {
    string htmlContent = fetchHtmlContentWithRetry(url);
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

    // Extract date
    string date = extractDate(root);
    if (!date.empty()) {
        articleContent += date + "\n\n";
    }

    // Extract title
    string title = extractTitle(root);
    if (!title.empty()) {
        articleContent += title + "\n\n";
    }

    // Extract lead/summary
    string lead = extractLead(root);
    if (!lead.empty()) {
        articleContent += lead + "\n\n";
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

string KurirScraper::extractTitle(lxb_dom_node_t* root) {
    if (!root) return "";

    lxb_dom_element_t* titleElement = findElementByClass(root, "article-header-title");
    if (titleElement) {
        return getElementTextContent(titleElement);
    }
    return "";
}

string KurirScraper::extractLead(lxb_dom_node_t* root) {
    if (!root) return "";

    lxb_dom_element_t* leadElement = findElementByClassAndTag(root, "article-lead", "strong");
    if (leadElement) {
        return getElementTextContent(leadElement);
    }
    return "";
}

string KurirScraper::extractMainContent(lxb_dom_node_t* root) {
    if (!root) return "";

    string content = "";
    lxb_dom_element_t* articleBodyDiv = findElementByClass(root, "article-body");
    if (articleBodyDiv) {
        lxb_dom_node_t* articleBodyNode = lxb_dom_interface_node(articleBodyDiv);
        extractContentText(articleBodyNode, content);
    }
    return content;
}

void KurirScraper::extractContentText(lxb_dom_node_t* node, string& content) {
    if (!node) return;

    if (node->type == LXB_DOM_NODE_TYPE_ELEMENT) {
        lxb_dom_element_t* element = lxb_dom_interface_element(node);
        string tagName = getTagName(element);

        // Skip unwanted elements (ads, videos, embedded content, etc.)
        if (hasClass(element, "article-ad") ||
            hasClass(element, "embed-video") ||
            hasClass(element, "audioStory") ||
            hasClass(element, "video-player") ||
            hasClass(element, "article-image") ||
            hasClass(element, "related-news__wrap") ||
            hasClass(element, "article-communities") ||
            hasClass(element, "article-engagement-bar") ||
            hasClass(element, "origami-widget") ||
            hasClass(element, "OOP-bannerWrapper") ||
            hasClass(element, "article-tags") ||
            hasClass(element, "article-progression") ||
            tagName == "script" ||
            tagName == "mov-player" ||
            tagName == "iframe") {
            return;
        }

        // Extract text from article-rte divs (main content containers)
        if (hasClass(element, "article-rte")) {
            string elementText = getElementTextContent(element);
            if (!elementText.empty() && elementText.length() > 10) {
                // Enhanced filtering for unwanted patterns
                if (!isUnwantedContent(elementText)) {
                    content += elementText + "\n\n";
                }
            }
        }

        // Extract headings
        if (tagName == "h2" || tagName == "h3") {
            string headingText = getElementTextContent(element);
            if (!headingText.empty() && headingText.length() > 5) {
                if (!isUnwantedContent(headingText)) {
                    content += headingText + "\n\n";
                }
            }
        }

        // Extract paragraph content from non-article-rte containers
        if (tagName == "p" && !hasClass(element, "article-rte")) {
            string paragraphText = getElementTextContent(element);
            if (!paragraphText.empty() && paragraphText.length() > 10) {
                if (!isUnwantedContent(paragraphText)) {
                    content += paragraphText + "\n\n";
                }
            }
        }
    }

    // Recursively process children for container elements
    if (node->type == LXB_DOM_NODE_TYPE_ELEMENT) {
        lxb_dom_element_t* element = lxb_dom_interface_element(node);
        string tagName = getTagName(element);

        // Only traverse children for container elements, not unwanted ones
        if ((tagName == "div" || tagName == "article" || tagName == "section") &&
            !hasClass(element, "article-ad") && !hasClass(element, "embed-video") &&
            !hasClass(element, "audioStory") && !hasClass(element, "video-player")) {
            lxb_dom_node_t* child = lxb_dom_node_first_child(node);
            while (child) {
                extractContentText(child, content);
                child = lxb_dom_node_next(child);
            }
        }
    }
}

// Add this new helper method to your KurirScraper class
bool KurirScraper::isUnwantedContent(const string& text) {
    // List of unwanted patterns to filter out
    vector<string> unwantedPatterns = {
        "Zabranjeno preuzimanje",
        "Kurir televizija je dostupna",
        "Kurir.rs",
        "Izvor:",
        "Arena Cloud",
        "MTS Iris TV",
        "m:SAT TV",
        "Supernova",
        "Yettel Hipernet TV",
        "SBB EON",
        "BeotelNet",
        "Orion telekoma",
        "Sat-trakt",
        "M:tel",
        "MTEL Global",
        "bez navođenja i linkovanja izvora",
        "WMG uslova korišćenja",
        "Zakonom o javnom informisanju"
    };

    // Convert text to lowercase for case-insensitive matching
    string lowerText = text;
    transform(lowerText.begin(), lowerText.end(), lowerText.begin(), ::tolower);

    // Check if any unwanted pattern is found
    for (const string& pattern : unwantedPatterns) {
        string lowerPattern = pattern;
        transform(lowerPattern.begin(), lowerPattern.end(), lowerPattern.begin(), ::tolower);

        if (lowerText.find(lowerPattern) != string::npos) {
            return true;
        }
    }

    return false;
}

string KurirScraper::extractDate(lxb_dom_node_t* root) {
    if (!root) return "";

    lxb_dom_element_t* dateElement = findElementByClass(root, "article-header-date-published");
    if (dateElement) {
        string dateText = getElementTextContent(dateElement);
        if (!dateText.empty()) {
            // Find the date pattern: dd.mm.yyyy.
            size_t start = dateText.find_first_of("0123456789");
            if (start != string::npos) {
                size_t end = dateText.find('.', dateText.find('.', dateText.find('.', start) + 1) + 1);
                if (end != string::npos) {
                    string dateOnly = dateText.substr(start, end - start + 1);
                    // Remove the trailing dot if present
                    if (dateOnly.back() == '.') {
                        dateOnly.pop_back();
                    }

                    // Parse dd.mm.yyyy format
                    size_t firstDot = dateOnly.find('.');
                    size_t secondDot = dateOnly.find('.', firstDot + 1);

                    if (firstDot != string::npos && secondDot != string::npos) {
                        string day = dateOnly.substr(0, firstDot);
                        string month = dateOnly.substr(firstDot + 1, secondDot - firstDot - 1);
                        string year = dateOnly.substr(secondDot + 1);

                        // Pad day and month with leading zeros if needed
                        if (day.length() == 1) day = "0" + day;
                        if (month.length() == 1) month = "0" + month;

                        return year + "-" + month + "-" + day;
                    }
                }
            }
        }
    }
    return "";
}