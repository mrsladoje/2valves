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