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