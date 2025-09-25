#include "Scraper021.h"
#include <iostream>

using namespace std;

vector<string> Scraper021::fetchAndExtractNewsLinks() {
    string url = BASE_URL + "/Najnovije/3";
    string htmlContent = fetchHtmlContentWithRetry(url);
    if (htmlContent.empty()) {
        cerr << "Failed to fetch HTML content from: " << url << endl;
        return vector<string>();
    }

    return extractNewsLinks(htmlContent);
}

vector<string> Scraper021::extractNewsLinks(const string& htmlContent) {
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

void Scraper021::findNewsLinks(lxb_dom_node_t* node, vector<string>& links) {
    if (!node) return;

    // Check if this is an element node
    if (node->type == LXB_DOM_NODE_TYPE_ELEMENT) {
        lxb_dom_element_t* element = lxb_dom_interface_element(node);

        // Check if this is an h3 with class "articleTitle"
        if (getTagName(element) == "h3" && hasClass(element, "articleTitle")) {
            // Look for anchor tag inside this h3
            lxb_dom_node_t* child = lxb_dom_node_first_child(node);
            while (child) {
                if (child->type == LXB_DOM_NODE_TYPE_ELEMENT) {
                    lxb_dom_element_t* childElement = lxb_dom_interface_element(child);

                    if (getTagName(childElement) == "a") {
                        string href = getHref(childElement);
                        if (!href.empty() && isRelevantForProtests(href)) {
                            // Links are already complete URLs, no need to prepend BASE_URL
                            links.push_back(href);
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

string Scraper021::fetchAndExtractArticleContent(const string& url) {
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

    // Add this after extracting the title:
    string date = extractDate(root);
    if (!date.empty()) {
        articleContent += date + "\n\n";
    }

    // Extract title
    string title = extractTitle(root);
    if (!title.empty()) {
        articleContent += title + "\n\n";
    }

    // Extract lead/subtitle
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

string Scraper021::extractTitle(lxb_dom_node_t* root) {
    if (!root) return "";

    // Look for h1 within storyHeading div
    lxb_dom_element_t* headingDiv = findElementByClass(root, "storyHeading");
    if (headingDiv) {
        lxb_dom_node_t* headingNode = lxb_dom_interface_node(headingDiv);
        lxb_dom_element_t* titleElement = findElementByTag(headingNode, "h1");
        if (titleElement) {
            return getElementTextContent(titleElement);
        }
    }
    return "";
}

string Scraper021::extractLead(lxb_dom_node_t* root) {
    if (!root) return "";

    lxb_dom_element_t* leadElement = findElementByClass(root, "storyLead");
    if (leadElement) {
        return getElementTextContent(leadElement);
    }
    return "";
}

string Scraper021::extractMainContent(lxb_dom_node_t* root) {
    if (!root) return "";

    string content = "";
    lxb_dom_element_t* storyBodyDiv = findElementByClass(root, "innerBody");
    if (storyBodyDiv) {
        lxb_dom_node_t* storyBodyNode = lxb_dom_interface_node(storyBodyDiv);
        lxb_dom_element_t* innerBodyDiv = findElementByClass(storyBodyNode, "innerBody");
        if (innerBodyDiv) {
            lxb_dom_node_t* innerBodyNode = lxb_dom_interface_node(innerBodyDiv);
            extractContentText(innerBodyNode, content);
        }
    }
    return content;
}

void Scraper021::extractContentText(lxb_dom_node_t* node, string& content) {
    if (!node) return;

    if (node->type == LXB_DOM_NODE_TYPE_ELEMENT) {
        lxb_dom_element_t* element = lxb_dom_interface_element(node);
        string tagName = getTagName(element);

        // Stop processing when we hit certain sections that mark the end of article content
        if (hasClass(element, "related") ||
            hasClass(element, "relatedArticles") ||
            hasClass(element, "moreCategory") ||
            hasClass(element, "tags") ||
            hasClass(element, "bottomShare") ||
            hasClass(element, "commentsBlock") ||
            hasClass(element, "commentsList") ||
            hasClass(element, "storyInlineArticles") ||
            hasClass(element, "contentExchangeStory") ||
            hasId(element, "related")) {
            return; // Stop processing this branch entirely
        }

        // Skip unwanted elements (ads, scripts, embedded content, etc.)
        if (hasClass(element, "centerAd") ||
            hasClass(element, "httpoolBanner") ||
            hasClass(element, "twitter-tweet") ||
            hasClass(element, "instagram-media") ||
            hasClass(element, "articleCopyright") ||
            tagName == "script" ||
            tagName == "iframe" ||
            tagName == "svg" ||
            tagName == "style") {
            return;
        }

        // Extract text from p elements (main content paragraphs)
        if (tagName == "p") {
            string elementText = getElementTextContent(element);
            if (!elementText.empty() && elementText.length() > 10) {
                // Filter out common unwanted patterns
                if (elementText.find("Autor") != 0 &&
                    elementText.find("Foto:") != 0 &&
                    elementText.find("OGLASI") == string::npos &&
                    elementText.find("Komentari") == string::npos &&
                    elementText.find("NAJČITANIJE") == string::npos &&
                    elementText.find("Pročitaj još") == string::npos) {
                    content += elementText + "\n\n";
                }
            }
        }

        // Extract text from div elements only if they contain substantial content
        // and don't have problematic classes
        else if (tagName == "div") {
            string elementText = getElementTextContent(element);
            if (!elementText.empty() &&
                elementText.length() > 20 &&
                elementText.length() < 1000 && // Avoid very long divs that might contain multiple elements
                !hasClass(element, "twitter-tweet") &&
                !hasClass(element, "instagram-media")) {

                // Check if this div contains mostly text content (not nested elements)
                lxb_dom_node_t* child = lxb_dom_node_first_child(lxb_dom_interface_node(element));
                int childElementCount = 0;
                while (child) {
                    if (child->type == LXB_DOM_NODE_TYPE_ELEMENT) {
                        childElementCount++;
                    }
                    child = lxb_dom_node_next(child);
                }

                // If div has few child elements, it's likely a content paragraph
                if (childElementCount <= 2) {
                    // Filter out common unwanted patterns
                    if (elementText.find("Autor") != 0 &&
                        elementText.find("Foto:") != 0 &&
                        elementText.find("OGLASI") == string::npos &&
                        elementText.find("Komentari") == string::npos &&
                        elementText.find("NAJČITANIJE") == string::npos &&
                        elementText.find("Pročitaj još") == string::npos) {
                        content += elementText + "\n\n";
                    }
                    return; // Don't process children if we extracted the div's text
                }
            }
        }

        // Extract text from strong elements (timestamps, emphasis) - but be more selective
        else if (tagName == "strong") {
            string strongText = getElementTextContent(element);
            if (!strongText.empty() && strongText.length() < 100) { // Longer strong elements are suspicious
                // Check if it looks like a timestamp or relevant emphasis
                if (strongText.find(":") != string::npos || // Likely timestamp
                    strongText.find("časova") != string::npos ||
                    strongText.find("dana") != string::npos) {
                    content += strongText + " - ";
                }
            }
        }

        // Only traverse children for container elements we want to process
        if (tagName == "div" &&
            !hasClass(element, "centerAd") &&
            !hasClass(element, "httpoolBanner") &&
            !hasClass(element, "commentsBlock") &&
            !hasClass(element, "related") &&
            !hasClass(element, "twitter-tweet") &&
            !hasClass(element, "instagram-media")) {

            lxb_dom_node_t* child = lxb_dom_node_first_child(node);
            while (child) {
                extractContentText(child, content);
                child = lxb_dom_node_next(child);
            }
        }
    }

    // Process text nodes directly (but only if parent wasn't already processed)
    else if (node->type == LXB_DOM_NODE_TYPE_TEXT) {
        size_t text_len;
        const lxb_char_t* text = lxb_dom_node_text_content(node, &text_len);
        if (text && text_len > 0) {
            string textContent(reinterpret_cast<const char*>(text), text_len);
            textContent = cleanTextContent(textContent);
            if (!textContent.empty() && textContent.length() > 5) {
                // Only add standalone text nodes that aren't whitespace
                string trimmed = textContent;
                trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r"));
                trimmed.erase(trimmed.find_last_not_of(" \t\n\r") + 1);
                if (!trimmed.empty()) {
                    content += textContent + " ";
                }
            }
        }
    }
}

string Scraper021::extractDate(lxb_dom_node_t* root) {
    if (!root) return "";

    // Look for the info div that contains the date
    lxb_dom_element_t* infoDiv = findElementByClass(root, "info");
    if (infoDiv) {
        lxb_dom_node_t* infoDivNode = lxb_dom_interface_node(infoDiv);

        // Look for h6 elements within the info div
        lxb_dom_node_t* child = lxb_dom_node_first_child(infoDivNode);
        while (child) {
            if (child->type == LXB_DOM_NODE_TYPE_ELEMENT) {
                lxb_dom_element_t* childElement = lxb_dom_interface_element(child);
                if (getTagName(childElement) == "div") {
                    // Look for h6 elements in this div
                    lxb_dom_node_t* grandchild = lxb_dom_node_first_child(child);
                    while (grandchild) {
                        if (grandchild->type == LXB_DOM_NODE_TYPE_ELEMENT) {
                            lxb_dom_element_t* grandchildElement = lxb_dom_interface_element(grandchild);
                            if (getTagName(grandchildElement) == "h6") {
                                string dateText = getElementTextContent(grandchildElement);
                                // Check if this looks like a date (contains dots and numbers)
                                if (dateText.find('.') != string::npos &&
                                    dateText.find_first_of("0123456789") != string::npos &&
                                    dateText.find(':') == string::npos) { // Exclude time entries

                                    // Parse dd.mm.yyyy. format
                                    size_t firstDot = dateText.find('.');
                                    size_t secondDot = dateText.find('.', firstDot + 1);
                                    size_t thirdDot = dateText.find('.', secondDot + 1);

                                    if (firstDot != string::npos && secondDot != string::npos && thirdDot != string::npos) {
                                        string day = dateText.substr(0, firstDot);
                                        string month = dateText.substr(firstDot + 1, secondDot - firstDot - 1);
                                        string year = dateText.substr(secondDot + 1, thirdDot - secondDot - 1);

                                        // Pad day and month with leading zeros if needed
                                        if (day.length() == 1) day = "0" + day;
                                        if (month.length() == 1) month = "0" + month;

                                        return year + "-" + month + "-" + day;
                                    }
                                }
                            }
                        }
                        grandchild = lxb_dom_node_next(grandchild);
                    }
                }
            }
            child = lxb_dom_node_next(child);
        }
    }
    return "";
}