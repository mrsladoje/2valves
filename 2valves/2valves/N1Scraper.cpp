#include "N1Scraper.h"
#include <iostream>
#include <cpr/cpr.h>

using namespace std;

vector<string> N1Scraper::fetchAndExtractNewsLinks() {
    string url = BASE_URL + "/najnovije/";
    string htmlContent = fetchHtmlContent(url);
    if (htmlContent.empty()) {
        cerr << "Failed to fetch HTML content from: " << url << endl;
        return vector<string>();
    }

    return extractNewsLinks(htmlContent);
}

string N1Scraper::fetchHtmlContent(const string& url) {
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

vector<string> N1Scraper::extractNewsLinks(const string& htmlContent) {
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

void N1Scraper::findNewsLinks(lxb_dom_node_t* node, vector<string>& links) {
    if (!node) return;

    // Check if this is an element node
    if (node->type == LXB_DOM_NODE_TYPE_ELEMENT) {
        lxb_dom_element_t* element = lxb_dom_interface_element(node);

        // Check if this is an h3 with attribute data-testid="article-title"
        if (getTagName(element) == "h3" && hasAttribute(element, "data-testid", "article-title")) {
            // Look for anchor tag inside this h3
            lxb_dom_node_t* child = lxb_dom_node_first_child(node);
            while (child) {
                if (child->type == LXB_DOM_NODE_TYPE_ELEMENT) {
                    lxb_dom_element_t* childElement = lxb_dom_interface_element(child);

                    if (getTagName(childElement) == "a") {
                        string href = getHref(childElement);
                        if (!href.empty() && isRelevantForProtests(href)) {
                            // Prepend BASE_URL to relative links
                            if (href[0] == '/') {
                                links.push_back(BASE_URL + href);
                            }
                            else {
                                links.push_back(href);
                            }
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

string N1Scraper::fetchAndExtractArticleContent(const string& url) {
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

    // Extract title
    string title = extractTitle(root);
    if (!title.empty()) {
        articleContent += title + "\n\n";
    }

    // Extract lead text
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

string N1Scraper::extractTitle(lxb_dom_node_t* root) {
    if (!root) return "";

    lxb_dom_element_t* titleElement = findElementByAttribute(root, "data-testid", "article-main-title");
    if (titleElement) {
        return getElementTextContent(titleElement);
    }
    return "";
}

string N1Scraper::extractLead(lxb_dom_node_t* root) {
    if (!root) return "";

    lxb_dom_element_t* leadElement = findElementByAttribute(root, "data-testid", "article-lead-text");
    if (leadElement) {
        return getElementTextContent(leadElement);
    }
    return "";
}

string N1Scraper::extractMainContent(lxb_dom_node_t* root) {
    if (!root) return "";

    string content = "";

    // Extract content from article-content-wrapper elements
    extractArticleContentWrappers(root, content);

    // Extract live blog entries if present
    extractLiveBlogEntries(root, content);

    return content;
}

void N1Scraper::extractArticleContentWrappers(lxb_dom_node_t* node, string& content) {
    if (!node) return;

    if (node->type == LXB_DOM_NODE_TYPE_ELEMENT) {
        lxb_dom_element_t* element = lxb_dom_interface_element(node);

        // Check if this is an article-content-wrapper
        if (hasClass(element, "article-content-wrapper")) {
            // Extract text from rich-text-block children
            lxb_dom_node_t* child = lxb_dom_node_first_child(node);
            while (child) {
                if (child->type == LXB_DOM_NODE_TYPE_ELEMENT) {
                    lxb_dom_element_t* childElement = lxb_dom_interface_element(child);
                    if (hasClass(childElement, "rich-text-block")) {
                        string blockText = getElementTextContent(childElement);
                        if (!blockText.empty() && blockText.length() > 10) {
                            content += blockText + "\n\n";
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
        extractArticleContentWrappers(child, content);
        child = lxb_dom_node_next(child);
    }
}

void N1Scraper::extractLiveBlogEntries(lxb_dom_node_t* node, string& content) {
    if (!node) return;

    if (node->type == LXB_DOM_NODE_TYPE_ELEMENT) {
        lxb_dom_element_t* element = lxb_dom_interface_element(node);

        // Check if this is a live blog entry
        if (hasClass(element, "lb-entry")) {
            // Extract entry title
            lxb_dom_element_t* titleElement = findElementByClass(lxb_dom_interface_node(element), "entry-title");
            if (titleElement) {
                string title = getElementTextContent(titleElement);
                if (!title.empty()) {
                    content += "## " + title + "\n\n";
                }
            }

            // Extract entry timestamp
            lxb_dom_element_t* timeElement = findElementByAttribute(lxb_dom_interface_node(element), "data-testid", "entry-time");
            if (timeElement) {
                string timestamp = getElementTextContent(timeElement);
                if (!timestamp.empty()) {
                    content += "[" + timestamp + "] ";
                }
            }

            // Extract entry body
            lxb_dom_element_t* bodyElement = findElementByClass(lxb_dom_interface_node(element), "live-blog-entry-body");
            if (bodyElement) {
                lxb_dom_node_t* bodyNode = lxb_dom_interface_node(bodyElement);
                extractArticleContentWrappers(bodyNode, content);
            }

            content += "\n";
        }
    }

    // Recursively search children
    lxb_dom_node_t* child = lxb_dom_node_first_child(node);
    while (child) {
        extractLiveBlogEntries(child, content);
        child = lxb_dom_node_next(child);
    }
}

lxb_dom_element_t* N1Scraper::findElementByAttribute(lxb_dom_node_t* root, const string& attributeName, const string& attributeValue) {
    if (!root) return nullptr;

    // If this is an element node, check if it has the attribute
    if (root->type == LXB_DOM_NODE_TYPE_ELEMENT) {
        lxb_dom_element_t* element = lxb_dom_interface_element(root);
        if (hasAttribute(element, attributeName, attributeValue)) {
            return element;
        }
    }

    // Search children
    lxb_dom_node_t* child = lxb_dom_node_first_child(root);
    while (child) {
        lxb_dom_element_t* found = findElementByAttribute(child, attributeName, attributeValue);
        if (found) return found;
        child = lxb_dom_node_next(child);
    }

    return nullptr;
}